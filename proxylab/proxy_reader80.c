#include <stdio.h>
#include <limits.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

struct URI
{
    char host[100];
    char port[6];
    char path[100];
};

typedef struct cache_line
{
    int valid;
    int time_stamp;
    char uri[MAXLINE];
    char object[MAX_OBJECT_SIZE];
} Cache_line;

typedef struct cache
{
    Cache_line blocks[MAX_CACHE_SIZE / sizeof(Cache_line)];
} Cache;

Cache *cache = NULL;
void Init_Cache();
void free_Cache();
int get_index(char *op_uri);
int find_LRU();
int is_full();
void update(char *op_uri, char *op_object);
void update_time_stamp();
int read_cache(char *op_uri, int fd);
void write_cache(char *op_uri, char *op_object);

void echo(int fd);
void parse_uri(char *uri_char, struct URI *uri);
void transform_header(char *server, struct URI *uri, rio_t *client_rio);
void *thread(void *vargp);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

sem_t mutex;
sem_t m;
int readcnt;

void sigchld_handler(int sig)
{ // reap all children
    int bkp_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = bkp_errno;
}

int main(int argc, char **argv)
{
    Signal(SIGPIPE, SIG_IGN);
    Signal(SIGCHLD, sigchld_handler);

    int listenfd, *connfdp;
    pthread_t tid;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    Init_Cache();
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    free_Cache();
    Close(listenfd);
    return 0;
}

void echo(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char server[MAXLINE];
    char cache_tag[MAXLINE];
    rio_t rio, server_rio;

    rio_readinitb(&rio, fd);
    if (!rio_readlineb(&rio, buf, MAXLINE))
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    memcpy(cache_tag, uri, sizeof(uri));

    if (strcasecmp(method, "GET"))
    {
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }

    struct URI *uri_data = (struct URI *)malloc(sizeof(struct URI));

    int index = read_cache(cache_tag, fd);
    if (index != -1)
    {
        return;
    }

    parse_uri(uri, uri_data);

    transform_header(server, uri_data, &rio);

    int serverfd = Open_clientfd(uri_data->host, uri_data->port);
    if (serverfd < 0)
    {
        clienterror(fd, method, "404", "Not Found",
                    "Tiny couldn't find this file");
        return;
    }
    Rio_readinitb(&server_rio, serverfd);
    Rio_writen(serverfd, server, strlen(server) + 1);

    size_t n = Rio_readnb(&server_rio, buf, MAXLINE);
    char cache_object[MAX_OBJECT_SIZE];
    size_t cache_size = 0;
    int can_store = 0;
    while (n > 0)
    {
        if (rio_writen(fd, buf, n) != n)
            break;
        if (cache_size + n > MAX_OBJECT_SIZE)
            can_store = -1;
        if (can_store != -1)
            memcpy(cache_object + cache_size, buf, n);
        cache_size += n;
        printf("proxy received %d bytes,then send\n", (int)n);
        n = Rio_readnb(&server_rio, buf, MAXLINE);
    }
    if (can_store != -1)
        write_cache(cache_tag, cache_object);
    if (close(serverfd) < 0)
        unix_error("close error");
}

void parse_uri(char *uri_char, struct URI *uri)
{
    char *hostpose = strstr(uri_char, "//");
    if (hostpose == NULL)
    {
        char *pathpose = strstr(uri_char, "/");
        if (pathpose != NULL)
            memcpy(uri->path, pathpose, sizeof(uri->path));
        memcpy(uri->port, "80", sizeof(uri->port));
        return;
    }
    else
    {
        char *portpose = strstr(hostpose + 2, ":");
        if (portpose != NULL)
        {
            int tmp;
            sscanf(portpose + 1, "%d%s", &tmp, uri->path);
            sprintf(uri->port, "%d", tmp);
            *portpose = '\0';
        }
        else
        {
            char *pathpose = strstr(hostpose + 2, "/");
            if (pathpose != NULL)
            {
                memcpy(uri->path, pathpose, sizeof(uri->path));
                memcpy(uri->port, "80", sizeof(uri->port));
                *pathpose = '\0';
            }
        }
        memcpy(uri->host, hostpose + 2, sizeof(uri->host));
    }
    return;
}

