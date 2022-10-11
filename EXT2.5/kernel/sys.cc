#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"
#include "config.h"
#include "ext2.h"
#include "kernel.h"
#include "threads.h"
#include "pcb.h"
#include "elf.h"

auto ide = Shared<Ide>::make(1);
auto fs = Shared<Ext2>::make(ide);

ssize_t write(int fd, void* buf, size_t nbyte){
    gheith::TCB *me = gheith::current();
    // check buf not in shared or invalid space
    if((char *) 0x00000000 <= buf && buf < (char *) 0x80000000){
        return -1;
    } else if (buf >= (char*) kConfig.localAPIC && (char *) kConfig.localAPIC + 0x1000 > buf){ //check does not start in localAPIC
        return -1;
    } else if (buf >= (char*) kConfig.ioAPIC && (char *) kConfig.ioAPIC + 0x1000 > buf){ //check does not start in ioAPIC
        return -1;
    } else if(fd < 1 || fd > 2){ // can't write to std.in
        return -1;
    } else if(me->processes->fd[fd] == nullptr){ // check if there is file there
        return -1;
    } else if (me->processes->fd[fd]->special_boi == false) { // if closed then opened
        return -1;
    } else if ((uint32_t) buf + nbyte >= kConfig.ioAPIC && (uint32_t) buf + nbyte < kConfig.ioAPIC + 4096) { //file runs into ioAPIC
        for(size_t i = 0; i < kConfig.ioAPIC - (uint32_t) buf; i++){
            Debug::printf("%c", ((char *) buf)[i]);
        }
        return kConfig.ioAPIC - (uint32_t) buf;
    } else if ((uint32_t) buf + nbyte >= kConfig.localAPIC && (uint32_t) buf + nbyte < kConfig.localAPIC + 4096) { //file runs into localAPIC
        for(size_t i = 0; i < kConfig.ioAPIC - (uint32_t) buf; i++){
            Debug::printf("%c", ((char *) buf)[i]);
        }
        return kConfig.localAPIC - (uint32_t) buf;
    } else {
        for(size_t i = 0; i < nbyte; i++){
            Debug::printf("%c", ((char *) buf)[i]);
        }
        return nbyte;
    }
}

void exit(int rc){
    gheith::TCB *me = gheith::current();
    me->processes->future->set(rc);
    stop();
}

int open(const char* fn, int flags){
    if(fs->find(fs->root, fn) == Shared<Node>{}){
        return -1;
    } else {
        gheith::TCB *me = gheith::current();
        FileDescriptor *new_fd = new FileDescriptor();
        new_fd->file = fs->find(fs->root, fn);
        new_fd->offset = 0;
        for(int i = 0; i < 10; i++){
            if(me->processes->fd[i] == nullptr){
                me->processes->fd[i] = new_fd;
                return i;
            }
        }
        return -1;
    }
    
}

ssize_t len(int fd){
    gheith::TCB *me = gheith::current();
    if(fd > 9 || fd < 0){
        return -1;
    } else if (me->processes->fd[fd] == nullptr){
        return -1;
    } else if (me->processes->fd[fd]->special_boi == true){
        return 0;
    } else {
        return me->processes->fd[fd]->file->size_in_bytes();
    }
}

// check buf does not go into apics
ssize_t read(int fd, void* buf, size_t nbyte){
    gheith::TCB *me = gheith::current();
    if(fd > 9 || fd < 0){
        return -1;
    } else if (me->processes->fd[fd] == nullptr){
        return -1;
    } else if (me->processes->fd[fd]->special_boi == true){
        return -1;
    } else if ((uint32_t) buf < 0x80000000 || ((uint32_t) buf > kConfig.ioAPIC && (uint32_t) buf < kConfig.ioAPIC + 4096) || 
    ((uint32_t) buf > kConfig.localAPIC && (uint32_t) buf < kConfig.localAPIC + 4096)){
        return -1;
    } else if ((uint32_t) buf + nbyte >= kConfig.ioAPIC && (uint32_t) buf + nbyte < kConfig.ioAPIC + 4096) {
        uint32_t value = me->processes->fd[fd]->file->read_all(me->processes->fd[fd]->offset, kConfig.ioAPIC - (uint32_t) buf, (char *) buf);
        me->processes->fd[fd]->offset += value;
        return value;
    } else if ((uint32_t) buf + nbyte >= kConfig.localAPIC && (uint32_t) buf + nbyte < kConfig.localAPIC + 4096) {
        uint32_t value = me->processes->fd[fd]->file->read_all(me->processes->fd[fd]->offset, kConfig.localAPIC - (uint32_t) buf, (char*) buf);
        me->processes->fd[fd]->offset += value;
        return value;
    } else {
        uint32_t value = me->processes->fd[fd]->file->read_all(me->processes->fd[fd]->offset, nbyte, (char *) buf);
        me->processes->fd[fd]->offset += value;
        return value;
    }
}

