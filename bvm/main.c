#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINE 80
#define MAX_TOKEN 30
#define MAX_PARTS 2048
#define MAX_PART_LEN 30
#define MAX_STACK 256
#define MAX_LABELS 256
#define MAX_VARS 26
#define MAX_BYTE_CODE 65535

//OPCODES
#define OP_NOP 100
#define OP_IADD 101
#define OP_ISUB 102
#define OP_IMUL 103
#define OP_IDIV 104
#define OP_IREM 105
#define OP_GOTO 106
#define OP_CALL 107
#define OP_RET 108
#define OP_ISTORE 109
#define OP_ILOAD 110
#define OP_ICMPLT 111
#define OP_ICMPGT 112
#define OP_ICMPLE 113
#define OP_ICMPGE 114
#define OP_ICMPEQ 115
#define OP_ICMPNE 116
#define OP_ICONST 117
#define OP_IAND 118
#define OP_IOR 119
#define OP_IXOR 120
#define OP_IINC 121
#define OP_IDEC 122
#define OP_ISHL 123
#define OP_ISHR 124
#define OP_DUP 125
#define OP_SWAP 126
#define OP_INEG 127
#define OP_POP 128
#define OP_POP2 129
#define OP_IFEQ 130
#define OP_IFLT 131
#define OP_IFGT 132
#define OP_IFLE 133
#define OP_IFGE 134
#define OP_LCMP 135
#define OP_SYSCALL 136
#define OP_SYSINT 137
#define OP_HALT 138

#define MATH_RAND 0
#define MATH_MIN 1
#define MATH_MAX 2
#define MATH_ABS 3
#define MATH_SRQ 4

struct TLabel{
    char lname[30];
    int ndx;
};

void load_input(char *);
void abort_1(int);
void abort_2(int);
int is_white(char);
int is_number(char *);
int is_opcode(char *);
void u_case(char *);
void s_trim(char *);
void vm_execute(void);
void eval_icmp(int, int);
void eval_if(int, int);
void eval_expr(int op);
int label_ndx(char *);

int p_code[MAX_BYTE_CODE] = {0};
int p_code_len = 0;
int pc = 0;
int is_running = 1;

//Memory for data stack
int ds_stk[MAX_STACK] = {0};
int ds_stk_prt = -1;

//Memory for return stack
int rs_stk[MAX_STACK] = {0};
int rs_stk_ptr = -1;

//Memory for variables
int _vars[MAX_VARS] = {0};

//Array of label locations
struct TLabel labels[MAX_LABELS];
int label_count;

int label_ndx(char *s){
    //Locate the label location from the labels array
    int flag = -1;
    register int x;
    u_case(s);

    for(x = 0; x < label_count;x++){
        //Compare label name with s
        if(strcmp(labels[x].lname,s) == 0){
            //Set flag to the label index found
            flag = labels[x].ndx;
            break;
        }
    }

    return flag;
}

//Data stack
void ds_push(int value){
    ds_stk_prt++;
    ds_stk[ds_stk_prt] = value;
}

int ds_pop(){
    int value;
    value = ds_stk[ds_stk_prt];
    ds_stk_prt--;
    return value;
}

int ds_size(){
    return ds_stk_prt;
}

//Call Return stack
void rs_push(int value){
    rs_stk_ptr++;
    rs_stk[rs_stk_ptr] = value;
}

int rs_pop(){
    int value;
    value = rs_stk[rs_stk_ptr];
    rs_stk_ptr--;
    return value;
}

int rs_size(){
    return rs_stk_ptr;
}

void abort_1(int ab_code){
    printf("Ben's VM Version 1.0\n");
    printf("Compile Error: %x\n", ab_code);

    switch(ab_code){
        case 100:
            printf("Use bvm input-file.asm\n");
            break;
        case 101:
            printf("There was an error reading the input file.\n");
            break;
    }

    exit(1);
}

void abort_2(int ab_code){
    printf("Ben's VM Version 1.0\n");
    printf("Runtime Error: %x\n", ab_code);

    switch(ab_code){
        case 100:
            printf("Stack underflow.\n");
            break;
    }

    is_running = 0;
    pc = p_code_len;
    exit(1);
}

