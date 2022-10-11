#include "ext2.h"

// 1. a function that will open a directory
// 2. a function to read that directory
// 3. a way to parse through the entries in the directory and get NAMES to be displayed, need a way to dynamically store this information since
// directories have variable number of entries
// 4. what if we were able to implement a way to go backwards? a back arrow? cd .., if you are at the root, then you would just point to yourself 

struct direc_entry{
    uint32_t inode;
    uint16_t entry_size;
    uint8_t name_length;
};

// Shared<Node> openDir(const char *dirName, Shared<Ext2> fs){
//     if(fs->find(fs->root, dirName) == Shared<Node>{}){
//         return Shared<Node>{};
//     } else {
//         Shared<Node> this_dir = fs->find(fs->root, dirName);
//         return this_dir;
//     }
// }

// for optimization purposes, we can limit directory/file names to 32 byte maximum, if less than, then we fill with spaces?

char* readDir(Shared<Node> dir, Shared<Ext2> fs) {
    // check to see if dir is even a directory
    if(dir == Shared<Node>{}){
        Debug::panic("Dir is not a directory\n");
    }
    if(!dir->is_dir()){
        Debug::panic("Dir is not a directory\n");
    }

    //int num_entries = dir->entry_count();
    //char *directory_names = new char[num_entries * 16];// directory names < 16 bytes or equal to 16 bytes including null terminator
    char *directory_names = (char *) 0x80000000;
    uint32_t indexer = 0;
    int iteration = 1;
    while(indexer != dir->size_in_bytes()){
        direc_entry db;
        dir->read_all(indexer, 7, (char *) (&db));
        char * temp_buff = new char[db.name_length + 1];
        dir->read_all(indexer + 8, db.name_length, temp_buff);
        temp_buff[db.name_length] = '\0';
        memcpy(directory_names, temp_buff, db.name_length);
        //Debug::printf("Iteration %d: %s\n", iteration, directory_names);       
        directory_names += 16; // is this correct?
        indexer += db.entry_size;
        iteration++;
    }
    directory_names = (char *) 0x80000000;
    return directory_names;
};
