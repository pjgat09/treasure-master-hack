#include <mmintrin.h>  //MMX
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
#include <smmintrin.h> //SSE4.1
#include <nmmintrin.h> //SSE4.2
//#include <ammintrin.h> //SSE4A
#include <immintrin.h> //AVX
//#include <zmmintrin.h> //AVX512

#include "../tm_8bit/data_sizes.h"

#if defined(__GNUC__)
#define _mm256_set_m128i(vh, vl) \
        _mm256_castpd_si256(_mm256_insertf128_pd(_mm256_castsi256_pd(_mm256_castsi128_si256(vl)), _mm_castsi128_pd(vh), 1))
#endif

void generate_rng_table(uint16 * rng_table)
{
    for (int i = 0; i <= 0xFF; i++)
    {
        for (int j = 0; j <= 0xFF; j++)
        {
            unsigned int rngA = i;
            unsigned int rngB = j;
            
            uint8 carry = 0;
            
            rngB = (rngB + rngA) & 0xFF;
            
            rngA = rngA + 0x89;
            carry = rngA > 0xFF ? 1 : 0;
            rngA = rngA & 0xFF;
            
            rngB = rngB + 0x2A + carry;
            carry = rngB > 0xFF ? 1 : 0;
            rngB = rngB & 0xFF;
            
            rngA = rngA + 0x21 + carry;
            carry = rngA > 0xFF ? 1 : 0;
            rngA = rngA & 0xFF;
            
            rngB = rngB + 0x43 + carry;
            carry = rngB > 0xFF ? 1 : 0;
            rngB = rngB & 0xFF;
            
            rng_table[(i * 0x100) + j] = (rngA << 8) | rngB;
        }
    }
}

uint8 run_rng(uint16 * rng_seed, uint16 * rng_table)
{
	uint16 result = rng_table[*rng_seed];
	*rng_seed = result;

	return ((result >> 8) ^ (result)) & 0xFF;
}


void generate_regular_rng_values(uint16 * regular_rng_values, uint16 * rng_seed, uint16 * rng_table)
{
    for (int i = 0; i < 0x10000; i++)
    {
		*rng_seed = i;
        for (int j = 0; j < 128; j++)
        {
            regular_rng_values[i*128 + (127 - j)] = run_rng(rng_seed, rng_table);
        }
    }
}

void generate_alg0_values(uint16 * alg0_values, uint16 * rng_seed, uint16 * rng_table)
{
    for (int i = 0; i < 0x10000; i++)
    {
        *rng_seed = i;
        for (int j = 0; j < 128; j++)
        {
            alg0_values[i*128 + (127 - j)] = (run_rng(rng_seed, rng_table) >> 7) & 0x01;
        }
    }
}

void generate_alg6_values(uint16 * alg6_values, uint16 * rng_seed, uint16 * rng_table)
{
    for (int i = 0; i < 0x10000; i++)
    {
        *rng_seed = i;
        for (int j = 0; j < 128; j++)
        {
            alg6_values[i*128 + j] = run_rng(rng_seed, rng_table) & 0x80;
        }
    }
}

void generate_alg2_values(uint8 * alg2_values, uint16 * rng_seed, uint16 * rng_table)
{
    for (int i = 0; i < 0x10000; i++)
    {
		for (int j = 0; j < 132; j++)
		{
			alg2_values[i*32 + j] = 0;
		}
        *rng_seed = i;
        alg2_values[i*32+30] = (run_rng(rng_seed, rng_table) & 0x80) >> 7;
    }
}

void generate_alg5_values(uint8 * alg5_values, uint16 * rng_seed, uint16 * rng_table)
{
    for (int i = 0; i < 0x10000; i++)
    {
		for (int j = 0; j < 32; j++)
		{
			alg5_values[i*32 + j] = 0;
		}
        *rng_seed = i;
		alg5_values[i*32+30] = run_rng(rng_seed, rng_table) & 0x80;
    }
}

void generate_seed_forward_1(uint16 * values, uint16 * rng_table)
{
	for (int i = 0; i < 0x10000; i++)
    {
		values[i] = rng_table[i];
    }
}

void generate_seed_forward_128(uint16 * values, uint16 * rng_seed, uint16 * rng_table)
{
	for (int i = 0; i < 0x10000; i++)
    {
		*rng_seed = i;
		for (int j = 0; j < 128; j++)
		{
			*rng_seed = rng_table[*rng_seed];
		}
		values[i] = *rng_seed;
    }
}


