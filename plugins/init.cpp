#include <API.hpp>
#include <sys/stat.h>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <string>
#include <sys/unistd.h>
#include <vector>

extern "C" int _getpid(){return getId();}

extern "C" int _isatty(int fd){
    return fd < 3;
}

extern "C" int _unlink(const char* name) {
    puts("unlinking file");
    return -1;
}

extern "C" int _open(const char* name, const char* mode){
    return vmOpen(name, mode);
};

extern "C" void _close(int fp){
    vmClose(fp);
};

extern "C" uint32_t _lseek(int fh, int off, int whence){
    return vmLSeek(fh, off, whence);
}

extern "C" int _read(int fh, const unsigned char *buffer, unsigned int length, int mode) {
    return vmRead(fh, buffer, length);
}

extern "C" void _kill(__pid_t pid, int signal) {
    printf("kill -%d %d\n", signal, pid);
}

extern "C" int _write(int fh, const unsigned char *buffer, unsigned int length, int mode) {
    return vmWrite(fh, buffer, length);
}

extern "C" int system(const char* s) {
    return vmSystem(s) == 0;
}

extern "C" char* getenv(const char* s) {
    return (char*) getString(s, nullptr);
}

extern "C" void _exit(int i){
    vmExit(i);
    while (true)
        yield();
}

extern "C" int _fstat(int fd, struct stat *st) {
    if (fd < 3) {
        st->st_mode = S_IFCHR;
        return  0;
    }
    return -1;
}

extern "C" void __libc_init_array(void);
extern uint32_t __data_section_table;
extern uint32_t __data_section_table_end;
extern uint32_t __bss_section_table;
extern uint32_t __bss_section_table_end;
int main(int argc, char** argv);

void data_init(uint32_t romstart, uint32_t start, uint32_t len) {
    len >>= 2;
    auto pulDest = reinterpret_cast<uint32_t*>(start);
    auto pulSrc = reinterpret_cast<uint32_t*>(romstart);
    for (uint32_t i = 0; i < len; ++i) pulDest[i] = pulSrc[i];
}

void bss_init(uint32_t start, uint32_t len) {
    len >>= 2;
    auto pulDest = reinterpret_cast<uint32_t*>(start);
    for (uint32_t i = 0; i < len; ++i) pulDest[i] = 0;
}

extern "C" void _init_internal() {
    uint32_t LoadAddr, ExeAddr, SectionLen;
    uint32_t *SectionTableAddr;

    SectionTableAddr = &__data_section_table;

    while (SectionTableAddr < &__data_section_table_end) {
        LoadAddr = *SectionTableAddr++;
        ExeAddr = *SectionTableAddr++;
        SectionLen = *SectionTableAddr++;
        data_init(LoadAddr, ExeAddr, SectionLen);
    }
    while (SectionTableAddr < &__bss_section_table_end) {
        ExeAddr = *SectionTableAddr++;
        SectionLen = *SectionTableAddr++;
        bss_init(ExeAddr, SectionLen);
    }

    __libc_init_array();
    uint32_t argc = popMessage();
    std::vector<std::string> args;
    args.resize(argc);
    char* arg[argc];
    for (uint32_t i = 0; i < argc; ++i) {
	args[i] = getMessageArg(i);
	arg[i] = args[i].data();
    }
    _exit(main(argc, arg));
}

#ifndef VM_HEAP_SIZE
#define VM_HEAP_SIZE 1
#endif

extern uint32_t _pvHeapStart;
extern "C" __attribute__ ((section(".bin_header"))) uint32_t const isr_vector[] = {
    0x54524944,
    uint32_t((uintptr_t)&_pvHeapStart) + VM_HEAP_SIZE*1024,
    uint32_t((uintptr_t)&_init_internal)
};

