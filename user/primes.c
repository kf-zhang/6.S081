#include "kernel/types.h"
#include"user/user.h"





void
stage(int in){

    int num ;
    if( read(in,&num,4) > 0 ){

        fprintf(1,"prime %d\n",num);

        int p[2];//p[0] read p[1] write
        pipe(p);
        if( fork() == 0){

            close(p[1]);
            stage(p[0]);
            close(p[0]);

        }else{
            int buf;
            close(p[0]);
            while( read( in ,&buf,4) > 0 ){
                if(buf%num!=0)
                    write(p[1],&buf,4);
            }
            //Must close pipe before wait, or the parent will wait for the child process to exit while the child process is waiting for the parent to close pipe
            close(p[1]);
            wait(0);
        }
    }
}


void
f(int in){
    int buf;
    int size;
    size = read(in,&buf,4);
    fprintf(1,"size:%d \n read: %d",size,buf);
}


int
main(int argc,char* argv[])
{
    int p[2];//p[0] read p[1] write
    pipe(p);   
    if( fork() == 0 ){
        close(p[1]);
        stage(p[0]);
        close(p[0]);
    }else{
        close(p[0]);
        for(int i=2;i<=35;i++)
            write(p[1],&i,4);
        //Must close pipe before wait, or the parent will wait for the child process to exit while the child process is waiting for the parent to close pipe
        close(p[1]);
        wait(0);
    }
    // fprintf(1,"%d exit\n",getpid());
    exit(0);
}