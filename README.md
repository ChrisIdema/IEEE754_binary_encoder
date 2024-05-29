# IEEE754_binary_encoder
[![build and test](https://github.com/ChrisIdema/IEEE754_binary_encoder/actions/workflows/build.yml/badge.svg?branch=unit_tests)](https://github.com/ChrisIdema/IEEE754_binary_encoder/actions/workflows/build.yml)

A portable C library for serializing/deserializing float and double values.
Uses standard functions instead of making assumptions about the platform's binary format (endianness etc.)

Functions
---------

```c
void IEE754_binary64_encode( double, char[8] );
double IEE754_binary64_decode( char[8] );
void IEE754_binary32_encode( float, char[4] );
float IEE754_binary32_decode( char[4] );
```

Limitations
-----------

Signaling NaN is turned to quiet NaN. Payloads of NaN are not preserved. Signbit of NaN is preserved.

Todo
----
* add shortcut for platforms that have known binary format and endianness
* implement arbitrary floating point size (custom mantissa and exponent sizes, but smaller than double)
* NaN payload? Who uses that?