int is_var(char *s){
    //Test if we have a variable
    //Variables in this version are single upper-case letters A - Z
    //Returns 1 if variable is valid otherwise 0
    int flag = 1;
    int len = strlen(s);
    char ch;
    //Check if length is one
    if(len == 1){
        ch = toupper(s[0]);
        //Check if char is alpha
        if(!isalpha(ch)){
            flag = 0;
        }
    }else{
        flag = 0;
    }

    return flag;
}

int is_number(char *s){
    //Return 1 if s is a number also works with negative numbers
    int flag = 1;
    register int x;

    for(x = 0; x< strlen(s);x++){
        //Test for negative sign
        if((x == 0) && (s[0] == '-')){
            continue;
        }
        //Check for digits
        if(!isdigit(s[x])){
            flag = 0;
            break;
        }
    }
    return flag;
}

int is_opcode(char *s){
    int retval;
    //Upper-case op-code
    u_case(s);
    //Compare
    if(strcmp(s,"NOP") == 0){
        retval = OP_NOP;
    }else if(strcmp(s,"IADD") == 0){
        retval = OP_IADD;
    }else if(strcmp(s,"IADD") == 0){
        retval = OP_IADD;
    }else if(strcmp(s,"ISUB") == 0){
        retval = OP_ISUB;
    }else if(strcmp(s,"IMUL") == 0){
        retval = OP_IMUL;
    }else if(strcmp(s,"IDIV") == 0){
        retval = OP_IDIV;
    }else if(strcmp(s,"IREM") == 0){
        retval = OP_IREM;
    }else if(strcmp(s,"GOTO") == 0){
        retval = OP_GOTO;
    }else if(strcmp(s,"CALL") == 0){
        retval = OP_CALL;
    }else if(strcmp(s,"RET") == 0){
        retval = OP_RET;
    }else if(strcmp(s,"ISTORE") == 0){
        retval = OP_ISTORE;
    }else if(strcmp(s,"ILOAD") == 0){
        retval = OP_ILOAD;
    }else if(strcmp(s,"ICMPLT") == 0){
        retval = OP_ICMPLT;
    }else if(strcmp(s,"ICMPGT") == 0){
        retval = OP_ICMPGT;
    }else if(strcmp(s,"ICMPLE") == 0){
        retval = OP_ICMPLE;
    }else if(strcmp(s,"ICMPGE") == 0){
        retval = OP_ICMPGE;
    }else if(strcmp(s,"ICMPEQ") == 0){
        retval = OP_ICMPEQ;
    }else if(strcmp(s,"ICMPEQ") == 0){
        retval = OP_ICMPNE;
    }else if(strcmp(s,"ICONST") == 0){
        retval = OP_ICONST;
    }else if(strcmp(s,"IAND") == 0){
        retval = OP_IAND;
    }else if(strcmp(s,"IOR") == 0){
        retval = OP_IOR;
    }else if(strcmp(s,"IXOR") == 0){
        retval = OP_IXOR;
    }else if(strcmp(s,"IINC") == 0){
        retval = OP_IINC;
    }else if(strcmp(s,"IDEC") == 0){
        retval = OP_IDEC;
    }else if(strcmp(s,"ISHL") == 0){
        retval = OP_ISHL;
    }else if(strcmp(s,"ISHR") == 0){
        retval = OP_ISHR;
    }else if(strcmp(s,"DUP") == 0){
        retval = OP_DUP;
    }else if(strcmp(s,"SWAP") == 0){
        retval = OP_SWAP;
    }else if(strcmp(s,"INEG") == 0){
        retval = OP_INEG;
    }else if(strcmp(s,"POP") == 0){
        retval = OP_POP;
    }else if(strcmp(s,"POP2") == 0){
        retval = OP_POP2;
    }else if(strcmp(s,"IFEQ") == 0){
        retval = OP_IFEQ;
    }else if(strcmp(s,"IFLT") == 0){
        retval = OP_IFLT;
    }else if(strcmp(s,"IFLE") == 0){
        retval = OP_IFLE;
    }else if(strcmp(s,"IFGT") == 0){
        retval = OP_IFGT;
    }else if(strcmp(s,"IFGE") == 0){
        retval = OP_IFGE;
    }else if(strcmp(s,"LCMP") == 0){
        retval = OP_LCMP;
    }else if(strcmp(s,"INT") == 0){
        retval = OP_SYSCALL;
    }else if(strcmp(s,"INT_MATH") == 0){
        retval = OP_SYSINT;
    }else if(strcmp(s,"HALT") == 0){
        retval = OP_HALT;
    }else{
        retval =-1;
    }

    return retval;
}