void run_alg(int alg_id, int iterations, uint8 * working_code, uint8 * regular_rng_values, uint8* alg0_values, uint8* alg2_values, uint8* alg5_values, uint8* alg6_values, uint16 * rng_seed, uint16 * rng_forward_1, uint16 * rng_forward_128)
{
	__m256i working_code0 = _mm256_load_si256((__m256i *)(working_code));
	__m256i working_code1 = _mm256_load_si256((__m256i *)(working_code + 32));
	__m256i working_code2 = _mm256_load_si256((__m256i *)(working_code + 64));
	__m256i working_code3 = _mm256_load_si256((__m256i *)(working_code + 96));
	__m256i working_code4 = _mm256_load_si256((__m256i *)(working_code + 128));
	__m256i working_code5 = _mm256_load_si256((__m256i *)(working_code + 160));
	__m256i working_code6 = _mm256_load_si256((__m256i *)(working_code + 192));
	__m256i working_code7 = _mm256_load_si256((__m256i *)(working_code + 224));

	__m256i mask_FF = _mm256_set1_epi16(0x00FF);
	__m256i mask_01 = _mm256_set1_epi16(0x0001);
	__m256i mask_top_01 = _mm256_set_epi16(0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	__m256i mask_alg2 = _mm256_set_epi16(0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0);
	__m256i mask_007F = _mm256_set_epi16(0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F);
	__m256i mask_FE00 = _mm256_set_epi16(0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0);
	__m256i mask_0080 = _mm256_set_epi16(0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80);

	__m256i mask_top_80 = _mm256_set_epi16(0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	__m256i mask_alg5 = _mm256_set_epi16(0,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0);
	__m256i mask_7F00 = _mm256_set_epi16(0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0);
	__m256i mask_00FE = _mm256_set_epi16(0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE);
	__m256i mask_0001 = _mm256_set_epi16(0,0x01,0,0x01,0,0x01,0,0x01,0,0x01,0,0x01,0,0x01,0,0x01);
	if (alg_id == 0)
	{
		for (int j = 0; j < iterations; j++)
		{
			uint8 * rng_start = alg0_values + ((*rng_seed) * 128 * 2);

			__m128i cur_val_lo = _mm256_castsi256_si128(working_code0);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			__m128i cur_val_hi = _mm256_extractf128_si256(working_code0,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code0 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			__m256i rng_val = _mm256_load_si256((__m256i *)(rng_start));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(rng_val)));
			working_code0 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code1);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code1,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code1 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 32));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(rng_val)));
			working_code1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code2);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code2,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code2 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 64));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(rng_val)));
			working_code2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code3);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code3,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code3 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 96));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(rng_val)));
			working_code3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code4);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code4,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code4 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 128));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(rng_val)));
			working_code4 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code5);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code5,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code5 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 160));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(rng_val)));
			working_code5 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code6);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code6,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code6 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 192));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(rng_val)));
			working_code6 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code7);
			cur_val_lo = _mm_slli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code7,1);
			cur_val_hi = _mm_slli_epi16(cur_val_hi,1);
			working_code7 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 224));
			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(rng_val)));
			working_code7 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(mask_FF)));

			*rng_seed = rng_forward_128[*rng_seed];
		}
	}
	else if (alg_id == 1)
	{
		for (int j = 0; j < iterations; j++)
		{
			uint8 * rng_start = regular_rng_values + ((*rng_seed) * 128 * 2);

			__m256i rng_val = _mm256_load_si256((__m256i *)(rng_start));
			__m128i sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code0), _mm256_castsi256_si128(rng_val));
			__m128i sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code0,1), _mm256_extractf128_si256(rng_val,1));
			working_code0 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code0 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 32));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code1), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code1,1), _mm256_extractf128_si256(rng_val,1));
			working_code1 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 64));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code2), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code2,1), _mm256_extractf128_si256(rng_val,1));
			working_code2 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 96));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code3), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code3,1), _mm256_extractf128_si256(rng_val,1));
			working_code3 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 128));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code4), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code4,1), _mm256_extractf128_si256(rng_val,1));
			working_code4 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code4 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 160));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code5), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code5,1), _mm256_extractf128_si256(rng_val,1));
			working_code5 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code5 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 192));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code6), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code6,1), _mm256_extractf128_si256(rng_val,1));
			working_code6 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code6 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 224));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code7), _mm256_castsi256_si128(rng_val));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code7,1), _mm256_extractf128_si256(rng_val,1));
			working_code7 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code7 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(mask_FF)));

			*rng_seed = rng_forward_128[*rng_seed];
		}
	}
	else if (alg_id == 2)
	{
		for (int j = 0; j < iterations; j++)
		{
			__m256i carry = _mm256_load_si256((__m256i *)(alg2_values + ((*rng_seed) * 32)));

			__m128i cur_val_lo = _mm256_castsi256_si128(working_code7);
			__m128i cur_val_hi = _mm256_extractf128_si256(working_code7,1);

			__m256i next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));

			__m128i temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			__m128i temp_hi = _mm_srli_si128(cur_val_hi,2);
			__m256i cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);

			__m256i temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			__m256i part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));

			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			__m256i part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));

			__m256i part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));

			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code7),_mm256_castsi256_pd(part3)));
			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code7),_mm256_castsi256_pd(carry)));

			carry = next_carry;



			cur_val_lo = _mm256_castsi256_si128(working_code6);
			cur_val_hi = _mm256_extractf128_si256(working_code6,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code6),_mm256_castsi256_pd(part3)));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code6),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code5);
			cur_val_hi = _mm256_extractf128_si256(working_code5,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code5),_mm256_castsi256_pd(part3)));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code5),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code4);
			cur_val_hi = _mm256_extractf128_si256(working_code4,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code4),_mm256_castsi256_pd(part3)));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code4),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code3);
			cur_val_hi = _mm256_extractf128_si256(working_code3,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code3),_mm256_castsi256_pd(part3)));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code3),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code2);
			cur_val_hi = _mm256_extractf128_si256(working_code2,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code2),_mm256_castsi256_pd(part3)));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code2),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code1);
			cur_val_hi = _mm256_extractf128_si256(working_code1,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_01)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code1),_mm256_castsi256_pd(part3)));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code1),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code0);
			cur_val_hi = _mm256_extractf128_si256(working_code0,1);
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg2)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_007F)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_FE00)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0080)));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code0),_mm256_castsi256_pd(part3)));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code0),_mm256_castsi256_pd(carry)));

			*rng_seed = rng_forward_1[*rng_seed];
		}
	}
	else if (alg_id == 3)
	{
		for (int j = 0; j < iterations; j++)
		{
			uint8 * rng_start = regular_rng_values + ((*rng_seed) * 128 * 2);

			__m256i rng_val = _mm256_load_si256((__m256i *)(rng_start));
			working_code0 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 32));
			working_code1 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 64));
			working_code2 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 96));
			working_code3 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 128));
			working_code4 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 160));
			working_code5 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 192));
			working_code6 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(rng_val)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 224));
			working_code7 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(rng_val)));

			*rng_seed = rng_forward_128[*rng_seed];
		}
	}
	else if (alg_id == 4)
	{
		for (int j = 0; j < iterations; j++)
		{
			uint8 * rng_start = regular_rng_values + ((*rng_seed) * 128 * 2);

			__m256i rng_val = _mm256_load_si256((__m256i *)(rng_start));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			__m128i sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code0), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			__m128i sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code0,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code0 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code0 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 32));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code1), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code1,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code1 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 64));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code2), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code2,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code2 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 96));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code3), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code3,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code3 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 128));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code4), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code4,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code4 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code4 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 160));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code5), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code5,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code5 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code5 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 192));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code6), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code6,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code6 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code6 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(mask_FF)));

			rng_val = _mm256_load_si256((__m256i *)(rng_start + 224));
			rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask_FF)));
			sum_lo = _mm_add_epi16(_mm256_castsi256_si128(working_code7), _mm256_castsi256_si128(rng_val));
			sum_lo = _mm_add_epi16(sum_lo, _mm256_castsi256_si128(mask_01));
			sum_hi = _mm_add_epi16(_mm256_extractf128_si256(working_code7,1), _mm256_extractf128_si256(rng_val,1));
			sum_hi = _mm_add_epi16(sum_hi, _mm256_castsi256_si128(mask_01));
			working_code7 = _mm256_set_m128i(sum_hi, sum_lo);
			working_code7 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(mask_FF)));

			*rng_seed = rng_forward_128[*rng_seed];
		}
	}
	else if (alg_id == 5)
	{
		for (int j = 0; j < iterations; j++)
		{
			__m256i carry = _mm256_load_si256((__m256i *)(alg5_values + ((*rng_seed) * 32)));

			__m128i cur_val_lo = _mm256_castsi256_si128(working_code7);
			__m128i cur_val_hi = _mm256_extractf128_si256(working_code7,1);

			__m256i next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));

			__m128i temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			__m128i temp_hi = _mm_srli_si128(cur_val_hi,2);
			__m256i cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);

			__m256i temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			__m256i part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));

			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			__m256i part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));

			__m256i part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));

			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code7),_mm256_castsi256_pd(part3)));
			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code7),_mm256_castsi256_pd(carry)));

			carry = next_carry;



			cur_val_lo = _mm256_castsi256_si128(working_code6);
			cur_val_hi = _mm256_extractf128_si256(working_code6,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code6),_mm256_castsi256_pd(part3)));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code6),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code5);
			cur_val_hi = _mm256_extractf128_si256(working_code5,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code5),_mm256_castsi256_pd(part3)));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code5),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code4);
			cur_val_hi = _mm256_extractf128_si256(working_code4,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code4),_mm256_castsi256_pd(part3)));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code4),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code3);
			cur_val_hi = _mm256_extractf128_si256(working_code3,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code3),_mm256_castsi256_pd(part3)));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code3),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code2);
			cur_val_hi = _mm256_extractf128_si256(working_code2,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code2),_mm256_castsi256_pd(part3)));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code2),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code1);
			cur_val_hi = _mm256_extractf128_si256(working_code1,1);
			next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(mask_top_80)));
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code1),_mm256_castsi256_pd(part3)));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code1),_mm256_castsi256_pd(carry)));
			carry = next_carry;

			cur_val_lo = _mm256_castsi256_si128(working_code0);
			cur_val_hi = _mm256_extractf128_si256(working_code0,1);
			temp_lo = _mm_srli_si128(cur_val_lo,2);
			temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
			temp_hi = _mm_srli_si128(cur_val_hi,2);
			cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
			temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_alg5)));
			carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));
			temp_lo = _mm_srli_epi16(cur_val_lo,1);
			temp_hi = _mm_srli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_7F00)));
			temp_lo = _mm_slli_epi16(cur_val_lo,1);
			temp_hi = _mm_slli_epi16(cur_val_hi,1);
			temp256 = _mm256_set_m128i(temp_hi, temp_lo);
			part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(mask_00FE)));
			part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(mask_0001)));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code0),_mm256_castsi256_pd(part3)));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code0),_mm256_castsi256_pd(carry)));

			*rng_seed = rng_forward_1[*rng_seed];
		}
	}
	else if (alg_id == 6)
	{
		for (int j = 0; j < iterations; j++)
		{
			uint8 * rng_start = alg6_values + ((*rng_seed) * 128 * 2);

			__m128i cur_val_lo = _mm256_castsi256_si128(working_code0);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			__m128i cur_val_hi = _mm256_extractf128_si256(working_code0,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code0 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			__m256i rng_val = _mm256_load_si256((__m256i *)(rng_start));
			working_code0 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(rng_val)));
			working_code0 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code1);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code1,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code1 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 32));
			working_code1 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(rng_val)));
			working_code1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code2);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code2,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code2 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 64));
			working_code2 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(rng_val)));
			working_code2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code3);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code3,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code3 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 96));
			working_code3 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(rng_val)));
			working_code3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code4);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code4,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code4 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 128));
			working_code4 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(rng_val)));
			working_code4 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code5);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code5,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code5 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 160));
			working_code5 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(rng_val)));
			working_code5 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code6);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code6,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code6 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 192));
			working_code6 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(rng_val)));
			working_code6 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(mask_FF)));

			cur_val_lo = _mm256_castsi256_si128(working_code7);
			cur_val_lo = _mm_srli_epi16(cur_val_lo,1);
			cur_val_hi = _mm256_extractf128_si256(working_code7,1);
			cur_val_hi = _mm_srli_epi16(cur_val_hi,1);
			working_code7 = _mm256_set_m128i(cur_val_hi, cur_val_lo);
			rng_val = _mm256_load_si256((__m256i *)(rng_start + 224));
			working_code7 = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(rng_val)));
			working_code7 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(mask_FF)));

			*rng_seed = rng_forward_128[*rng_seed];
		}
	}
	else if (alg_id == 7)
	{
		for (int j = 0; j < iterations; j++)
		{
			working_code0 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code0), _mm256_castsi256_pd(mask_FF)));
			working_code1 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code1), _mm256_castsi256_pd(mask_FF)));
			working_code2 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code2), _mm256_castsi256_pd(mask_FF)));
			working_code3 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code3), _mm256_castsi256_pd(mask_FF)));
			working_code4 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code4), _mm256_castsi256_pd(mask_FF)));
			working_code5 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code5), _mm256_castsi256_pd(mask_FF)));
			working_code6 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code6), _mm256_castsi256_pd(mask_FF)));
			working_code7 = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(working_code7), _mm256_castsi256_pd(mask_FF)));
		}
	}



	_mm256_store_si256 ((__m256i *)(working_code), working_code0);
	_mm256_store_si256 ((__m256i *)(working_code + 32), working_code1);
	_mm256_store_si256 ((__m256i *)(working_code + 64), working_code2);
	_mm256_store_si256 ((__m256i *)(working_code + 96), working_code3);
	_mm256_store_si256 ((__m256i *)(working_code + 128), working_code4);
	_mm256_store_si256 ((__m256i *)(working_code + 160), working_code5);
	_mm256_store_si256 ((__m256i *)(working_code + 192), working_code6);
	_mm256_store_si256 ((__m256i *)(working_code + 224), working_code7);
}

