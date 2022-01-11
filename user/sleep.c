#include "kernel/types.h"
#include"user/user.h"



int
main(int argc,char* argv[])
{
    int i = 0;
    if( argc < 2){
        fprintf(2, "sleep: unknown arguments\n");
        exit(1);
    }
    
    i = atoi(argv[1]);
    sleep(i);
    exit(0);
}