int is_white(char c){
    //Return 1 if char is white otherwise 0 is returned
    return (c == ' ' || c == '\t'
            || c == '\r' || c== '\n');
}

void u_case(char *s){
    //Upper-case s
    while(*s){
        *s = toupper(*s);
        s++;
    }
}

void s_trim(char *s) {
    //Used to trim leading and trailing spaces in a string.
    char * p = s;
    register int l = strlen(p);

    while(is_white(p[l - 1])) p[--l] = 0;
    while(* p && is_white(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

void load_input(char *filename){
    //This loads the file and assembles the tokens into byte-code for the VM to execute
    FILE *fp = NULL;
    int len;
    register int x;
    char strline[MAX_LINE] = {'\0'};
    char sParts[MAX_PARTS][MAX_PART_LEN];
    char s_holder[30];
    char ch = '\0';
    register int t;
    register int y;
    int addr_loc;
    char *part;
    register int parts_cnt;

    //Open file for reading
    fp = fopen(filename,"r");

    //Test for null
    if(fp == NULL){
        abort_1(101);
    }
        parts_cnt = 0;
        //Load in a string of size 80
        while(fgets(strline,MAX_LINE,fp) != NULL){
                s_trim(strline);
                //Get length of line
                len = strlen(strline);
                //Zap line feed
                if(strline[len-1] == '\n'){
                    strline[len-1] = '\0';
                }
                //Get length of strline
                len = strlen(strline);
                //Get first char of strline
                ch = strline[0];

                if((len > 0) && (ch != ';')){
                    //Get first token
                    part = strtok(strline," \t");

                    while(part != NULL){
                        if(strlen(part) > 0){
                            //Check for label symbol always at start of label ex :LabelName
                            if(part[0] == ':')
                            {
                                t = 0;
                                //Upper-case label found
                                for(y = 1;y < strlen(part);y++){
                                   s_holder[t] = toupper(part[y]);
                                   t++;
                                }
                                //Zap end of string with null-char
                                s_holder[t] = '\0';
                                //Copy the label name to the label array
                                strcpy(labels[label_count].lname,s_holder);
                                //Set the index in the label array to the number of parts already collected
                                labels[label_count].ndx = parts_cnt -1;
                                label_count++;
                            }else{
                                //Copy part to parts array
                                strcpy(sParts[parts_cnt],part);
                                parts_cnt++;
                            }
                        }
                        //Fetch next token
                        part = strtok(NULL," \t");
                    }
                }
        }
        fclose(fp);

        //Build p-code
        x = 0;
        p_code_len = 0;

        //Phase and build the byte-code.
        for(x = 0; x < parts_cnt;x++){
            //Check for op-codes
            if(is_opcode(sParts[x]) != -1){
                p_code[p_code_len] = is_opcode(sParts[x]);
                p_code_len++;
            //Check for numbers
            }else if(is_number(sParts[x])){
                p_code[p_code_len] = atoi(sParts[x]);
                p_code_len++;
            //Check for variables
            }else if(is_var(sParts[x])){
                ch = toupper(sParts[x][0] - 'A');
                p_code[p_code_len] = (int)ch;
                p_code_len++;
            }else {
                //Might be a label here
                //Try and get the label location
                addr_loc = label_ndx(sParts[x]);
                if(addr_loc > -1){
                    p_code[p_code_len] = addr_loc;
                    p_code_len++;
                }
            }
        }
}

void eval_icmp(int op, int addr){
    int lvalue;
    int rvalue;

    if(ds_size() < 1){
        abort_2(100);
    }else{
        lvalue = ds_pop();
        rvalue = ds_pop();
        switch(op){
            case OP_ICMPLT:
                if(lvalue < rvalue){
                    pc = addr;
                }
                break;
            case OP_ICMPGT:
                if(lvalue > rvalue){
                    pc = addr;
                }
                break;
            case OP_ICMPLE:
                if(lvalue <= rvalue){
                    pc = addr;
                }
                break;
            case OP_ICMPGE:
                if(lvalue >= rvalue){
                    pc = addr;
                }
                break;
            case OP_ICMPEQ:
                if(lvalue == rvalue){
                    pc = addr;
                }
                break;
            case OP_ICMPNE:
                if(lvalue != rvalue){
                    pc = addr;
                }
                break;
        }
    }
}

void eval_if(int op, int addr){
    int lvalue;

    if(ds_size() < 0){
        abort_2(100);
    }else{
        lvalue = ds_pop();

        switch(op){
            case OP_IFEQ:
                if(lvalue == 0){
                    pc = addr;
                }
                break;
            case OP_IFLT:
                if(lvalue < 0){
                    pc = addr;
                }
                break;
            case OP_IFGT:
                if(lvalue > 0){
                    pc = addr;
                }
                break;
            case OP_IFLE:
                if(lvalue <= 0){
                    pc = addr;
                }
                break;
            case OP_IFGE:
                if(lvalue >= 0){
                    pc = addr;
                }
                break;
        }
    }
}

void eval_lcmp(int op){
    int rvalue;
    int lvalue;
    int i;

    if(ds_size() < 1){
        abort_2(100);
    }else{
        rvalue = ds_pop();
        lvalue = ds_pop();

        if(lvalue > rvalue){
            i = 1;
        }else if(lvalue < rvalue){
            i = -1;
        }else{
            i = 0;
        }

        ds_push(i);
    }
}

void eval_expr(int op)
{
    int lvalue;
    int rvalue;

    if(ds_size() < 1){
        abort_2(100);
    }else{
        lvalue = ds_pop();
        rvalue = ds_pop();
        switch(op){
            case OP_IADD:
                ds_push(lvalue + rvalue);
                break;
            case OP_ISUB:
                ds_push(rvalue - lvalue);
                break;
            case OP_IMUL:
                ds_push(lvalue * rvalue);
                break;
            case OP_IDIV:
                ds_push(rvalue / lvalue);
                break;
            case OP_IREM:
                ds_push(rvalue % lvalue);
                break;
            case OP_IAND:
                ds_push(rvalue & lvalue);
                break;
            case OP_IOR:
                ds_push(rvalue | lvalue);
                break;
            case OP_IXOR:
                ds_push(rvalue % lvalue);
                break;
            case OP_ISHL:
                ds_push(rvalue << lvalue);
                break;
            case OP_ISHR:
                ds_push(rvalue >> lvalue);
                break;
            case OP_SWAP:
                ds_push(lvalue);
                ds_push(rvalue);
                break;
        }
    }
}

void eval_min_max(int op){
    int lvalue;
    int rvalue;

    if(ds_size() < 1){
        abort_2(100);
    }else{
        rvalue = ds_pop();
        lvalue = ds_pop();

        switch(op){
            case 1:
                //Min
                if(rvalue < lvalue){
                    ds_push(rvalue);
                }else{
                    ds_push(lvalue);
                }
                break;
            case 2:
                if(rvalue > lvalue){
                    ds_push(rvalue);
                }else{
                    ds_push(lvalue);
                }
                break;
        }

    }
}

void vm_execute(){
    pc = 0;
    int op;
    int v_ndx;
    int value;
    register int addr_loc;

    while(is_running && pc < p_code_len){
        //Get op-code
        op = p_code[pc];

        switch(op){
            case OP_NOP:
                break;
            case OP_HALT:
                is_running = 0;
                pc = p_code_len;
                break;
            case OP_ICONST:
                pc++;
                ds_push(p_code[pc]);
                break;
            case OP_IADD:
            case OP_ISUB:
            case OP_IMUL:
            case OP_IDIV:
            case OP_IREM:
            case OP_IAND:
            case OP_IOR:
            case OP_IXOR:
            case OP_ISHL:
            case OP_ISHR:
                eval_expr(op);
                break;
            case OP_ICMPLT:
            case OP_ICMPGT:
            case OP_ICMPLE:
            case OP_ICMPGE:
            case OP_ICMPEQ:
            case OP_ICMPNE:
                pc++;
                addr_loc = p_code[pc];
                eval_icmp(op,addr_loc);
                break;
            case OP_IFEQ:
            case OP_IFLT:
            case OP_IFLE:
            case OP_IFGT:
            case OP_IFGE:
                pc++;
                addr_loc = p_code[pc];
                eval_if(op,addr_loc);
                break;
            case OP_LCMP:
                eval_lcmp(op);
                break;
            case OP_IINC:
                pc++;
                v_ndx = p_code[pc];
                pc++;
                value = p_code[pc];
                _vars[v_ndx] += value;
                break;
            case OP_IDEC:
                pc++;
                v_ndx = p_code[pc];
                pc++;
                value = p_code[pc];
                _vars[v_ndx] -= value;
                break;
            case OP_DUP:
                if(ds_size() < 0){
                    abort_2(100);
                }
                else{
                    value = ds_pop();
                    ds_push(value);
                    ds_push(value);
                }
                break;
            case OP_INEG:
                if(ds_size() < 0){
                    abort_2(100);
                }else{
                    ds_push(-ds_pop());
                }
                break;
            case OP_POP:
                if(ds_size() < 0){
                    abort_2(100);
                }else{
                    ds_pop();
                }
                break;
            case OP_POP2:
                if(ds_size() < 1){
                    abort_2(100);
                }else{
                    ds_pop();
                    ds_pop();
                }
                break;
            case OP_GOTO:
                pc++;
                addr_loc = pc;
                pc = p_code[pc];
                break;
            case OP_CALL:
                pc++;
                addr_loc = p_code[pc];
                rs_push(pc);
                pc = addr_loc;
                break;
            case OP_ISTORE:
                pc++;
                _vars[p_code[pc]] = ds_pop();
                break;
            case OP_ILOAD:
                pc++;
                ds_push(_vars[p_code[pc]]);
                break;
            case OP_RET:
                pc = rs_pop();
                break;
            case OP_SYSINT:
                pc++;
                if(ds_size() < 0){
                    abort_2(100);
                }
                else{
                    if(p_code[pc] == MATH_RAND){
                        ds_push(rand() % ds_pop());
                    }else if(p_code[pc] == MATH_MAX){
                        eval_min_max(1);
                    }else if(p_code[pc] == MATH_MIN){
                        eval_min_max(2);
                    }else if(p_code[pc] == MATH_ABS){
                        value = ds_pop();
                        if(value < 0){
                            value = -value;
                        }
                        ds_push(value);
                    }else if(p_code[pc] == MATH_SRQ){
                        value = ds_pop();
                        ds_push(value * value);
                    }else{

                    }
                }
                break;
            case OP_SYSCALL:
                pc++;
                if(ds_size() < 0){
                    abort_2(100);
                }
                else{
                    switch(p_code[pc]){
                        case 0:
                            printf("%d",ds_pop());
                            break;
                        case 1:
                            printf("%d\n",ds_pop());
                            break;
                        case 2:
                            printf("%c",(char)ds_pop());
                            break;
                        case 3:
                            printf("%c\n",(char)ds_pop());
                            break;
                        }
                    }
                    break;
            }
        pc++;
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2) {
        abort_1(100);
    }
    else{
        srand(time(0));
        label_count = 0;
        load_input(argv[1]);
        //Check p_code_len > 0
        if(p_code_len > 0){
            //Interpret the byte-code
            vm_execute();
        }
    }

    return 0;
}
