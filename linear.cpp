#include <cstring>

#include "consts.h"
#include <cassert>
#include <string.h>

//__attribute__((always_inline))
//uint8_t GF_mul(uint8_t a, uint8_t b)
//{
//    uint8_t c = 0;
//    uint8_t hi_bit;
//    int i;
//    for (i = 0; i < 8; i++)
//    {
//        if (b & 1)
//            c ^= a;
//        hi_bit = a & 0x80;
//        a <<= 1;
//        if (hi_bit)
//            a ^= 0xc3;
//        b >>= 1;
//    }
//    return c;
//}

uint8_t mul_table[256][256];

__attribute__((always_inline))
uint8_t GF_mul(uint8_t a, uint8_t b)
{
    return mul_table[a][b];
}

uint8_t PolyMul (uint8_t left, uint8_t right)
{
    uint8_t res = 0;
    while (left && right)
    {
        if (right & 1)
            res ^= left;
        left = (left << 1) ^ (left & 0x80 ? 0xC3 : 0x00);
        right >>= 1;
    }
    return res;
}

void GenerateMulTable()
{
    for (unsigned i = 0; i < 256; ++i)
        for (unsigned j = 0; j < 256; ++j)
            mul_table[i][j] = PolyMul(i, j);
}

const unsigned char l_vec[16] = {
    1, 148, 32, 133, 16, 194, 192, 1,
    251, 1, 192, 194, 16, 133, 32, 148
};

__attribute__((always_inline))
void R(uint8_t *state) // (1)
{
    uint8_t a_15 = 0;
    vect internal;
    for (int i = 15; i >= 0; i--)
    {
        if(i != 0)
            internal[i - 1] = state[i];
//        a_15 ^= GF_mul(state[i], l_vec[i]);
        a_15 ^= mul_table[state[i]][l_vec[i]];
    }
    internal[15] = a_15;
    memcpy(state, internal, BLOCK_SIZE);
}

void L(const uint8_t *in_data, uint8_t *out_data)
{
    int i;
    vect internal;
    memcpy(internal, in_data, BLOCK_SIZE);
    for (i = 0; i < 16; i++) //(6)
        R(internal);
    memcpy(out_data, internal, BLOCK_SIZE);
}

//static uint8_t kuz_mul_gf256(uint8_t x, uint8_t y)
//{
//    uint8_t z;

//    z = 0;
//    while (y) {
//        if (y & 1)
//            z ^= x;
//        x = (x << 1) ^ (x & 0x80 ? 0xC3 : 0x00);
//        y >>= 1;
//    }

//    return z;
//}

//void L_(const uint8_t * in_data, uint8_t * out_data)
//{
//    int i, j;
//    uint8_t x;

//    memcpy(out_data, in_data, BLOCK_SIZE);
//    // 16 rounds
//    for (j = 0; j < 16; j++) {

//        // An LFSR with 16 elements from GF(2^8)
//        x = out_data[15];	// since lvec[15] = 1

//        for (i = 14; i >= 0; i--) {
//            out_data[i + 1] = out_data[i];
//            x ^= kuz_mul_gf256(out_data[i], l_vec[i]);
//        }
//        out_data[0] = x;
//    }
//}

void R_reverse(uint8_t *state)
{
    int i;
    uint8_t a_0;
    a_0 = state[15];
    vect internal;
    for (i = 0; i < 16; i++)
    {
        internal[i] = state[i - 1];// Двигаем все на старые места
//        a_0 ^= GF_mul(internal[i], l_vec[i]);
        a_0 ^= mul_table[internal[i]][l_vec[i]];
    }
    internal[0] = a_0;
    memcpy(state, internal, BLOCK_SIZE);
}

void L_reverse(const uint8_t *in_data, uint8_t *out_data)
{
    int i;
    vect internal;
    memcpy(internal, in_data, BLOCK_SIZE);
    for (i = 0; i < 16; i++)
        R_reverse(internal);
    memcpy(out_data, internal, BLOCK_SIZE);
}

void test_linear()
{
    {
        uint8_t test[16] =
        {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        R(test);
        uint8_t ans[16] =
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94};
        assert(memcmp(test, ans, 16 * sizeof (uint8_t)) == 0);
    }
    {
        uint8_t test[16] =
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94};
        R(test);
        uint8_t ans[16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0xa5};
        assert(memcmp(test, ans, 16 * sizeof (uint8_t)) == 0);
    }
    {
        uint8_t test[16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0xa5};
        R(test);
        uint8_t ans[16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0xa5, 0x64};
        assert(memcmp(test, ans, 16 * sizeof (uint8_t)) == 0);
    }
    {
        uint8_t test[16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0xa5, 0x64};
        R(test);
        uint8_t ans[16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x94, 0xa5, 0x64, 0x0d};
        assert(memcmp(test, ans, 16 * sizeof (uint8_t)) == 0);
    }
    {
        uint8_t test[16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0xa5, 0x64};
        uint8_t out[16];
        L(test, out);
        uint8_t ans[16] =
        {0x0d, 0x89, 0xa2, 0x7f, 0x4b, 0x6e, 0x16, 0xc3,
         0x4c, 0xe8, 0xe3, 0xd0, 0x4d, 0x58, 0x56, 0xd4};
        assert(memcmp(out, ans, 16 * sizeof (uint8_t)) == 0);
    }
}
