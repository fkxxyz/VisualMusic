#pragma once
#include "PCMScaler.h"
#include <mad.h>


template <class dest_type>
inline void PCMScaler::Scale(dest_type *dest, void *src, size_t length, BaseAudioDecoder::sample_type sample_type){
	switch (sample_type){
	case BaseAudioDecoder::st_uchar:
		Scale(dest, static_cast<unsigned char *>(src), length);
		return;
	case BaseAudioDecoder::st_short:
		Scale(dest, static_cast<short *>(src), length);
		return;
	case BaseAudioDecoder::st_int:
		Scale(dest, static_cast<int *>(src), length);
		return;
	case BaseAudioDecoder::st_float:
		Scale(dest, static_cast<float *>(src), length);
		return;
	case BaseAudioDecoder::st_double:
		Scale(dest, static_cast<double *>(src), length);
		return;
	default:
		assert(0);
	}
}

inline void PCMScaler::Scale(unsigned char *dest, mad_fixed_t *src, size_t length){
	const mad_fixed_t sgn_bit_flag = static_cast<const mad_fixed_t>(1L << (sizeof(mad_fixed_t) * 8 - 1));
	for (size_t i = 0; i < length; i++){
		mad_fixed_t s = src[i];
		if (s >= MAD_F_ONE) s = MAD_F_ONE - 1;
		if (s < -MAD_F_ONE) s = -MAD_F_ONE;
		int sgn_bit = s & sgn_bit_flag;
		s &= ~sgn_bit_flag;
		s >>= MAD_F_FRACBITS - sizeof(unsigned char)*8 + 1;
		dest[i] = static_cast<unsigned char>(sgn_bit) | static_cast<unsigned char>(s);
	}
}

inline void PCMScaler::Scale(short int *dest, mad_fixed_t *src, size_t length){
	const mad_fixed_t sgn_bit_flag = static_cast<const mad_fixed_t>(1L << (sizeof(mad_fixed_t) * 8 - 1));
	for (size_t i = 0; i < length; i++){
		mad_fixed_t s = src[i];
		if (s >= MAD_F_ONE) s = MAD_F_ONE - 1;
		if (s < -MAD_F_ONE) s = -MAD_F_ONE;
		int sgn_bit = s & sgn_bit_flag;
		s &= ~sgn_bit_flag;
		s >>= MAD_F_FRACBITS - sizeof(short int)*8 + 1;
		dest[i] = static_cast<short>(sgn_bit) | static_cast<short>(s);
	}
}

inline void PCMScaler::Scale(float *dest, mad_fixed_t *src, size_t length){
	for (size_t i = 0; i < length; i++){
		mad_fixed_t s = src[i];
		if (s > MAD_F_ONE) s = MAD_F_ONE;
		if (s < -MAD_F_ONE) s = -MAD_F_ONE;
		dest[i] = static_cast<float>(s) / MAD_F_ONE;
	}
}

inline void PCMScaler::Scale(double *dest, mad_fixed_t *src, size_t length){
	for (size_t i = 0; i < length; i++){
		mad_fixed_t s = src[i];
		if (s > MAD_F_ONE) s = MAD_F_ONE;
		if (s < -MAD_F_ONE) s = -MAD_F_ONE;
		dest[i] = static_cast<double>(s) / MAD_F_ONE;
	}
}


inline void PCMScaler::Scale(unsigned char *, unsigned char *, size_t ){
	return;
}

inline void PCMScaler::Scale(short int *dest, unsigned char *src, size_t length){
	for (size_t i = 0; i < length; i++)
		dest[i] = static_cast<short int>((static_cast<int>(src[i]) - 128) << (sizeof(short int)/8 - sizeof(unsigned char)/8));
}

inline void PCMScaler::Scale(float *dest, unsigned char *src, size_t length){
	for (size_t i = 0; i < length; i++)
		dest[i] = static_cast<float>(src[i])/128 - 1.0f;
}

inline void PCMScaler::Scale(double *dest, unsigned char *src, size_t length){
	for (size_t i = 0; i < length; i++)
		dest[i] = static_cast<double>(src[i])/128 - 1.0;
}


inline void PCMScaler::Scale(unsigned char *dest, short int *src, size_t length){
	for (size_t i = 0; i < length; i++)
		dest[i] = static_cast<unsigned char>((src[i] >> (sizeof(short int)/8 - sizeof(unsigned char)/8)) + 128);
}

inline void PCMScaler::Scale(short int *, short int *, size_t ){
	return;
}

inline void PCMScaler::Scale(float *dest, short int *src, size_t length){
	for (size_t i = 0; i < length; i++)
		dest[i] = static_cast<float>(src[i])/32768;
}

inline void PCMScaler::Scale(double *dest, short int *src, size_t length){
	for (size_t i = 0; i < length; i++)
		dest[i] = static_cast<double>(src[i])/32768;
}


inline void PCMScaler::Scale(unsigned char *dest, float *src, size_t length){
	for (size_t i = 0; i < length; i++){
		float s = src[i];
		assert(s >= -1.0f && s <= 1.0f);
		if (s > 127.0f/128) s = 127.0f/128;
		dest[i] = static_cast<unsigned char>(s * 128 + 128);
	}
}

inline void PCMScaler::Scale(short int *dest, float *src, size_t length){
	for (size_t i = 0; i < length; i++){
		float s = src[i];
		assert(s >= -1.0f && s <= 1.0f);
		if (s >= 32767.0f/32768) s = 32767.0f/32768;
		dest[i] = static_cast<short int>(s * 32768);
	}
}

inline void PCMScaler::Scale(float *, float *, size_t ){
	return;
}

inline void PCMScaler::Scale(double *dest, float *src, size_t length){
	for (size_t i = 0; i < length; i++){
		float s = src[i];
		assert(s >= -1.0f && s <= 1.0f);
		dest[i] = static_cast<double>(s);
	}
}


inline void PCMScaler::Scale(unsigned char *dest, double *src, size_t length){
	for (size_t i = 0; i < length; i++){
		double s = src[i];
		assert(s >= -1.0 && s <= 1.0);
		if (s > 127.0/128) s = 127.0/128;
		dest[i] = static_cast<unsigned char>(s * 128 + 128);
	}
}

inline void PCMScaler::Scale(short int *dest, double *src, size_t length){
	for (size_t i = 0; i < length; i++){
		double s = src[i];
		assert(s >= -1.0 && s <= 1.0);
		if (s >= 32767.0/32768) s = 32767.0/32768;
		dest[i] = static_cast<short int>(s * 32768);
	}
}

inline void PCMScaler::Scale(float *dest, double *src, size_t length){
	for (size_t i = 0; i < length; i++){
		double s = src[i];
		assert(s >= -1.0 && s <= 1.0);
		dest[i] = static_cast<float>(s);
	}
}

inline void PCMScaler::Scale(double *, double *, size_t ){
	return;
}

