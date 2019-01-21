#include <new>
#include <stdlib.h>

/* Override libc++ new implementation
 * to optimize final build size */

void* operator new(std::size_t s) { return malloc(s); }
void* operator new[](std::size_t s) { return malloc(s); }
void  operator delete(void *p) { free(p); }
void  operator delete[](void *p) { free(p); }
void* operator new(std::size_t s, const std::nothrow_t&) noexcept { return malloc(s); }
void* operator new[](std::size_t s, const std::nothrow_t&) noexcept { return malloc(s); }
void  operator delete(void *p, const std::nothrow_t&) noexcept { free(p); }
void  operator delete[](void *p, const std::nothrow_t&) noexcept { free(p); }
