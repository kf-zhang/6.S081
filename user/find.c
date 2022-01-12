#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"



char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}


void 
search(char* path,char* fname){
    char buf[512];
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if( fstat(fd,&st)<0 ){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if( strlen(path) + 1 + DIRSIZ + 1 > sizeof( buf ) ){
        printf("find: path too long\n");
        return;
    }
    strcpy(buf,path);
    p = buf + strlen(buf);
    *p = '/';
    p++;

    while( read(fd,&de,sizeof(de))== sizeof(de) ){
        if(de.inum == 0)
            continue;
        if( strcmp(de.name,".")==0 || strcmp(de.name,"..")==0 )
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        stat(buf,&st);
        switch (st.type)
        {
            case T_FILE:
            case T_DEVICE:
                if( !strcmp(de.name,fname) )
                    printf("%s\n",buf);
                break;
            case T_DIR:
                search(buf,fname);
                break;
            default:
                break;
        }
    }

    close(fd);

}

int
main(int argc,char* argv[]){
    if( argc<3 ){
        printf("The argument is too little");
        exit(-1);
    }

    char* dir =  argv[1];
    char* file = argv[2];

    search(dir,file);
    exit(0);
}