/*
void alg0(uint8 * working_code, uint8 * alg0_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	//return;
    for (int i = 0; i < 8; i++)
    {
		__m128i cur_val_lo = _mm_loadu_si128((__m128i *)(working_code + i*32));
		cur_val_lo = _mm_slli_epi16(cur_val_lo,1);

		__m128i cur_val_hi = _mm_loadu_si128((__m128i *)(working_code + i*32 + 16));
		cur_val_hi = _mm_slli_epi16(cur_val_hi,1);

		__m256i cur_val = _mm256_set_m128i(cur_val_hi, cur_val_lo);

		__m256i rng_val = _mm256_load_si256((__m256i *)(alg0_values + ((*rng_seed) * 128 * 2) + (i * 32)));

		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(rng_val)));

		__m256i mask = _mm256_set1_epi16(0x00FF);
		cur_val = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(mask)));
		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);
    }
	*rng_seed = rng_forward[*rng_seed];
}
*/

/*
void alg1(uint8 * working_code, uint8 * regular_rng_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	//return;
    for (int i = 0; i < 8; i++)
    {
		__m256i cur_val = _mm256_load_si256((__m256i *)(working_code + i*32));
		__m256i rng_val = _mm256_load_si256((__m256i *)(regular_rng_values + ((*rng_seed) * 128 * 2) + (i * 32)));

		__m128i sum_lo = _mm_add_epi16(_mm256_castsi256_si128(cur_val), _mm256_castsi256_si128(rng_val));

		cur_val = _mm256_castpd_si256(_mm256_permute2f128_pd(_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(cur_val), 1));
		rng_val = _mm256_castpd_si256(_mm256_permute2f128_pd(_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(rng_val), 1));

		__m128i sum_hi = _mm_add_epi16(_mm256_castsi256_si128(cur_val), _mm256_castsi256_si128(rng_val));

		cur_val = _mm256_set_m128i(sum_hi, sum_lo);

		__m256i mask = _mm256_set1_epi16(0x00FF);
		cur_val = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(mask)));

		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);
    }
	*rng_seed = rng_forward[*rng_seed];
}
*/

