#include <new>
#include <stdlib.h>

void* operator new(std::size_t s) { return malloc(s); }
void* operator new[](std::size_t s) { return malloc(s); }
void  operator delete(void *p) { free(p); }
void  operator delete[](void *p) { free(p); }
void* operator new(std::size_t s, const std::nothrow_t&) { return malloc(s); }
void* operator new[](std::size_t s, const std::nothrow_t&) { return malloc(s); }
void  operator delete(void *p, const std::nothrow_t&) { free(p); }
void  operator delete[](void *p, const std::nothrow_t&) { free(p); }
