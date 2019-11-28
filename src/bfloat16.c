// bfloat16.c
//
// convert between float32 and bfloat16
// public domain + no warranty
// milahu at gmail.com



// background
//
// bfloat16 is simply a "truncated float32"
// = lossy compression, with only 7 bit mantissa
//
// bfloat16 = SEEEEEEE EMMMMMMM
//  float32 = SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM
//
// 		float f = 3.1415;
// 		int *ip = NULL;
// 		ip = (int *)&f;
// 		printf("f = %f\ni = %i = 0x%x\n", f, *ip, *ip);
//
// f = 3.141500
// i = 1078529622 = 0x40490e56
//                  ^^^^^^
//                  bfloat16
//
// bfloat16 came to hype with "machine learning",
// but is also used for sensor data.
// some exotic processors have hardware support for bfloat16.



// how to use
//
// #include "bfloat16.c"
// bfloat16_init();
//
//
//
// // convert one number - pass by value
//
//  float32 f1 = 3.1415;
// bfloat16 b1 = bfloat16_from_float32(f1);
//
// bfloat16 b2 = 0x4049;
//  float32 f2 = float32_from_bfloat16(b2);
//
// printf("%1.4f --> 0x%04x\n", f1, b1);
// printf("%1.4f <-- 0x%04x\n", f2, b2);
//
//
//
// // convert one number - pass by reference
//
//  float32 f3 = 3.1415;
// bfloat16 b3 = 0;
// float32_to_bfloat16(&f3, &b3, 1);
//
// bfloat16 b4 = 0x4049;
//  float32 f4 = 0;
// bfloat16_to_float32(&b4, &f4, 1);
//
// printf("%1.4f --> 0x%04x\n", f3, b3);
// printf("%1.4f <-- 0x%04x\n", f4, b4);
//
//
//
// // convert many numbers, the fast way
//
//  float32 f5[2] = {2.7182, 1.4142};
// bfloat16 b5[2] = {0};
// float32_to_bfloat16(f5, b5, 2);
//
// bfloat16 b6[2] = {0x402d, 0x3fb5};
//  float32 f6[2] = {0};
// bfloat16_to_float32(b6, f6, 2);
//
// printf("%1.4f --> 0x%04x\n", f5[0], b5[0]);
// printf("%1.4f <-- 0x%04x\n", f6[0], b6[0]);
//
// printf("%1.4f --> 0x%04x\n", f5[1], b5[1]);
// printf("%1.4f <-- 0x%04x\n", f6[1], b6[1]);



#include <stdint.h>  // uint16_t, int8_t
#include <stdlib.h>  // malloc, free, size_t
#include <string.h>  // memset
#include <stdbool.h> // bool



// pretty type names

typedef uint16_t bfloat16;
typedef float     float32;



// function pointers for the current byte order
// set on runtime by bfloat16_init()

void (*float32_to_bfloat16)(float32*, bfloat16*, size_t) = NULL;
void (*bfloat16_to_float32)(bfloat16*, float32*, size_t) = NULL;



// the four bfloat16 functions
// stolen from tensorflow bfloat16.cc

void float32_to_bfloat16_be(float32 *src, bfloat16 *dst, size_t size)
{
	uint16_t *s = (uint16_t *)src;
	uint16_t *d = (uint16_t *)dst; // maybe remove

	for (; size != 0; s += 2, d++, size--) {
		// big endian byte order
		*d = s[0];
	}
}

void float32_to_bfloat16_le(float32 *src, bfloat16 *dst, size_t size)
{
	uint16_t *s = (uint16_t *)src;
	uint16_t *d = (uint16_t *)dst; // maybe remove

	for (; size != 0; s += 2, d++, size--) {
		// little endian byte order
		*d = s[1];
	}
}

void bfloat16_to_float32_be(bfloat16* src, float32* dst, size_t size)
{
	uint16_t *s = (uint16_t *)src; // maybe remove
	uint16_t *d = (uint16_t *)dst;

	for (; size != 0; s++, d += 2, size--) {
		// big endian byte order
		d[0] = *s;
		d[1] = 0;
	}
}

void bfloat16_to_float32_le(bfloat16* src, float32* dst, size_t size)
{
	uint16_t *s = (uint16_t *)src; // maybe remove
	uint16_t *d = (uint16_t *)dst;

	for (; size != 0; s++, d += 2, size--) {
		// little endian byte order
		d[0] = 0;
		d[1] = *s;
	}
}



// convenience functions - pass by value

float32 float32_from_bfloat16(bfloat16 b)
{
	float32 f = 0.0;
	bfloat16_to_float32(&b, &f, 1);
	return f;
}

bfloat16 bfloat16_from_float32(float32 f)
{
	bfloat16 b = 0;
	float32_to_bfloat16(&f, &b, 1);
	return b;
}



// convenience functions - pass by value - integer vs bfloat16

int int_from_bfloat16(bfloat16 b)
{
	float32 f = 0.0;
	bfloat16_to_float32(&b, &f, 1);
	return (int) f;
}

bfloat16 bfloat16_from_int(int i)
{
	float32 f = (float) i;
	bfloat16 b = 0;
	float32_to_bfloat16(&f, &b, 1);
	return b;
}



// detect byte order at runtime
// http://esr.ibiblio.org/?p=5095#comment-415728
// this usually does not generate any code at all with GCC even with -O1
// ignore byte orders other than LE and BE

static inline bool bfloat16_is_big_endian()
{
	const uint16_t endianness = 256; // 0b 00000001 00000000
	return *(const uint8_t *)&endianness;
}



// init function pointers

void bfloat16_init()
{
	// global variables:
	//   float32_to_bfloat16
	//   bfloat16_to_float32

	// detect byte order at runtime
	if (bfloat16_is_big_endian())
	{
		float32_to_bfloat16 = &float32_to_bfloat16_be;
		bfloat16_to_float32 = &bfloat16_to_float32_be;
	}
	else
	{
		float32_to_bfloat16 = &float32_to_bfloat16_le;
		bfloat16_to_float32 = &bfloat16_to_float32_le;
	}
}