/*
void alg2(uint8 * working_code, uint8 * alg2_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	//return;
	// Load the 256 carry

	// Fetch current working value
	// Set the next carry by getting lowest of cur_val, shifting to top of 256
		// do 128bit shift to top of 128 lane, then 256 permute

	// Get extra carry:
		// do shift to cur val lo, store to temp
		// permute, do shift to hi, store to temp2
		// combine into 256
		// 256 mask it
		// or with carry

	// final combine
		// shift cur val lo, store to temp 1
		// shift cur val hi, store to temp 2
		// combine into temp large
		// mask it

		// shift cur val lo, store to temp 1
		// shift cur val hi, store to temp 2
		// combine into temp large
		// mask it

		// Shift across 128 lanes...
		// shift cur_val hi down 1 cell
		// shift cur_val hi up 7 cells, keep top 0xFF (probably don't need to mask it) (carry across lanes)
		// shift cur_val lo down 1 cell, or it with lane carry
		// combine into a thing
		// 256 mask it


		// 256-or the 4 parts together


	__m256i carry = _mm256_load_si256((__m256i *)(alg2_values + ((*rng_seed) * 32)));
    for (int i = 7; i >= 0; i--)
    {
		__m256i cur_val = _mm256_load_si256((__m256i *)(working_code + i*32));
		__m128i cur_val_lo = _mm256_castsi256_si128(cur_val);
		cur_val = _mm256_castpd_si256(_mm256_permute2f128_pd(_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(cur_val), 1));
		__m128i cur_val_hi = _mm256_castsi256_si128(cur_val);

		// Maybe 128-bit mask it, then reinterpret-cast it to 256 and permute it?
		__m256i next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(_mm256_set_epi16(0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0))));

		// Fetch extra carry and apply it to carry
		__m128i temp_lo = _mm_srli_si128(cur_val_lo,2);
		temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
		__m128i temp_hi = _mm_srli_si128(cur_val_hi,2);
		__m256i cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
		__m256i temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(_mm256_set_epi16(0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0))));
		carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));


		temp_lo = _mm_srli_epi16(cur_val_lo,1);
		temp_hi = _mm_srli_epi16(cur_val_hi,1);
		temp256 = _mm256_set_m128i(temp_hi, temp_lo);
		__m256i part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(_mm256_set_epi16(0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F))));

		temp_lo = _mm_slli_epi16(cur_val_lo,1);
		temp_hi = _mm_slli_epi16(cur_val_hi,1);
		temp256 = _mm256_set_m128i(temp_hi, temp_lo);
		__m256i part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(_mm256_set_epi16(0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0))));

		__m256i part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(_mm256_set_epi16(0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80))));

		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(cur_val),_mm256_castsi256_pd(part3)));
		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(cur_val),_mm256_castsi256_pd(carry)));

		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);

		carry = next_carry;
    }
	*rng_seed = rng_forward[*rng_seed];
}
*/

