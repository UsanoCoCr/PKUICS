# System-level I/O

### Unix I/O
- a linux file is a sequence of bytes
- cool fact: all I/O devices are represented as files(even the kernel is representeed as a file)

- elegant mapping of files to devices allows kernel to export simple interface called Unix I/O
    - opening and closing files: open(), close()
    - reading and writing files: read(), write()
    - changing the current file position: lseek()
        - indicates next offset to read/write

### File Types
- each file has a type indicating its role in the system
    - regular files: user data 普通文件：二进制文件、文本文件
    - directory files: index for a related group of files 目录文件，专门用来管理文件
    - socket: for communication with a process on another machine

- regular files
    - applications often distinguish between binary and text files
    - kernel doesn't care

- End of line
    - Unix uses a single character \n to represent end of line
    - Windows uses two characters \r\n
    - Mac uses \r

- Directory files
    - consists of an array of links
        - each link maps a filename to a file
    - each directory has a special link called . that refers to itself(.. refers to parent directory)
    - commands:
        - ls: list files in a directory
        - cd: change directory
        - mkdir: create a new directory
        - rmdir: remove a directory
        - pwd: print working directory

- pathnames
    - absolute pathnames: start with / e.g. /home/droh/hello.c
    - relative pathnames: start with a filename e.g. ../droh/hello.c

### Opening files
- opening a file informs the kernel that you are getting ready to access that file
```c
#include <fcntl.h>
int open(const char *pathname, int flags, mode_t mode);
```
- returns a small indentifying interger file descriptor
    - subsequent operations on the file use this descriptor
    - file descriptors 0, 1, 2 are reserved for stdin, stdout, stderr
    - -1 indicates an error

### Closing files
- closing a file informs the kernel that you are done accessing that file
```c
#include <unistd.h>
int close(int fd);

if(close(fd) == -1)
    err_sys("close error");
```
- returns 0 on success, -1 on error
- all open files are closed when a process terminates

### Reading files
- reading a file reads a sequence of bytes from the file into a buffer
```c
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);

if((nbytes = read(fd, buf, BUFSIZE)) < 0)
    err_sys("read error");
```
- returns the number of bytes read
    - 0 indicates end of file
    - -1 indicates an error
    - nbytes < count indicates end of file
    - nbytes < 0 indicates an error

### Writing files
- writing a file writes a sequence of bytes from a buffer into the file
```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t count);   

if(write(fd, buf, n) != n)
    err_sys("write error");
```
- returns the number of bytes written
    - -1 indicates an error
    - nbytes < count indicates a short counts, not an error
    - nbytes < 0 indicates an error

- short counts can occur:
    - encounter EOF on reading
    - reading text lines from a terminal
    - reading and writing network sockets

### File Metadata
- metadata is data about data, in this case file data
- per-file metadata maintained by kernel

