#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "ext2.h"
#include "shared.h"

// Resolution: 320x200 bytes (40x25 characters)
// indexed x = [1, 40], y = [1, 25]

class Display{

public:
    char* data;
    int row;
    Display();
    //static char** entries;
    //static int index;
    Atomic<uint32_t> ref_count{0};
    void print_display();
    void test_print(volatile char* adr, const char* s, int color);
    void clear_display();
    void write_to_display(const char* s, int color);
    void write_to_display(int x, int y, const char* s, int color);
    void initDisplay();
    void displayDir();
    void print_directory(Shared<Node> dir, Shared<Ext2> ext, const char* name);
    void inc_index();
    void dec_index();
    char** get_entries();
    int get_index();
};

#endif