int sem(uint32_t initial){
    Semaphore *this_sem = new Semaphore(initial);
    gheith::TCB *me = gheith::current();
    for(int i = 0; i < 10; i++){
        if(me->processes->sp[i] == nullptr){
            me->processes->sp[i] = this_sem;
            return i + 10;
        }
    }
    return -1;
}

int up(int id){
    gheith::TCB *me = gheith::current();
    if(id > 19 || id < 10 || me->processes->sp[id-10] == nullptr){
        return -1;
    } else {
        me->processes->sp[id-10]->up();
        return 0;
    }
}

int down(int id){
    gheith::TCB *me = gheith::current();
    if(id > 19 || id < 10 || me->processes->sp[id-10] == nullptr){
        return -1;
    } else {
        me->processes->sp[id-10]->down();
        return 0;
    }
}

int close(int id){
    gheith::TCB *me = gheith::current();
    if(id < 0 || id > 29){
        return -1;
    } 
    if(id >= 0 && id < 10){
        if(me->processes->fd[id] == nullptr){
            return -1;
        } else {
            me->processes->fd[id] = nullptr;
            return 0;
        }
    }
    if(id >= 10 && id < 19){
        if(me->processes->sp[id-10] == nullptr){
            return -1;
        } else {
            me->processes->sp[id-10] = nullptr;
            return 0;
        }
    }
    if(id >= 20 && id < 29){
        if(me->processes->cp[id-20] == nullptr){
            return -1;
        } else {
            me->processes->cp[id-20] = nullptr;
            return 0;
        }
    }
    return -1;
}

int shutdown(void){
    Debug::shutdown();
    return -1;
}

int wait(int id, uint32_t *status){
    gheith::TCB *me = gheith::current();
    if((uint32_t) status < 0x80000000){
        return -1;
    }
    if(((uint32_t) status >= kConfig.ioAPIC && (uint32_t) status < kConfig.ioAPIC + 4096) || 
    ((uint32_t) status >= kConfig.localAPIC && (uint32_t) status < kConfig.localAPIC + 4096)){
        return -1;
    }
    if(id < 20 || id > 29){
        return -1;
    } else if (me->processes->cp[id-20] == nullptr){
        return -1;
    } else {
        uint32_t child_status = me->processes->cp[id-20]->future->get();
        *status = child_status;
        return 0;
    }
}

off_t seek(int fd, off_t offset){
    gheith::TCB *me = gheith::current();
    if(fd < 0 || fd > 9){
        return -1;
    } else if (me->processes->fd[fd]->special_boi == true){
        return -1;
    } else if (me->processes->fd[fd] == nullptr){
        return -1;
    } else {
        me->processes->fd[fd]->offset = offset;
        return offset;
    }
}

int fork(uint32_t pc, uint32_t esp){
    gheith::TCB *me = gheith::current();
    bool full = true;
    for(int i = 0; i < 10; i++){
        if(me->processes->cp[i] == nullptr){
            full = false;
            break;
        }
    }
    if(full == false){
        gheith::TCB *tcb = make_children([pc, esp]{
            switchToUser(pc, esp, 0);
        });
        //shallow copy
        for(int i = 0; i < 10; i++){
            tcb->processes->fd[i] = me->processes->fd[i];
            tcb->processes->sp[i] = me->processes->sp[i];
        }
        // deep copy virtual private space
        uint32_t current_address = 0x80000000;
        for(int i = 512; i < 1024; i++){
            uint32_t pde = me->pd[i]; // parent page directory
            if(((pde & 1) == 1)){ // check parent's pde's present bit
                uint32_t *pt = (uint32_t *) ((pde >> 12) * 4096);
                for(int j = 0; j < 1024; j++){
                    if(((pt[j] & 1) == 1) && (uint32_t) current_address != kConfig.localAPIC && (uint32_t) current_address != kConfig.ioAPIC){
                        uint32_t *new_frame = (uint32_t *) PhysMem::alloc_frame();
                        gheith::map(tcb->pd, (uint32_t) current_address, (uint32_t) new_frame);
                        memcpy((uint32_t *)new_frame, (uint32_t*)current_address, 4096);
                    }
                    current_address += 4096;
                }
            } else {
                current_address += 4096 * 1024;
            }
        }
        for(int i = 0; i < 10; i++){
            if(me->processes->cp[i] == nullptr){
                me->processes->cp[i] = tcb->processes;
                gheith::schedule(tcb);
                return i + 20;
            }
        }
    }
    return -1;
}

