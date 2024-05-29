#ifndef IEE754_FLOAT_H
#define IEE754_FLOAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void IEE754_float64_encode( double x, uint8_t out[8] );
double IEE754_float64_decode( uint8_t out[8] );
void IEE754_float32_encode( float x, uint8_t out[4] );
float IEE754_float32_decode( uint8_t out[4] );

#ifdef __cplusplus
}
#endif

#endif
