#include "../kernel/types.h"
#include "./user.h"
#include "../kernel/param.h"

#define MAXSZ 512
enum state {
    S_WAIT,                         //�ȴ��������룬��״̬Ϊ��ʼ״̬���ߵ�ǰ�ַ�Ϊ�ո�
    S_ARG,                          //������
    S_ARG_END,                      //��������
    S_ARG_LINE_END,                 //����в����Ļ��У�"arg\n"
    S_LINE_END,                     //���Ϊ�ո�Ļ��У�"arg \n"
    S_END                           //����, EOF
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

//cur_typeΪ��Ҫ��ȡ���ַ�
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

//�������б�����Ԫ��ȫ��Ϊ�գ����ڻ���ʱ���¸������
//x_argv�ǲ���ָ�����飬begΪ��յ���ʼ�±�
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
    //����ָ�����飬ȫ����ʼ��Ϊ��ָ��
    char *x_argv[MAXARG] = {0};
    //�洢ԭ�еĲ���
    for (int i = 1;i < argc;i++) {
        x_argv[i - 1] = argv[i];
    }
    //����������ʼ�±�
    int line_beg = 0;
    //�������������±�
    int line_end = 0;
    //��ǰ����������֮���Բ��Ǵ�0��ʼ����Ϊ������argv��ԭʼ����
    int arg_cnt = argc - 1;
    enum state current_state = S_WAIT;

    while (current_state != S_END) {
        //�ӱ�׼�����ж�ȡ
        if (read(0, p, sizeof(char)) != sizeof(char)) {
            //��ȡΪ�յĻ����˳�
            current_state = S_END;
        } else {
            current_state = trans_state(current_state, get_type(*p));
        }
        //line_end����ָ��ǰ�ַ�����һ�����������line_end��ʱҲ�͵���Line��ʵ�ʳ���
        if (++line_end >= MAXSZ) {
            fprintf(2, "xargs: arguments too long.\n");
            exit(1);
        }
        switch (current_state) {
            //����ָ��ǰ��
            case S_WAIT:
                //�����ո񣬿ո�Ҳ�ᱻ���뵽lines��
                ++line_beg;
                break;
            //����������������ַ���뵽x_argv������
            case S_ARG_END:
                //���������ʼ�ĵ�ַ��ע���������ǵ�ַ��x_argv�������һ���洢��ַ������
                x_argv[arg_cnt++] = &lines[line_beg];
                line_beg = line_end;
                //�滻Ϊ�ַ�������������Ϊ��ʱ*pΪ�ո�
                *p = '\0';
                break;
            //��������ַ�洢��xargv������ͬʱִ��ָ��
            case S_ARG_LINE_END:
                x_argv[arg_cnt++] = &lines[line_beg];
                //����break˳��ִ����һ��case��
            case S_LINE_END:
                //��Ϊ�пո�����Ҫ��һ��
                line_beg = line_end;
                *p = '\0';
                if (fork() == 0) {
                    exec(argv[1], x_argv);
                }
                arg_cnt = argc - 1;
                //��arg_c - 1��ʼ��ԭ����x_argv[0]��x_argv[argc - 2]��ŵ���argv[1]��argv[argc - 1]
                clear_argv(x_argv, arg_cnt);
                //�ȴ��ӽ��̽���
                wait(0);
                break;
            default:
                break;
        }
        ++p;
    }
    exit(0);
}

