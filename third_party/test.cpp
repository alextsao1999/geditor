#include <stdio.h>
#include <rpmalloc.h>
#include <exception>

class Base {
public:
    static void* operator new(size_t size) noexcept(false) {
        printf("base alloc ");
        return ::operator new(size);
    }
    static void operator delete(void* pMemory)noexcept {
        ::operator delete(pMemory);
    }
    int value = 1234;
};
class Sub : public Base {
public:
    int test = 55;
};
class Subs : public Sub {
public:
    int aa = 444;
};
int main (int argc, char * const argv[]) {
    Base *base = new Subs();
    printf("%d", base->value);
    return 0;
}

