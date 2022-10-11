#include "elf.h"
#include "debug.h"

uint32_t ELF::load(Shared<Node> file) {
    ElfHeader *elf_header = new ElfHeader;
    file->read_all(0, 52, (char *) elf_header);
    if(elf_header->magic0 != 0x7f || elf_header->magic1 != 0x45 || elf_header->magic2 != 0x4c || elf_header->magic3 != 0x46){
        Debug::panic("There is an invalid magic number for this ELF file.");
    }
    ProgramHeader *ph_array = new ProgramHeader[elf_header->phnum];
    file->read_all(elf_header->phoff, elf_header->phentsize * elf_header->phnum, (char *) ph_array);
    for(uint32_t i = 0; i < elf_header->phnum; i++){
        if(ph_array[i].type == 1){
            file->read_all(ph_array[i].offset, ph_array[i].filesz, (char *) ph_array[i].vaddr);
        }
    }
    return elf_header->entry;
}


















































#if 0
    ElfHeader hdr;

    file->read(0,hdr);

    uint32_t hoff = hdr.phoff;

    for (uint32_t i=0; i<hdr.phnum; i++) {
        ProgramHeader phdr;
        file->read(hoff,phdr);
        hoff += hdr.phentsize;

        if (phdr.type == 1) {
            char *p = (char*) phdr.vaddr;
            uint32_t memsz = phdr.memsz;
            uint32_t filesz = phdr.filesz;

            Debug::printf("vaddr:%x memsz:0x%x filesz:0x%x fileoff:%x\n",
                p,memsz,filesz,phdr.offset);
            file->read_all(phdr.offset,filesz,p);
            bzero(p + filesz, memsz - filesz);
        }
    }

    return hdr.entry;
#endif
