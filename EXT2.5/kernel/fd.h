#include "ext2.h"

struct FileDescriptor{
    Shared<Node> file;
    uint32_t offset;
    bool special_boi = false;
    Atomic<uint32_t> ref_count{0};
    //bool has_been_closed;
};