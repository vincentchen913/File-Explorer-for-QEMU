#include "libc.h"

void one(int fd) {
    printf("*** fd = %d\n",fd);
    printf("*** len = %d\n",len(fd));

    cp(fd,2);
}

int main(int argc, char** argv) {

    //*((uint32_t*)0xb8000)=0x07690748;
    printf("*** working");
    // printf("*** %d\n",argc);
    // printf("*** %d\n",argc);
    // for (int i=0; i<argc; i++) {
    //     printf("*** %s\n",argv[i]);
    // }
    // int fd = open("/etc/data.txt",0);
    // one(fd);
    // printf("*** close = %d\n",close(fd));

    // one(fd);
    // one(100);


    // printf("*** open again %d\n",open("/etc/data.txt",0));
    // printf("*** seek %ld\n",seek(3,17));
    
    // int id = fork();
    // printf("id = %d\n", id);
    // if (id < 0) {
    //     printf("fork failed");
    // } else if (id == 0) {
    //     /* child */
    //     printf("*** in child\n");
    //     int rc = execl("/sbin/shell","shell","a","b","c",0);
    //     printf("*** execl failed, rc = %d\n",rc);
    // } else {
    //     /* parent */
    //     uint32_t status = 42;
    //     wait(id,&status);
    //     printf("*** back from wait %ld\n",status);

    //     int fd = open("/etc/panic.txt",0);
    //     cp(fd,1);
    // }

    // auto d = Shared<Ide>::make(1);
    // auto fs = Shared<Ext2>::make(d);
    // auto root = checkDir("/",fs->root);

    // char* path = "";
    // auto node = fs->find(path);




    shutdown();
    return 0;
}
