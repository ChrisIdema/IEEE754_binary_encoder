#include "IEEE754_binary_encoder.h"

#include <cstdio>
//#include <cstdint>

#include <cfloat>  // FLT_TRUE_MIN
#include <cmath>   // isnan, signbit
#include <cstdlib> // rand
#include <cstring> // memcmp

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std; // for std::isnan, needed in gcc

#define RED   "\033[0;31m"
#define GREEN "\033[0;32m"
#define NC    "\033[0m"


//helper functions:

#if defined(_MSC_VER)
// Sleep() expects an argument in milliseconds
#define SLEEP(time) Sleep((time) * 1000)
#else
// sleep() expects an argument in seconds so use usleep
#define SLEEP(time) usleep((time) * 1000000)
#endif


//nan==nan, nan!=-nan, inf==inf, inf!=-inf
static bool float32_strict_compare(float f1, float f2)
{
    return  (isnan(f1) && isnan(f2) && signbit(f1) == signbit(f2)) ||
            (f1 == f2 && signbit(f1) == signbit(f2));
}

//nan==nan, nan!=-nan, inf==inf, inf!=-inf
static bool float64_strict_compare(double f1, double f2)
{
    return  (isnan(f1) && isnan(f2) && signbit(f1) == signbit(f2)) ||
            (f1 == f2 && signbit(f1) == signbit(f2));
}


//64-bit version of rand()
static uint64_t rand64()
{
//RAND_MAX is at least 32767
return 	((uint64_t)(rand() & 0x000F)) << 60 |
		((uint64_t)(rand() & 0x7FFF)) << 45 |
		((uint64_t)(rand() & 0x7FFF)) << 30 |
		((uint64_t)(rand() & 0x7FFF)) << 15 |
		((uint64_t)(rand() & 0x7FFF)) << 0;
}

//32-bit version of rand()
static uint32_t rand32()
{
//RAND_MAX is at least 32767
return 	((uint32_t)(rand() & 0x0003)) << 30 |
		((uint32_t)(rand() & 0x7FFF)) << 15 |
		((uint32_t)(rand() & 0x7FFF)) << 0;
}

static uint16_t rand15()
{
	//RAND_MAX is at least 32767
	return 	rand() & 0x7FFF;
}

static void print_byte_array(const uint8_t* buffer, size_t size, bool line_ending)
{
    while(size)
    {
        if(size == 1)
        {
            printf("%02X", *buffer);
            if(line_ending)
            {
                puts("");
            }
        }
        else
        {
            printf("%02X-", *buffer);
            ++buffer;
        }
        --size;
    }
}

//test cases:

bool IEE754_float32_special_cases_TEST()
{
    bool success = true;
    bool compare;

    uint8_t buffer[4];
    float in,out;
    bool sign;

    //edge/corner cases
    float in_array[] = {1.0f, 1.0f + FLT_EPSILON, 1.0f + FLT_EPSILON*2, 1.0f + FLT_EPSILON*3, 0.0f, FLT_TRUE_MIN, FLT_TRUE_MIN*2, FLT_TRUE_MIN*3, FLT_MIN, FLT_MAX, nanf(""),
    #ifdef INFINITY
    INFINITY
    #endif
    };

    for(int i = 0; i < sizeof(in_array) / sizeof(in_array[0]); ++i)
    {
        sign = false;
        do
        {
            in = sign ? -in_array[i] : in_array[i];
            IEE754_float32_encode(in, buffer);
            //print_byte_array(buffer, sizeof(buffer), true);
            out = IEE754_float32_decode(buffer);
            compare = float32_strict_compare(in, out);
            printf("%+.*g %c= %+.*g\n", FLT_DECIMAL_DIG - 1, out, compare ? '=' : '!' , FLT_DECIMAL_DIG - 1, in );
            success = success && compare;
            sign = !sign;
        }
        while(sign);
    }

    return success;
}


