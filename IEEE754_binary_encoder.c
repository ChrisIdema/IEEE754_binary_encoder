#include "IEEE754_binary_encoder.h"

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>

void IEE754_float64_encode( double x, uint8_t out[8] )
{
	bool sign = signbit(x) != 0;
	uint16_t exponent;
	uint64_t fraction;

	if( isinf( x ) )
	{
		exponent = ((1<<11)-1);
		fraction = 0;
	}
	else if( isnan( x ) )
	{
		// A NaN has all exponent bits set to 1 and and at least one bit of the fraction/mantissa set to 1. The value of the sign bit is ignored.
		// Commonly the most significant bit is used to mark the NaN as quiet (1) or signaling (0).
		// There is no portable way to create a signalling NaN and there is no guarantee that its usage will raise an exception.
		// Other fraction bits and the sign bit can be used as a payload. For instance to communicate the cause of the NaN.
		// Payloads are not guaranteed to be preserved/propagate and cannot be read in a portable way.
		exponent = ((1<<11)-1);
		fraction = 1ULL<<(DBL_MANT_DIG-2); // quiet NaN without a payload
	}
	else if(!isnormal(x))
	{
		if (x == 0.0f) // +0 or -0
		{
			exponent = 0;
			fraction = 0;
		}
		else
		{
			if(sign)
			{
				x = -x;
			}

			exponent = 0;

			int e = 0;
			double dfraction = frexp( x, &e );
			fraction = (uint64_t)(dfraction * (1ULL << (e + (DBL_MAX_EXP-2) + (DBL_MANT_DIG-1))));
		}
	}
	else
 	{
		if(sign)
		{
			x = -x;
		}

		int e = 0;
		fraction = (uint64_t)(frexp( x, &e ) * ((uint64_t)1uLL<<DBL_MANT_DIG));
		exponent = (uint16_t)(e + (DBL_MAX_EXP-2));
  	}

	out[0] = ((sign << 7) & 0x80) |
			 ((exponent >> 4) & 0x7F);
	out[1] = ((exponent << 4) & 0xF0) |
			((fraction >> (8*6)) & 0x0F) ;
	out[2] = (fraction >> (8*5)) & 0xFF;
	out[3] = (fraction >> (8*4)) & 0xFF;
	out[4] = (fraction >> (8*3)) & 0xFF;
	out[5] = (fraction >> (8*2)) & 0xFF;
	out[6] = (fraction >> (8*1)) & 0xFF;
	out[7] = (fraction >> (8*0)) & 0xFF;
}

double IEE754_float64_decode( uint8_t out[8] )
{
	bool sign = (out[0] & 0x80) != 0;
	int16_t exponent =   ((out[0] & 0x7F) << 4 )
					   | ((out[1]) >> 4 ) ;
	uint64_t fraction =   ( (uint64_t)(out[1]&0x0F) << (8*6) )
						| ( (uint64_t)(out[2]) << (8*5) )
						| ( (uint64_t)(out[3]) << (8*4) )
						| ( (uint64_t)(out[4]) << (8*3) )
						| ( (uint64_t)(out[5]) << (8*2) )
						| ( (uint64_t)(out[6]) << (8*1) )
						| ( (uint64_t)(out[7]) << (8*0) );

	double dfraction;

	if(exponent == 0) // subnormal number
  	{
		if (fraction != 0) // non-zero subnormal number
		{
			dfraction = (double)fraction;
			exponent -= (DBL_MAX_EXP-2) + (DBL_MANT_DIG-1);
		}
		else
		{
			//exponent -= (FLT_MAX_EXP-2);
			return sign ? -0.0f : 0.0f;
		}
  	}
	else if (exponent == ((1<<11)-1))
	{
		if( fraction == 0){ // Infinity
			return sign ? -INFINITY : INFINITY;
		}
		else // NaN
		{
			return sign ? -nanf("") : nanf(""); // return quiet NaN
		}
	}
	else
	{
		// only normal finite numbers have an implicit 1
		fraction |= (uint64_t)1ULL<<(DBL_MANT_DIG-1);
		dfraction = (double)fraction / ( (uint64_t)1ULL<<DBL_MANT_DIG);
		exponent -= (DBL_MAX_EXP-2);
	}

	return ldexp(dfraction, exponent) * (sign ? -1 : 1);
}


