/*************************************************************************
	> File Name: pingpong.c
	> Author:chuanchuan
	> Mail:jianchuanwang@163.com
	> Created Time: Wed 20 Dec 2023 03:13:29 PM CST
 ************************************************************************/

#include "./user.h"
#include "../kernel/types.h"

#define RD 0                        //pipe的read端
#define WR 1                        //pipe的write端


int main() {

    char buf = 'p';
    int fd_c2p[2];                  //子进程->父进程
    int fd_p2c [2];                 //父进程->子进程
    pipe(fd_c2p);
    pipe(fd_p2c);
    

    int pid = fork();
    int exit_status = 0;
    
    //处理错误
    if (pid < 0) {
        fprintf(2, "fork() error!\n");
        close(fd_c2p[WR]);
        close(fd_c2p[RD]);
        close(fd_p2c[WR]);
        close(fd_p2c[RD]);
        exit(1);
    //处理子进程
    } else if (pid == 0) {
        close(fd_p2c[WR]); 
        close(fd_c2p[RD]);    
        if (read(fd_p2c[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child read error!\n");
            exit_status = 1;     //标记出错       
        } else {
            fprintf(1, "%d:received ping\n", getpid());
        }
        if (write(fd_c2p[WR], &buf, sizeof(buf)) != sizeof(char)) {
            fprintf(2, "child write error!\n");
            exit_status = 1;     //标记出错       
        }
        close(fd_p2c[RD]);
        close(fd_c2p[WR]);
        exit(exit_status);
        //处理父进程
    } else {
        close(fd_p2c[RD]); 
        close(fd_c2p[WR]);
        if (write(fd_p2c[WR], &buf, sizeof(buf)) != sizeof(char)) {
            fprintf(2, "parent write error!\n");
            exit_status = 1;     //标记出错       
        }
        if (read(fd_c2p[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent read error!\n");
            exit_status = 1;     //标记出错       
        } else {
            fprintf(1, "%d:received pong\n", getpid());
        }
        close(fd_p2c[WR]);
        close(fd_c2p[RD]);
        exit(exit_status);
    }
    return 0;
}
