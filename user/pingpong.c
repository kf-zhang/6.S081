#include "kernel/types.h"
#include"user/user.h"


int
main(int argc,char* argv[])
{
    int p1[2];//p1[0] read p1[1] write
    int p2[2];//p2[0] read p2[1] write

    pipe(p1);
    pipe(p2);
    if( fork() == 0){
        uint8 buf[3]={0,1,2};
        read(p1[0],buf,1);
        fprintf(1,"%d: received ping\n",getpid());
        write(p2[1],buf,1);
    }
    else{
        uint8 buf[3]={4,5,6};
        write(p1[1],buf,1);
        read(p2[0],buf,1);
        fprintf(1,"%d: received pong\n",getpid());
    }

    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    exit(0);
}