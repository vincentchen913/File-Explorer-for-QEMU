#include "display.h"
#include "ext2.h"
#include "shared.h"
#include "debug.h"
#include "display.h"
//#include "directory.h"



int row = 0;
int index = 0;
char** entries = nullptr;
int count = 0;

Display::Display(){
    data = new char[2000];
    row = 0;
}

void Display::print_display(){
    for(int i = 0; i < 2000; i++){
        volatile char *out = (volatile char*)0xB8000 + i;
        *out = data[i];
    }
}
void Display::write_to_display(const char* s, int color){
    int start = row * 80;
    int i = 0;
    while( *s != 0 )
    {
        data[start + i] = *s++;
        data[start + i + 1] = color;
        i += 2;
    }
    row += i % 40 == 0 ? i / 40 : i / 40 + 1;
}

void Display::test_print(volatile char* adr, const char* s, int color){
    volatile char *out = adr;
    while( *s != 0 ){
        *out++ = *s++;
        *out++ = color;
    }
}

// Prints a given string at a given position
void Display::write_to_display(int x, int y, const char* s, int color) {
    char* out = (char*) data + 80 * y;
    out += x * 2;
    while (*s != 0) {
        *out++ = *s++;
        *out++ = color;
    }
}

void Display::displayDir() {
    //
}

void Display::initDisplay() {
    //
}

char* int_to_string(uint32_t inode){
    int len = 0;
    uint32_t copy = inode;
    while(copy > 0){
        len++;
        copy /= 10;
    }
    char* result = new char[len + 1];
    result[len] = '\0';
    int i = len - 1;
    while(i >= 0){
        result[i] = (char)inode % 10;
        inode /= 10;
        i--;
    }
    return result;
}

void Display::print_directory(Shared<Node> dir, Shared<Ext2> ext, const char* name) {
    if(name[0] == '.' && name[1] == '\0'){
        return;
    }
    clear_display();
    count = 0;
    index = 0;
    ASSERT(dir != nullptr);
    ASSERT(ext != nullptr);
    ASSERT(dir->is_dir());
    uint32_t offset = 0;
    char* buffer = new char[dir->size_in_bytes()];
    Debug::printf("%d\n", dir->size_in_bytes());
    dir->read_all(offset, dir->size_in_bytes(), buffer);
    const char* title = "Printing contents of current directory:";
    test_print((volatile char*)0xB8000, title, 0x07);
    if(name[0] == '.' && name[1] == '.'){
        test_print((volatile char*)0xB8000 + 80, "root", 0x0c);
    }else{
        test_print((volatile char*)0xB8000 + 80, name, 0x0c);
    }

    test_print((volatile char*)0xB8000 + 1840, "Selected Item: ", 0x07);
    test_print((volatile char*)0xB8000 + 1920, "                                        ", 0x07);
    test_print((volatile char*)0xB8000 + 1920, ".", 0x07);
    
    entries = new char*[dir->entry_count()];
    Debug::printf("%d\n", dir->entry_count());

    
    int index = 2;

    while((uint32_t) count < dir->entry_count()){
        uint16_t total_size = *(uint16_t*)&buffer[offset + 4];
        uint8_t name_length = *(uint8_t*)&buffer[offset + 6];

        char* s = new char[name_length + 1];
        s[name_length] = '\0';
        for(int i = 0; i < name_length; i++){
            s[i] = buffer[offset + 8 + i];
        }
        auto node = ext->find(dir, (const char*)s);
        int color;

        if(node->is_file()){
            color = 0x09;
        }else if(node->is_dir()){
            color = 0x0c;
        }else if(node->is_symlink()){
            color = 0x0a;
        }else{
            color = 0x07;
        }

        entries[index - 2] = s;

        offset += total_size;
        test_print((volatile char*)0xB8000 + 80 * index + 4, s, color);
        Debug::printf("%s\n", s);
        index++;
        count++;
    }

}


void Display::clear_display(){
    for(uint32_t i = 0xb8000 + 80; i < 0xb8000 + 1840; i+=80){
        //data[i] = 0x20;
        test_print((volatile char*)i, "                                        ", 0x07);
    }


}

void Display::inc_index(){
    if(index < count - 1){
        index++;
    }
}

void Display::dec_index(){
    if(index > 0){
        index--;
    }
}

char** Display::get_entries(){
    return entries;
}

int Display::get_index(){
    return index;
}