bool IEE754_float32_fuzzing_TEST()
{
    bool sign = false;
    bool compare;
    bool success = true;
    uint8_t buffer[4];

    srand(0);
    for(int i = 0; i < 1000; ++i)
    {
        int16_t exponent = rand15() % (1<<8);
        //printf("exponent=%d\n", exponent);

        //exponent = 255; // test NaN

        uint32_t fraction = rand32() % (1ULL<<(FLT_MANT_DIG-1));
        //printf("fraction=%llu\n", fraction);

        sign = rand15() & 1;

        buffer[0] = ((sign << 7) & 0x80) |
                    ((exponent >>  1) & 0x7F);
        buffer[1] = ((exponent <<  7) & 0x80) |
                    ((fraction >> 16) & 0x7F);
        buffer[2] =  (fraction >>  8);
        buffer[3] =  (fraction >>  0);


        float f = IEE754_float32_decode(buffer);
        uint8_t out[4];

        IEE754_float32_encode(f, out);

        // if (i==100) // test fail
        // {
        // 	out[0] ^= 0xff;
        // }

        if (exponent == 255 && fraction !=0) // +-NaN (quiet or signalling, with or without payload)
        {
            //f = 1; // test fail
            compare = isnan(f) && (signbit(f)!=0) == sign; // tests decode

            if (!compare)
            {
                printf("NaN decode failed\n");
                printf("%g, ", f);
                print_byte_array(buffer, sizeof(buffer), true);
            }
            else
            {
                //out[0] ^= 0xff; // test fail
                f = IEE754_float32_decode(out);
                compare = compare && isnan(f) && (signbit(f)!=0) == sign; // tests decode-encode-decode

                if (!compare)
                {
                    printf("NaN encode failed");
                    print_byte_array(out, sizeof(out), true);
                }
            }
        }
        else
        {
            compare = memcmp(buffer, out, sizeof(buffer)) == 0;

            if (!compare)
            {
                print_byte_array(buffer, sizeof(buffer), false);
                printf(" != ");
                print_byte_array(out, sizeof(out), true);
            }
        }

        success = success && compare;
        if (!success)
        {
            printf("fuzzzing failed at index: %d\n", i);
            break; // break at first fail
        }
    }

    return success;
}


bool IEE754_float64_special_cases_TEST()
{
    bool success = true;
    bool compare;

    uint8_t buffer[8];
    double in, out;

    double in_array[] = {1.0, 1.0 + DBL_EPSILON, 1.0 + DBL_EPSILON*2, 1.0 + DBL_EPSILON*3, 0.0, DBL_TRUE_MIN, DBL_TRUE_MIN*2, DBL_TRUE_MIN*3, DBL_MIN, DBL_MAX, nan(""),
    #ifdef HUGE_VAL
    HUGE_VAL
    #endif
    };


    for(int i = 0; i < sizeof(in_array) / sizeof(in_array[0]); ++i)
    {
        bool sign = false;
        do
        {
            in = sign ? -in_array[i] : in_array[i];
            IEE754_float64_encode(in, buffer);
            out = IEE754_float64_decode(buffer);
            compare = float64_strict_compare(in, out);
            printf("%+.*g %c= %+.*g\n", DBL_DECIMAL_DIG, out, compare ? '=' : '!' , DBL_DECIMAL_DIG, in );
            success = success && compare;
            sign = !sign;
        }
        while(sign);
    }

    return success;
}


