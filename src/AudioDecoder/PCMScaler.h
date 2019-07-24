#pragma once
#include "BaseAudioDecoder.h"
#include <mad.h>
#include <cstddef>

class PCMScaler {
public:
	static void Scale(unsigned char *dest, mad_fixed_t *src, size_t length);
	static void Scale(short int *dest, mad_fixed_t *src, size_t length);
	static void Scale(float *dest, mad_fixed_t *src, size_t length);
	static void Scale(double *dest, mad_fixed_t *src, size_t length);

	static void Scale(unsigned char *dest, unsigned char *src, size_t length);
	static void Scale(short int *dest, unsigned char *src, size_t length);
	static void Scale(float *dest, unsigned char *src, size_t length);
	static void Scale(double *dest, unsigned char *src, size_t length);

	static void Scale(unsigned char *dest, short int *src, size_t length);
	static void Scale(short int *dest, short int *src, size_t length);
	static void Scale(float *dest, short int *src, size_t length);
	static void Scale(double *dest, short int *src, size_t length);

	static void Scale(unsigned char *dest, float *src, size_t length);
	static void Scale(short int *dest, float *src, size_t length);
	static void Scale(float *dest, float *src, size_t length);
	static void Scale(double *dest, float *src, size_t length);

	static void Scale(unsigned char *dest, double *src, size_t length);
	static void Scale(short int *dest, double *src, size_t length);
	static void Scale(float *dest, double *src, size_t length);
	static void Scale(double *dest, double *src, size_t length);

};

#include "PCMScaler.inl"

