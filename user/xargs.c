#include"kernel/types.h"
#include"kernel/param.h"
#include"user/user.h"

#define BUFLEN (1024)

int
main(int argc,char* argv[]){
    char* exe;
    char* args[MAXARG]={0};
    char readBuf[BUFLEN];
    char argBuf[BUFLEN];
    int size;
    int idx = 0;
    int readBufIdx = 0;
    if(argc < 2){
        fprintf(2,"xargs: you should specify the command\n");
        exit(1);
    }
    exe = argv[1];
    for(int i = 1;i<argc;i++){
        args[idx++] = argv[i];
    }
    args[idx] = argBuf;

    while( (size = read(0,readBuf,BUFLEN) )>0 ){
        for(int i=0;i<size;i++ )
            if( readBuf[i] == '\n' ){
                if( fork()==0 ){
                    argBuf[readBufIdx++] = 0;
                    exec(exe,args);
                }
                else{
                    readBufIdx = 0;
                    wait(0);
                }
            }
            else{
                argBuf[readBufIdx++] = readBuf[i];
            }
    }

    exit(0);
}