/*
void alg3(uint8 * working_code, uint8 * regular_rng_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	uint8 * rng_start = regular_rng_values + ((*rng_seed) * 128 * 2);
	//return;
    for (int i = 0; i < 8; i++)
    {
		__m256i cur_val = _mm256_load_si256((__m256i *)(working_code + i*32));
		__m256i rng_val = _mm256_load_si256((__m256i *)(rng_start + (i * 32)));
		cur_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(rng_val)));
		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);
    }
	*rng_seed = rng_forward[*rng_seed];
}
*/

/*
void alg4(uint8 * working_code, uint8 * regular_rng_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	//return;
    for (int i = 0; i < 8; i++)
    {
		__m256i cur_val = _mm256_load_si256((__m256i *)(working_code + i*32));
		__m256i rng_val = _mm256_load_si256((__m256i *)(regular_rng_values + ((*rng_seed) * 128 * 2) + (i * 32)));
		__m256i mask = _mm256_set1_epi16(0x00FF);
		__m128i one = _mm_set1_epi16(0x0001);
		rng_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(mask)));

		__m128i sum_lo = _mm_add_epi16(_mm256_castsi256_si128(cur_val), _mm256_castsi256_si128(rng_val));
		sum_lo = _mm_add_epi16(sum_lo, one);

		cur_val = _mm256_castpd_si256(_mm256_permute2f128_pd(_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(cur_val), 1));
		rng_val = _mm256_castpd_si256(_mm256_permute2f128_pd(_mm256_castsi256_pd(rng_val), _mm256_castsi256_pd(rng_val), 1));

		__m128i sum_hi = _mm_add_epi16(_mm256_castsi256_si128(cur_val), _mm256_castsi256_si128(rng_val));
		sum_hi = _mm_add_epi16(sum_hi, one);

		cur_val = _mm256_set_m128i(sum_hi, sum_lo);

		cur_val = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(mask)));

		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);
    }
	*rng_seed = rng_forward[*rng_seed];
}
*/

