# IEEE754_binary_encoder
[![build and test](https://github.com/ChrisIdema/IEEE754_binary_encoder/actions/workflows/build.yml/badge.svg?branch=unit_tests)](https://github.com/ChrisIdema/IEEE754_binary_encoder/actions/workflows/build.yml)

A C library for converting float and double values to binary

Functions
---------

It contains the following function prototypes:

```c
void IEE754_binary64_encode( double, char[8] );
double IEE754_binary64_decode( char[8] );
void IEE754_binary32_encode( float, char[4] );
float IEE754_binary32_decode( char[4] );
```

Limitations
-----------

Subnormal floating point values (values really close to zero) are truncated to zero for simplicity reasons.
I may change this someday.