void transform_header(char *server, struct URI *uri, rio_t *client_rio)
{
    char buf[MAXLINE], request[MAXLINE], host[MAXLINE];
    sprintf(request, "GET %s HTTP/1.0\r\n", uri->path);
    while (Rio_readlineb(client_rio, buf, MAXLINE) > 0)
    {
        if (strcmp(buf, "\r\n") == 0)
            break;
        if (!strncasecmp(buf, "Host", strlen("Host")))
        {
            memcpy(host, buf, sizeof(host));
            continue;
        }
        if (!strncasecmp(buf, "Connection", strlen("Connection")) &&
            !strncasecmp(buf, "Proxy-Connection", strlen("Proxy-Connection")) &&
            !strncasecmp(buf, "User-Agent", strlen("User-Agent")))
            continue;
        strcat(request, buf);
    }
    if (strlen(host) == 0)
    {
        sprintf(host, "Host: %s\r\n", uri->host);
        strcat(request, host);
    }
    strcat(request, user_agent_hdr);
    strcat(request, "Connection: close\r\n");
    strcat(request, "Proxy-Connection: close\r\n");
    strcat(request, "\r\n");
    memcpy(server, request, sizeof(request));
    return;
}

void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(Pthread_self());
    Free(vargp);
    echo(connfd);
    Close(connfd);
    return NULL;
}

/*
Cache Items
*/
void Init_Cache()
{
    cache = (Cache *)malloc(sizeof(Cache));
    for (int i = 0; i < MAX_CACHE_SIZE / sizeof(Cache_line); i++)
    {
        cache->blocks[i].valid = 0;
        cache->blocks[i].time_stamp = 0;
        memcpy(cache->blocks[i].uri, "", 1);
        memcpy(cache->blocks[i].object, "", 1);
    }
    Sem_init(&mutex, 0, 1);
    Sem_init(&m, 0, 1);
    readcnt = 0;
}
void free_Cache()
{
    free(cache);
}
int get_index(char *op_uri)
{
    for (int i = 0; i < MAX_CACHE_SIZE / sizeof(Cache_line); i++)
    {
        if (cache->blocks[i].valid > 0 && strcmp(cache->blocks[i].uri, op_uri) == 0)
        {
            return i;
        }
    }
    return -1;
}
int find_LRU()
{
    int max_index = -1;
    int max_stamp = INT_MIN;
    for (int i = 0; i < MAX_CACHE_SIZE / sizeof(Cache_line); i++)
    {
        if (cache->blocks[i].time_stamp > max_stamp)
        {
            max_stamp = cache->blocks[i].time_stamp;
            max_index = i;
        }
    }
    return max_index;
}
int is_full()
{
    for (int i = 0; i < MAX_CACHE_SIZE / sizeof(Cache_line); i++)
    {
        if (cache->blocks[i].valid == 0)
        {
            return i;
        }
    }
    return -1;
}
void update(char *op_uri, char *op_object)
{
    int index = get_index(op_uri);
    if (index == -1)
    {
        int i = is_full();
        if (i == -1)
            i = find_LRU();
        memcpy(cache->blocks[i].uri, op_uri, sizeof(cache->blocks[i].uri));
        memcpy(cache->blocks[i].object, op_object, sizeof(cache->blocks[i].object));
        cache->blocks[i].valid = 1;
        cache->blocks[i].time_stamp = 0;
    }
    else
    {
        memcpy(cache->blocks[index].object, op_object, sizeof(cache->blocks[index].object));
        cache->blocks[index].time_stamp = 0;
    }
    update_time_stamp();
}
void update_time_stamp()
{
    for (int i = 0; i < MAX_CACHE_SIZE / sizeof(Cache_line); i++)
    {
        if (cache->blocks[i].valid > 0)
            cache->blocks[i].time_stamp++;
    }
}

/*
reader-writer problem
*/
int read_cache(char *op_uri, int fd)
{
    P(&mutex);
    readcnt++;
    if (readcnt == 1)
        P(&m);
    V(&mutex);

    int index = get_index(op_uri);
    if (index == -1)
    {
        P(&mutex);
        readcnt--;
        if (readcnt == 0)
            V(&m);
        V(&mutex);
        return -1;
    }
    Rio_writen(fd, cache->blocks[index].object, strlen(cache->blocks[index].object) + 1);

    P(&mutex);
    readcnt--;
    if (readcnt == 0)
        V(&m);
    V(&mutex);

    if (index != -1)
    {
        P(&m);
        cache->blocks[index].time_stamp = 0;
        update_time_stamp();
        V(&m);
    }
    return 0;
}
void write_cache(char *op_uri, char *op_object)
{
    P(&m);
    update(op_uri, op_object);
    V(&m);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-overflow"
    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor="
                  "ffffff"
                  ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
#pragma GCC diagnostic pop
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}