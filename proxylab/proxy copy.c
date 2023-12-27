#include <stdio.h>
#include <limits.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 4
#define SBUFSIZE 16

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

struct URI
{
    char host[100];
    char port[6];
    char path[100];
};

typedef struct
{
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

typedef struct cache_line
{
    int valid;
    int time_stamp;
    char uri[MAXLINE];
    char object[MAX_OBJECT_SIZE];
} Cache_line;

int countline = (MAX_CACHE_SIZE - 2 * sizeof(sem_t) - sizeof(int)) / sizeof(Cache_line);

typedef struct cache
{
    Cache_line blocks[(MAX_CACHE_SIZE - 2 * sizeof(sem_t) - sizeof(int)) / sizeof(Cache_line)];
    sem_t mutex;
    sem_t m;
    int readcnt;
} Cache;

Cache *cache = NULL;
void Init_Cache();
void free_Cache();
int get_index(char *op_uri);
int find_LRU();
int is_full();
void update(char *op_uri, char *op_object);
void update_time_stamp();

void echo(int fd);
void parse_uri(char *uri_char, struct URI *uri);
void transform_header(char *server, struct URI *uri, rio_t *client_rio);
void *thread(void *vargp);

sbuf_t sbuf;
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

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

    Init_Cache();

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

    /* while (1)
    {
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    } */

    sbuf_init(&sbuf, SBUFSIZE);
    for (int i = 0; i < NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        sbuf_insert(&sbuf, *connfdp);
    }

    free_Cache();
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
    strcpy(cache_tag, uri);

    if (strcasecmp(method, "GET"))
    {
        printf("proxy does not implement this method\n");
        return;
    }

    struct URI *uri_data = (struct URI *)malloc(sizeof(struct URI));

    int index = get_index(cache_tag);
    if (index != -1)
    {
        /* P(&cache->blocks[index].mutex);
        cache->blocks[index].readcnt++;
        if (cache->blocks[index].readcnt == 1)
            P(&cache->blocks[index].m);
        V(&cache->blocks[index].mutex); */

        P(&cache->mutex);
        cache->readcnt++;
        if (cache->readcnt == 1)
            P(&cache->m);
        V(&cache->mutex);

        memcpy(buf, cache->blocks[index].object, MAX_OBJECT_SIZE);
        Rio_writen(fd, buf, strlen(buf));

        P(&cache->mutex);
        cache->readcnt--;
        if (cache->readcnt == 0)
            V(&cache->m);
        V(&cache->mutex);

        /* P(&cache->blocks[index].mutex);
        cache->blocks[index].readcnt--;
        if (cache->blocks[index].readcnt == 0)
            V(&cache->blocks[index].m);
        V(&cache->blocks[index].mutex); */
        return;
    }

    parse_uri(uri, uri_data);

    transform_header(server, uri_data, &rio);

    int serverfd = Open_clientfd(uri_data->host, uri_data->port);
    if (serverfd < 0)
    {
        printf("connection failed\n");
        return;
    }
    Rio_readinitb(&server_rio, serverfd);
    Rio_writen(serverfd, server, strlen(server));

    size_t n;
    char cache_object[MAX_OBJECT_SIZE];
    size_t cache_size = 0;
    while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0)
    {
        if (cache_size + n < MAX_OBJECT_SIZE)
        {
            strcat(cache_object, buf);
            cache_size += n;
        }
        printf("proxy received %d bytes,then send\n", (int)n);
        Rio_writen(fd, buf, n);
    }
    if (cache_size < MAX_OBJECT_SIZE)
    {
        P(&cache->m);
        update(cache_tag, cache_object);
        V(&cache->m);
    }
    Close(serverfd);
}

void parse_uri(char *uri_char, struct URI *uri)
{
    char *hostpose = strstr(uri_char, "//");
    if (hostpose == NULL)
    {
        char *pathpose = strstr(uri_char, "/");
        if (pathpose != NULL)
            strcpy(uri->path, pathpose);
        strcpy(uri->port, "80");
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
                strcpy(uri->path, pathpose);
                strcpy(uri->port, "80");
                *pathpose = '\0';
            }
        }
        strcpy(uri->host, hostpose + 2);
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
            strcpy(host, buf);
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
    strcpy(server, request);
    return;
}

void *thread(void *vargp)
{
    Pthread_detach(pthread_self());
    while (1)
    {
        int connfd = sbuf_remove(&sbuf);
        echo(connfd);
        Close(connfd);
    }
    return NULL;
}

/*
Cache Items
*/
void Init_Cache()
{
    cache = (Cache *)malloc(sizeof(Cache));
    for (int i = 0; i < countline; i++)
    {
        cache->blocks[i].valid = 0;
        cache->blocks[i].time_stamp = -1;
        /* cache->blocks[i].readcnt = 0;
        Sem_init(&cache->blocks[i].mutex, 0, 1);
        Sem_init(&cache->blocks[i].m, 0, 1); */
        strcpy(cache->blocks[i].uri, "");
        memcpy(cache->blocks[i].object, "", 1);
    }
    cache->readcnt = 0;
    Sem_init(&cache->mutex, 0, 1);
    Sem_init(&cache->m, 0, 1);
}
void free_Cache()
{
    free(cache);
}
int get_index(char *op_uri)
{
    for (int i = 0; i < countline; i++)
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
    for (int i = 0; i < countline; i++)
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
    for (int i = 0; i < countline; i++)
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
        {
            i = find_LRU();
        }
        // P(&cache->blocks[i].m);
        strcpy(cache->blocks[i].uri, op_uri);
        memcpy(cache->blocks[i].object, op_object, MAX_OBJECT_SIZE);
        cache->blocks[i].valid = 1;
        cache->blocks[i].time_stamp = 0;
        // V(&cache->blocks[i].m);
    }
    else
    {
        // P(&cache->blocks[index].m);
        memcpy(cache->blocks[index].object, op_object, MAX_OBJECT_SIZE);
        cache->blocks[index].time_stamp = 0;
        // V(&cache->blocks[index].m);
    }
    update_time_stamp();
}
void update_time_stamp()
{
    for (int i = 0; i < countline; i++)
    {
        if (cache->blocks[i].valid > 0)
            cache->blocks[i].time_stamp++;
    }
}

/*
code for sbuf
*/
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1);
    Sem_init(&sp->slots, 0, n);
    Sem_init(&sp->items, 0, 0);
}
void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[(++sp->rear) % (sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
}
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item = sp->buf[(++sp->front) % (sp->n)];
    V(&sp->mutex);
    V(&sp->slots);
    return item;
}