bool IEE754_float64_fuzzing_TEST()
{
    bool sign = false;
    bool compare;
    bool success = true;
    uint8_t buffer[8];

    srand(0);
    for(int i = 0; i < 10000; ++i)
    {
        int16_t exponent = rand15() % (1<<11);

        //exponent = 2047; // test NaN

        uint64_t fraction = rand64() % (1ULL<<(DBL_MANT_DIG-1));

        sign = rand15() & 1;

        buffer[0] = (( sign << 7 ) & 0x80) |
                    (( exponent >> 4) & 0x7F);
        buffer[1] =  exponent << 4 |
                    (fraction >> (8*6)) & 0x0F;
        buffer[2] =  fraction >> (8*5);
        buffer[3] =  fraction >> (8*4);
        buffer[4] =  fraction >> (8*3);
        buffer[5] =  fraction >> (8*2);
        buffer[6] =  fraction >> (8*1);
        buffer[7] =  fraction >> (8*0);


        double d = IEE754_float64_decode(buffer);
        uint8_t out[8];

        IEE754_float64_encode(d, out);

        // if (i==500) // test fail
        // {
        // 	out[0] ^= 0xff;
        // }

        if (exponent == 2047 && fraction !=0) // +-NaN (quiet or signalling, with or without payload)
        {
            //d = 1; // test fail
            compare = isnan(d) && (signbit(d)!=0) == sign; // tests decode

            if (!compare)
            {
                printf("NaN decode failed\n");
                printf("%lg, ", d);

                print_byte_array(buffer, sizeof(buffer), true);
            }
            else
            {
                //out[0] ^= 0xff; // test fail
                d = IEE754_float64_decode(out);
                compare = compare && isnan(d) && (signbit(d)!=0) == sign; // tests decode-encode-decode

                if (!compare)
                {
                    printf("NaN encode failed");
                    print_byte_array(out, sizeof(out), true);
                }
            }
        }
        else
        {
            compare = memcmp(buffer, out, sizeof(buffer)) == 0;

            if (!compare)
            {
                print_byte_array(buffer, sizeof(buffer), false);
                printf(" != ");
                print_byte_array(out, sizeof(out), true);
            }
        }

        success = success && compare;
        if (!success)
        {
            printf("fuzzzing failed at index: %d\n", i);
            break; // break at first fail
        }
    }

    return success;
}


int main(void)
{
    int res = 0;
    printf("Testing IEEE754_binary_encoder...\n");

    printf("starting IEE754_float32_special_cases_TEST()...\n");
    if (!IEE754_float32_special_cases_TEST())
    {
        fflush(stdout); // flush stdout buffer before writing to stderr(unbuffered), to preserve order
        SLEEP(0.100);
        fprintf(stderr, RED "[ERROR]" NC ": IEE754_float32_special_cases_TEST() failed!\n");
        res = -1; // Error: Process completed with exit code 255
        //return res;
    }
    else
    {
        printf(GREEN "[SUCCESS]" NC ": IEE754_float32_special_cases_TEST() succeeded!\n");
    }

    printf("starting IEE754_float32_fuzzing_TEST()...\n");
    if (!IEE754_float32_fuzzing_TEST())
    {
        fflush(stdout); // flush stdout buffer before writing to stderr(unbuffered), to preserve order
        SLEEP(0.100);
        fprintf(stderr, RED "[ERROR]" NC ": IEE754_float32_fuzzing_TEST() failed!\n");
        res = -1; // Error: Process completed with exit code 255
        //return res;
    }
    else
    {
        printf(GREEN "[SUCCESS]" NC ": IEE754_float32_fuzzing_TEST() succeeded!\n");
    }

    printf("starting IEE754_float64_special_cases_TEST()...\n");
    if (!IEE754_float64_special_cases_TEST())
    {
        fflush(stdout); // flush stdout buffer before writing to stderr(unbuffered), to preserve order
        SLEEP(0.100);
        fprintf(stderr, RED "[ERROR]" NC ": IEE754_float64_special_cases_TEST() failed!\n");
        res = -1; // Error: Process completed with exit code 255
        //return res;
    }
    else
    {
        printf(GREEN "[SUCCESS]" NC ": IEE754_float64_special_cases_TEST() succeeded!\n");
    }

    printf("starting IEE754_float64_fuzzing_TEST()...\n");
    if (!IEE754_float64_fuzzing_TEST())
    {
        fflush(stdout); // flush stdout buffer before writing to stderr(unbuffered), to preserve order
        SLEEP(0.100);
        fprintf(stderr, RED "[ERROR]" NC ": IEE754_float64_fuzzing_TEST() failed!\n");
        res = -1; // Error: Process completed with exit code 255
        //return res;
    }
    else
    {
        printf(GREEN "[SUCCESS]" NC ": IEE754_float64_fuzzing_TEST() succeeded!\n");
    }

    return res;
}
