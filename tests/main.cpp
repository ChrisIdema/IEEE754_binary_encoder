#include <cstdio>
#include <cstdint>

#define RED "\e[0;31m"
#define NC "\e[0m"

int64_t add(int32_t a, int32_t b)
{
    //return a+b; // potential overflow
    return int64_t(a)+b; // no overflow
}

bool test0()
{
    return add(1,1)==2;
}

bool test1()
{
    return add(INT32_MAX,1) == (1ULL << 31);
}

int main(void)
{
    printf("Hello world!\n");

    printf("1+1=%lld\n",add(1,1));

    if (!test0())
    {
        fprintf(stderr, RED "[ERROR]" NC ": test0() failed!\n");
        return -1; // Error: Process completed with exit code 255.
    }

    
    if (!test1())
    {
        fprintf(stderr, RED "[ERROR]" NC ": test1() failed!\n");
        return -1; // Error: Process completed with exit code 255.
    }

    return 0;
}
