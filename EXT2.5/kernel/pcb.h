#include "semaphore.h"
#include "fd.h"
#include "future.h"


struct PCB {
    Shared<FileDescriptor> fd[10];
    Shared<Semaphore> sp[10];
    PCB *cp[10];
    Shared<Future<uint32_t>> future = Shared<Future<uint32_t>>::make();
    PCB(){
        FileDescriptor *temp = new FileDescriptor();
        temp->special_boi= true;
        fd[0] = temp;
        FileDescriptor *temp1 = new FileDescriptor();
        temp1->special_boi = true;
        fd[1] = temp1;
        FileDescriptor *temp2 = new FileDescriptor();
        temp2->special_boi = true;
        fd[2] = temp2;
    };
};