void IEE754_float32_encode( float x, uint8_t out[4] )
{
	bool sign = signbit(x) != 0;
	uint8_t exponent;
	uint32_t fraction;

	if( isinf( x ) )
	{
		exponent = ((1<<8)-1);
		fraction = 0;
	}
	else if( isnan( x ) )
	{
		// A NaN has all exponent bits set to 1 and and at least one bit of the fraction/mantissa set to 1. The value of the sign bit is ignored.
		// Commonly the most significant bit is used to mark the NaN as quiet (1) or signaling (0).
		// There is no portable way to create a signalling NaN and there is no guarantee that its usage will raise an exception.
		// Other fraction bits and the sign bit can be used as a payload. For instance to communicate the cause of the NaN.
		// Payloads are not guaranteed to be preserved/propagate and cannot be read in a portable way.
		exponent = ((1<<8)-1);
		fraction = 1ULL<<(FLT_MANT_DIG-2); // quiet NaN without a payload
	}
	else if(!isnormal(x))
	{
		if (x == 0.0f) // +0 or -0
		{
			exponent = 0;
			fraction = 0;
		}
		else
		{
			if(sign)
			{
				x = -x;
			}

			exponent = 0;

			int e = 0;
			float ffraction = frexpf( x, &e );
			fraction = (uint32_t)(ffraction * (1ULL << (e + (FLT_MAX_EXP-2) + (FLT_MANT_DIG-1))));
		}
	}
	else
 	{
		if(sign)
		{
			x = -x;
		}

		int e = 0;
		fraction = (uint32_t)(frexp( x, &e ) * ((uint32_t)1<<FLT_MANT_DIG));
		exponent = (uint8_t)(e + (FLT_MAX_EXP-2));
		if( e + (FLT_MAX_EXP-2) > ((1<<8)-1) )
		{
			exponent = ((1<<8)-1);
			fraction = 0;
		}
  	}

	out[0] = ( ( sign << 7 ) & 0x80 )
			| ( ( exponent >>  1 ) & 0x7F );
	out[1] =  ( ( exponent <<  7 ) & 0x80 )
			| ( ( fraction >> 16 ) & 0x7F );
	out[2] =    ( fraction >>  8 ) & 0xFF;
	out[3] =      fraction         & 0xFF;
}

float IEE754_float32_decode( uint8_t out[4] )
{
	bool sign = (out[0] & 0x80) != 0;
	int16_t exponent = ( ( out[0] << 1 ) & 0xFE )
					 | ( ( out[1] >> 7 ) & 0x01 );
	uint32_t fraction = ( (uint32_t)( out[1] & 0x7F ) << 16 )
					  | ( (uint32_t)( out[2] & 0xFF ) <<  8 )
					  |   (uint32_t)( out[3] & 0xFF );

	float ffraction;

	if(exponent == 0) // subnormal number
  	{
		if (fraction != 0) // non-zero subnormal number
		{
			ffraction = (float)fraction;
			exponent -= (FLT_MAX_EXP-2) + (FLT_MANT_DIG-1);
		}
		else
		{
			//exponent -= (FLT_MAX_EXP-2);
			return sign ? -0.0f : 0.0f;
		}
  	}
	else if (exponent == ((1<<8)-1))
	{
		if( fraction == 0){ // Infinity
			return sign ? -INFINITY : INFINITY;
		}
		else // NaN
		{
			return sign ? -nanf("") : nanf(""); // return quiet NaN
		}
	}
	else
	{
		// only normal finite numbers have an implicit 1
		fraction |= (uint32_t)1UL<<(FLT_MANT_DIG-1);
		ffraction = (float)fraction / ( (uint32_t)1UL<<FLT_MANT_DIG);
		exponent -= (FLT_MAX_EXP-2);
	}

	return ldexpf(ffraction, exponent) * (sign ? -1 : 1);
}
