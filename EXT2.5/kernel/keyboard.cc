#include "idt.h"
#include "machine.h"
#include "smp.h"
#include "debug.h"
#include "display.h"

Shared<Display> dis;
Shared<Node> dir;
Shared<Ext2> ext;

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)


void kb_init(Shared<Display> display, Shared<Node> directory, Shared<Ext2> ext2) {
    dis = display;
    dir = directory;
    ASSERT(dir != nullptr);
    ext = ext2;
    dis->print_directory(dir, ext, "root");
    const char* name = dis->get_entries()[dis->get_index()];
    dis->test_print((volatile char*)0xB8000 + 1840, "Selected Item: ", 0x07);
    dis->test_print((volatile char*)0xB8000 + 1920, "                                        ", 0x07);
    dis->test_print((volatile char*)0xB8000 + 1920, name, 0x07);
    // register to port
    // read what is in port 21 into fd
    outb(PIC1_DATA, 0xfd);
    // IDT::interrupt(9, (uint32_t) &sysHandler_);
    IDT::interrupt(9, (uint32_t) &kbHandler_);
}

extern "C" void kbHandler() {
    uint8_t status = inb(0x64);
    //auto output_buffer_status = status & 0x10000000;

    if (status & 0x00000001) {
        // Debug::printf("yo mama 3\n");
        uint8_t scancode = inb(0x60);
        if (scancode == 0x11) { // up -> w
            dis->dec_index();
            const char* name = dis->get_entries()[dis->get_index()];
            dis->test_print((volatile char*)0xB8000 + 1840, "Selected Item: ", 0x07);
            dis->test_print((volatile char*)0xB8000 + 1920, "                                        ", 0x07);
            dis->test_print((volatile char*)0xB8000 + 1920, name, 0x07);
        } else if (scancode == 0x1F) { // down -> s
            dis->inc_index();
            const char* name = dis->get_entries()[dis->get_index()];
            dis->test_print((volatile char*)0xB8000 + 1840, "Selected Item: ", 0x07);
            dis->test_print((volatile char*)0xB8000 + 1920, "                                        ", 0x07);
            dis->test_print((volatile char*)0xB8000 + 1920, name, 0x07);
        } else if (scancode == 0x12) { // enter
            //Debug::printf("Enter\n");
            //dis->clear_display();
            const char* name = dis->get_entries()[dis->get_index()];
            Debug::printf(name);
            ASSERT(dir != nullptr);
            auto child = ext->find(dir, name);
            ASSERT(child != nullptr);
            dis->print_directory(child, ext, name);
            // Debug::printf(name);
            // const char* title = "Printing contents of current directory:";
            // dis->test_print((volatile char*)0xB8000, title, 0x07);
            // dis->test_print((volatile char*)0xB8000 + 80, name, 0x0c);
            // char* buffer = new char[dir->size_in_bytes()];
            // dir->read_block(4096, buffer);

        } else {
            // Debug::printf("yo mama\n");
            //dis->write_to_display((const char*)"not recognized", 0x07);
        }

    }

    outb(PIC1, PIC1); // finished handling the interrupt
}