void delete_virtual_memory(uint32_t* pd){
    uint32_t current_address = 0x80000000;
    for(int i = 512; i < 1024; i++){
        uint32_t pde = pd[i]; // parent page directory
        if(((pde & 1) == 1)){ // check parent's pde's present bit
            uint32_t *pt = (uint32_t *) ((pde >> 12) * 4096);
            for(int j = 0; j < 1024; j++){
                if(((pt[j] & 1) == 1) && (uint32_t) current_address != kConfig.localAPIC && (uint32_t) current_address != kConfig.ioAPIC){
                    gheith::unmap(pd, current_address);
                    //PhysMem::dealloc_frame((uint32_t) pt[j]);
                }
                current_address += 4096;
            }
        } else {
            current_address += 4096 * 1024;
        }
    }
    vmm_on(getCR3());
}

int execl(const char* path, const char** argv){
    if(fs->find(fs->root, path) == Shared<Node>{}){
        return -1;
    } else {
        // save arguments
        Shared<Node> file = fs->find(fs->root, path);
        int argc = 0; // number of arguments
        int index = 0; //current index of argv
        char *start_of_strings = (char *) 0xfee01000;
        uint32_t *user_esp = (uint32_t*) 0xefffe008;
        while(argv[index] != nullptr){
            argc++;
            const char* this_string = argv[index];
            int string_index = 0;
            while(this_string[string_index] != '\0'){
                string_index++;
            }
            string_index++; //null terminator
            memcpy(start_of_strings, this_string, string_index);
            //Debug::printf("%x", start_of_strings[0]);
            user_esp[index] = (uint32_t) start_of_strings;
            start_of_strings += string_index;
            index++;
        }
        char *saved_args = new char[start_of_strings - (char *) 0xfee01000]; 
        memcpy(saved_args, (char *) 0xfee01000, start_of_strings - (char *) 0xfee01000); // saved arguments
        user_esp = (uint32_t*) 0xefffe000;
        user_esp[0] = argc; // num of args
        user_esp[1] = 0xefffe008; // pointer to string array aka argv
        char *saved_user_stack = new char[8 + 4 * argc]; //4 bytes + 4 bytes + argc * 4
        memcpy(saved_user_stack, (uint32_t*) 0xefffe000, 8 + 4 * argc); 
        gheith::TCB *me = gheith::current();
        // delete virtual memory space
        delete_virtual_memory(me->pd);
        //restore
        memcpy((uint32_t*) 0xefffe000, saved_user_stack, 8 + 4 * argc);
        memcpy((char *) 0xfee01000, saved_args, start_of_strings - (char *) 0xfee01000);
        delete[] saved_args;
        delete[] saved_user_stack;
        //load elf file
        uint32_t entry_num = ELF::load(file);
        // switchToUser
        switchToUser(entry_num,(uint32_t) user_esp, 0);
        return -1;
    }
}

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    uint32_t *user_esp = (uint32_t*) frame[3];
    if(eax == 0){
        exit(user_esp[1]);
    } else if (eax == 10){
        return open((char *) user_esp[1], user_esp[2]);
    } else if (eax == 11){
        return len(user_esp[1]);
    } else if (eax == 12){
        return read(user_esp[1], (char *) user_esp[2], user_esp[3]);
    } else if (eax == 1){
        return write(user_esp[1], (char *) user_esp[2], user_esp[3]);
    } else if (eax == 3) {
        return sem(user_esp[1]);
    } else if (eax == 4){
        return up(user_esp[1]);
    } else if (eax == 5){
        return down(user_esp[1]);
    } else if (eax == 6){
        return close(user_esp[1]);
    } else if (eax == 13){
        return seek(user_esp[1], user_esp[2]);
    } else if (eax == 2){
        return fork(frame[0], frame[3]);
    } else if (eax == 9){
        return execl((const char *)user_esp[1], (const char **) &user_esp[2]);
    } else if (eax == 8){
        return wait(user_esp[1], (uint32_t*) user_esp[2]);
    } else if (eax == 7){
        return shutdown();
    } else {
        Debug::panic("system call not written yet: %d", eax);
    }
    return 0;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
