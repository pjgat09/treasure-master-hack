#include "data_sizes.h"
#include "tm_64_16.h"

void alg0(uint64 * working_code, const uint64 * alg0_values, const uint16 rng_seed)
{
    for (int i = 0; i < 32; i++)
    {
        working_code[i] = ((working_code[i] << 1) | alg0_values[(rng_seed * 128) / 4 + i]) & 0x00FF00FF00FF00FFull;
    }
}

void alg1(uint64 * working_code, const uint64 * regular_rng_values, const uint16 rng_seed)
{
    for (int i = 0; i < 32; i++)
    {
        working_code[i] = (working_code[i] + regular_rng_values[(rng_seed * 128) / 4 + i]) & 0x00FF00FF00FF00FFull;
    }
}

void alg2(uint64 * working_code, const uint64 * alg2_values, const uint16 rng_seed)
{
    uint64 carry = alg2_values[rng_seed];
    for (int i = 31; i >= 0; i--)
    {
        uint64 next_carry = (working_code[i] & 0x0000000000000001ull) << 48;
        
		carry = carry | ((working_code[i] & 0x0000000100000000ull) >> 16);
        working_code[i] = ((working_code[i] >> 1) & 0x0000007F0000007Full) | ((working_code[i] << 1) & 0x00FE000000FE0000ull) | ((working_code[i] >> 16) & 0x0000008000000080ull) | carry;
        
        carry = next_carry;
    }
}

void alg3(uint64 * working_code, const uint64 * regular_rng_values, const uint16 rng_seed)
{
    for (int i = 0; i < 32; i++)
    {
        working_code[i] = working_code[i] ^ regular_rng_values[(rng_seed * 128) / 4 + i];
    }
}

void alg5(uint64* working_code, const uint64 * alg5_values, const uint16 rng_seed)
{
    uint64 carry = alg5_values[rng_seed];
    for (int i = 31; i >= 0; i--)
    {
        uint64 next_carry = (working_code[i] & 0x0000000000000080ull) << 48;
        
		carry = carry | ((working_code[i] & 0x0000008000000000ull) >> 16);
        working_code[i] = ((working_code[i] << 1) & 0x000000FE000000FEull) | ((working_code[i] >> 1) & 0x007F0000007F0000ull) | ((working_code[i] >> 16) & 0x0000000100000001ull) | carry;
        
        carry = next_carry;
    }
}


void alg6(uint64 * working_code, const uint64 * alg6_values, const uint16 rng_seed)
{
    for (int i = 0; i < 32; i++)
    {
        working_code[i] = ((working_code[i] >> 1) | alg6_values[(rng_seed * 128) / 4 + i]) & 0x00FF00FF00FF00FFull;
    }
}


void alg7(uint64 * working_code)
{
    for (int i = 0; i < 32; i++)
    {
        working_code[i] = working_code[i] ^ 0x00FF00FF00FF00FFull;
    }
}