/*
void alg5(uint8* working_code, uint8 * alg5_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	__m256i carry = _mm256_load_si256((__m256i *)(alg5_values + ((*rng_seed) * 32)));
    for (int i = 7; i >= 0; i--)
    {
		__m256i cur_val = _mm256_load_si256((__m256i *)(working_code + i*32));
		__m128i cur_val_lo = _mm256_castsi256_si128(cur_val);
		cur_val = _mm256_castpd_si256(_mm256_permute2f128_pd(_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(cur_val), 1));
		__m128i cur_val_hi = _mm256_castsi256_si128(cur_val);

		// Maybe 128-bit mask it, then reinterpret-cast it to 256 and permute it?
		__m256i next_carry = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(_mm256_set_m128i(_mm_slli_si128(cur_val_lo,14), cur_val_lo)), _mm256_castsi256_pd(_mm256_set_epi16(0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0))));

		// Fetch extra carry and apply it to carry
		__m128i temp_lo = _mm_srli_si128(cur_val_lo,2);
		temp_lo = _mm_or_si128(temp_lo, _mm_slli_si128(cur_val_hi,14));
		__m128i temp_hi = _mm_srli_si128(cur_val_hi,2);
		__m256i cur_val_srl = _mm256_set_m128i(temp_hi, temp_lo);
		__m256i temp256 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(_mm256_set_epi16(0,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0,0x80,0))));
		carry = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(carry),_mm256_castsi256_pd(temp256)));


		temp_lo = _mm_srli_epi16(cur_val_lo,1);
		temp_hi = _mm_srli_epi16(cur_val_hi,1);
		temp256 = _mm256_set_m128i(temp_hi, temp_lo);
		__m256i part1 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(_mm256_set_epi16(0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0,0x7F,0))));

		temp_lo = _mm_slli_epi16(cur_val_lo,1);
		temp_hi = _mm_slli_epi16(cur_val_hi,1);
		temp256 = _mm256_set_m128i(temp_hi, temp_lo);
		__m256i part2 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(temp256), _mm256_castsi256_pd(_mm256_set_epi16(0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE,0,0xFE))));

		__m256i part3 = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val_srl), _mm256_castsi256_pd(_mm256_set_epi16(0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1))));

		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(part1),_mm256_castsi256_pd(part2)));
		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(cur_val),_mm256_castsi256_pd(part3)));
		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(cur_val),_mm256_castsi256_pd(carry)));

		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);

		carry = next_carry;
    }
	*rng_seed = rng_forward[*rng_seed];







	return;
}
*/

