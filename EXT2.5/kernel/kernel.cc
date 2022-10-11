#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "elf.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "semaphore.h"
#include "display.h"
#include "directory.h"
#include "keyboard.h"
#include "shared.h"

Shared<Node> checkFile(const char* name, Shared<Node> node) {
    //CHECK(node != nullptr);
    //CHECK(node->is_file());
    //Debug::printf("*** file %s is ok\n",name);
    return node;
}

Shared<Node> getFile(Shared<Ext2> fs, Shared<Node> node, const char* name) {
    return checkFile(name,fs->find(node,name));
}

Shared<Node> checkDir(const char* name, Shared<Node> node) {
    //Debug::printf("*** checking %s\n",name);
    //CHECK (node != nullptr);
    //CHECK (node->is_dir());
    //Debug::printf("*** directory %s is ok\n",name);
    return node;
}

Shared<Node> getDir(Shared<Ext2> fs, Shared<Node> node, const char* name) {
    return checkDir(name,fs->find(node,name));
}

char * getEntryName(int entry_num, char *names){
    int index = 0 + entry_num * 16;
    char *this_entry = new char[16];
    while(names[index] != '\0'){
        this_entry[index - entry_num * 16] = names[index];
        index++;
    }
    return this_entry;
}

// *((uint32_t*)0xb8000)=0x07690748;
// *((uint32_t*)0xb8004)=0x07410720;
// *((uint32_t*)0xb8008)=0x07730775;
// *((uint32_t*)0xb800c)=0x07690774;
// *((uint32_t*)0xb8010)=0x0720076e;

int BORDER_X = 1;
int BORDER_Y = 1;
int INDENT = 2;

// void printSample(Display* dis) {
//     dis->printToDisplay(BORDER_X, BORDER_Y, "root");
//     const char* names[] = {"data", "etc", "fortunes", "goodbye", "hello"};
//     int num_entries = 0;
//     while (names[num_entries]) num_entries++;
//     for (int i = 0; i < num_entries; i++) {
//         dis->printToDisplay(BORDER_X + INDENT, BORDER_Y + i + 1, names[i]);
//     }
// }

void kernelMain(void) {

    Semaphore* s = new Semaphore(0);
    Shared<Display> dis = Shared<Display>::make();

    auto d = Shared<Ide>::make(1);
    auto fs = Shared<Ext2>::make(d);
    auto root = checkDir("/",fs->root);

    // const char* title = "Printing contents of current directory:";
    // dis->write_to_display(title, 0x07);
    // dis->print_display();

    // const char* name = "root";
    //dis->print_directory(root, fs, "root");
    //dis->print_display();
    //dis->clear_display();
    kb_init(dis, root, fs);
    //auto data = fs->find(root, name);


    
    s->down();

    // //Debug::printf("*** loading init\n");
    // uint32_t e = ELF::load(init);
    // //Debug::printf("*** entry %x\n",e);
    // auto userEsp = (uint32_t *) 0xefffe000;
    //Debug::printf("*** user esp %x\n",userEsp);
    // Current state:
    //     - %eip points somewhere in the middle of kernelMain
    //     - %cs contains kernelCS (CPL = 0)
    //     - %esp points in the middle of the thread's kernel stack
    //     - %ss contains kernelSS
    //     - %eflags has IF=1
    // Desired state:
    //     - %eip points at e
    //     - %cs contains userCS (CPL = 3)
    //     - %eflags continues to have IF=1
    //     - %esp points to the bottom of the user stack
    //     - %ss contain userSS
    // User mode will never "return" from switchToUser. It will
    // enter the kernel through interrupts, exceptions, and system
    // calls.
    // userEsp[0] = 1;
    // userEsp[1] = (uint32_t) &userEsp[2];
    // memcpy((char *) 0xfee01000, "init", 5);
    // userEsp[2] = 0xfee01000;


    //switchToUser(e,(uint32_t) userEsp,0);
    //Debug::panic("*** implement switchToUser in machine.S\n");
}

