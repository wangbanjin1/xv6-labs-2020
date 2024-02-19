/*************************************************************************
	> File Name: find.c
	> Author:chuanchuan
	> Mail:jianchuanwang@163.com
	> Created Time: Mon 22 Jan 2024 10:48:06 AM CST
 ************************************************************************/
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "./user.h"
#include "../kernel/fs.h"

void find(const char *path, const char *filename) {
    char *p, buff[512];
    int fd;
    struct stat st;
    struct dirent de;
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    //获取打开文件目录的信息并保存到st结构体中
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot fstat %s\n", path); 
        close(fd);
        return;
    }

    /*
    struct stat {
        int dev;     // File system's disk device
        uint ino;    // Inode number
        short type;  // Type of file
        short nlink; // Number of links to file
        uint64 size; // Size of file in bytes
    };
    */

    /*
    struct dirent {
        ushort inum;
        char name[DIRSIZ];
    };
    */

    //find的第一个参数必须是目录
    if (st.type != T_DIR) {
        fprintf(2, "usage: find<DIRECTORY><filename>\n");
        return;
    }
    if (strlen(path) + DIRSIZ + 1 > sizeof(buff)) {
        fprintf(2, "find: path is too long\n");
        return;
    }
    strcpy(buff, path);
    p = buff + strlen(buff);
    //后++
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        //无效或者没有使用的目录项
        if (de.inum == 0) {
            continue;
        }
        //添加路径名称
        memmove(p, de.name, DIRSIZ);
        //字符串结束标志
        p[DIRSIZ] = 0;
        if (stat(buff, &st) < 0) {
           fprintf(2, "find: cannot stat %s\n", buff); 
           continue;
        }
        //不要在.和..中递归
        if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
            find(buff, filename);
        } else if (strcmp(filename, p) == 0) {
            printf("%s\n", buff);
        }
    }
    close(fd);
    return;

}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "usage:find <directory> <filename>\n");
        exit(1);
    }
    //printf("arg is %s, and arg2 is %s\n", argv[1], argv[2]);
    find(argv[1], argv[2]);
    exit(0);
}

