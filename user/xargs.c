#include "../kernel/types.h"
#include "./user.h"
#include "../kernel/param.h"

#define MAXSZ 512
enum state {
    S_WAIT,                         //等待参数输入，此状态为初始状态或者当前字符为空格
    S_ARG,                          //参数内
    S_ARG_END,                      //参数结束
    S_ARG_LINE_END,                 //左侧有参数的换行，"arg\n"
    S_LINE_END,                     //左侧为空格的换行，"arg \n"
    S_END                           //结束, EOF
};

enum type{
    C_SPACE,
    C_CHAR,
    C_LINE_END
};

enum type get_type(char c) {
    switch (c) {
        case ' ':
            return C_SPACE;
        case '\n':
            return C_LINE_END;
        default:
            return C_CHAR;
    }
}

//cur_type为将要读取的字符
enum state trans_state(enum state cur_state, enum type cur_type) {
    switch(cur_state) {
        case S_WAIT:
            if (cur_type == C_SPACE) {
                return S_WAIT;
            }
            if (cur_type == C_LINE_END) {
                return S_LINE_END;
            }
            if (cur_type == C_CHAR) {
                return S_ARG;
            }
            break;
        case S_ARG:
            if (cur_type == C_SPACE) {
                return S_ARG_END;
            }
            if (cur_type == C_LINE_END) {
                return S_ARG_LINE_END;
            }
            if (cur_type == C_CHAR) {
                return S_ARG;
            }
            break;
        case S_ARG_END:
        case S_ARG_LINE_END:
        case S_LINE_END:
            if (cur_type == C_SPACE) {
                return S_WAIT;
            }
            if (cur_type == C_LINE_END) {
                return S_LINE_END;
            }
            if (cur_type == C_CHAR) {
                return S_ARG;
            }
            break;
        default:
            break;
    }
    return S_END;
}

//将参数列表后面的元素全置为空，用于换行时重新赋予参数
//x_argv是参数指针数组，beg为清空的起始下标
void clear_argv(char *x_argv[MAXARG], int beg) {
    for (int i = beg;i < MAXARG;i++) {
        x_argv[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc >= MAXARG + 1) {
        fprintf(2, "xargs: too many arguments");
        exit(1);
    }
    char lines[MAXSZ];
    char *p = lines;
    //参数指针数组，全部初始化为空指针
    char *x_argv[MAXARG] = {0};
    //存储原有的参数
    for (int i = 1;i < argc;i++) {
        x_argv[i - 1] = argv[i];
    }
    //单个参数起始下标
    int line_beg = 0;
    //单个参数结束下标
    int line_end = 0;
    //当前参数索引，之所以不是从0开始是因为有来自argv的原始参数
    int arg_cnt = argc - 1;
    enum state current_state = S_WAIT;

    while (current_state != S_END) {
        //从标准输入中读取
        if (read(0, p, sizeof(char)) != sizeof(char)) {
            //读取为空的话就退出
            current_state = S_END;
        } else {
            current_state = trans_state(current_state, get_type(*p));
        }
        //line_end总是指向当前字符的下一个索引，因此line_end此时也就等于Line的实际长度
        if (++line_end >= MAXSZ) {
            fprintf(2, "xargs: arguments too long.\n");
            exit(1);
        }
        switch (current_state) {
            //参数指针前移
            case S_WAIT:
                //跳过空格，空格也会被读入到lines中
                ++line_beg;
                break;
            //参数结束，参数地址存入到x_argv数组中
            case S_ARG_END:
                //储存参数起始的地址，注意这里存的是地址，x_argv本身就是一个存储地址的数组
                x_argv[arg_cnt++] = &lines[line_beg];
                line_beg = line_end;
                //替换为字符串结束符，因为此时*p为空格
                *p = '\0';
                break;
            //将参数地址存储到xargv数组中同时执行指令
            case S_ARG_LINE_END:
                x_argv[arg_cnt++] = &lines[line_beg];
                //不加break顺序执行下一个case块
            case S_LINE_END:
                //因为有空格所以要跳一下
                line_beg = line_end;
                *p = '\0';
                if (fork() == 0) {
                    exec(argv[1], x_argv);
                }
                arg_cnt = argc - 1;
                //从arg_c - 1开始的原因是x_argv[0]到x_argv[argc - 2]存放的是argv[1]到argv[argc - 1]
                clear_argv(x_argv, arg_cnt);
                //等待子进程结束
                wait(0);
                break;
            default:
                break;
        }
        ++p;
    }
    exit(0);
}

