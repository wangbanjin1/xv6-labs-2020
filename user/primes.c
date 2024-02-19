/*************************************************************************
	> File Name: primes.c
	> Author:chuanchuan
	> Mail:jianchuanwang@163.com
	> Created Time: Thu 21 Dec 2023 02:53:25 PM CST
 ************************************************************************/
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "./user.h"

#define RD 0
#define WR 1
#define INT_LEN sizeof(int)

int getFirstData(int lpipe[2], int *dst) {
    if (read(lpipe[RD], dst, INT_LEN) == INT_LEN) {
        printf("prime:%d\n", *dst);
        return 0;
    }
    return -1;
}

//读取左邻居的数据，将不能被first整除的写入右邻居
void transData(int lpipe[2], int rpipe[2], int first) {
    int data;
    while (read(lpipe[RD], &data, INT_LEN) == INT_LEN) {
        //data将不可能是素数
        if (data % first != 0) {
            write(rpipe[WR], &data, INT_LEN);
        }
    }
    close(lpipe[RD]);
    close(rpipe[WR]);
}

void primes(int lpipe[2]) {
    close(lpipe[WR]);
    int first;
    if (getFirstData(lpipe, &first) == 0) {
        int p[2];
        //这里的p实际上是为了递归处理
        pipe(p);
        transData(lpipe, p, first);
        if (fork() == 0) {
            primes(p);
        } else {
            close(p[RD]);
            close(p[WR]);
            wait(0);
        }
    }
    exit(0);
}
int main() {

    int p[2];
    pipe(p);

    for (int i = 2;i <= 35;++i) {
        write(p[WR], &i, INT_LEN);
    }
    if (fork() == 0) {
        primes(p);
    } else {
        close(p[WR]);
        close(p[RD]);
        wait(0);
    }
    exit(0);
}
