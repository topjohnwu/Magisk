#include <new>
#include <cstdlib>

/* Override libc++ new implementation
 * to optimize final build size */

void* operator new(std::size_t s) { return std::malloc(s); }
void* operator new[](std::size_t s) { return std::malloc(s); }
void  operator delete(void *p) { std::free(p); }
void  operator delete[](void *p) { std::free(p); }
void* operator new(std::size_t s, const std::nothrow_t&) noexcept { return std::malloc(s); }
void* operator new[](std::size_t s, const std::nothrow_t&) noexcept { return std::malloc(s); }
void  operator delete(void *p, const std::nothrow_t&) noexcept { std::free(p); }
void  operator delete[](void *p, const std::nothrow_t&) noexcept { std::free(p); }