/*
void alg6(uint8 * working_code, uint8 * alg6_values, uint16 * rng_seed, uint16 * rng_table, uint16 * rng_forward)
{
	//return;
    for (int i = 0; i < 8; i++)
    {
		__m128i cur_val_lo = _mm_loadu_si128((__m128i *)(working_code + i*32));
		cur_val_lo = _mm_srli_epi16(cur_val_lo,1);

		__m128i cur_val_hi = _mm_loadu_si128((__m128i *)(working_code + i*32 + 16));
		cur_val_hi = _mm_srli_epi16(cur_val_hi,1);

		__m256i cur_val = _mm256_set_m128i(cur_val_hi, cur_val_lo);

		__m256i rng_val = _mm256_load_si256((__m256i *)(alg6_values + ((*rng_seed) * 128 * 2) + (i * 32)));

		cur_val = _mm256_castpd_si256(_mm256_or_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(rng_val)));

		__m256i mask = _mm256_set1_epi16(0x00FF);
		cur_val = _mm256_castpd_si256(_mm256_and_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(mask)));
		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);

    }
	*rng_seed = rng_forward[*rng_seed];
}
*/

/*
void alg7(uint8 * working_code)
{
	//return;
    for (int i = 0; i < 8; i++)
    {
		__m256i cur_val = _mm256_load_si256((__m256i *)(working_code + i*32));
		__m256i mask = _mm256_set1_epi16(0x00FF);
		cur_val = _mm256_castpd_si256(_mm256_xor_pd (_mm256_castsi256_pd(cur_val), _mm256_castsi256_pd(mask)));
		_mm256_store_si256 ((__m256i *)(working_code + i*32), cur_val);
    }
}
*/

void * aligned_malloc(int byte_count, int align_size)
{
	uint8 * new_array = new uint8[byte_count + align_size];
	int mod = ((uint64)new_array) % align_size;
	int remainder = align_size - mod;
	uint64 result = (uint64)new_array + (uint64)remainder;
	return (void*)(result);
}