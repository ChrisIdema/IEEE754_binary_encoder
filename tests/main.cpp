#include <cstdio>
#include <cstdint>

#include "IEEE754_binary_encoder.h"

#define RED   "\033[0;31m"
#define GREEN "\033[0;32m"
#define NC    "\033[0m"

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

bool IEE754_binary32_encode_TEST()
{
    float testFloat = 1.23f;
    uint8_t testbuffer[sizeof(float)] = {0};

    IEE754_binary32_encode(testFloat, testbuffer);

    return IEE754_binary32_decode(testbuffer) == testFloat;
}

int main(void)
{
    printf("Testing IEEE754_binary_encoder...\n");

    if (!IEE754_binary32_encode_TEST())
    {
        fprintf(stderr, RED "[ERROR]" NC ": IEE754_binary32_encode_TEST() failed!\n");
        return -1; // Error: Process completed with exit code 255.
    }
    else
    {
        printf(GREEN "[SUCCESS]" NC ": IEE754_binary32_encode_TEST() succeeded!\n");
    }

    return 0;
}
