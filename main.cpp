#include <cstring>

#include "consts.h"
#include "linear.h"
#include "non_linear.h"
#include <vector>
#include <iostream>

#include <ctime>

typedef unsigned __int128 uint128_t;


__attribute__((always_inline))
void X(const uint8_t *a, const uint8_t *b, uint8_t *c)
{
    int i;
    for (i = 0; i < BLOCK_SIZE; i++)
        c[i] = a[i] ^ b[i];
}


vect iter_C[32];

void calc_itec_C() // TODO сделать constexpr
{
    vect iter_num[32];
    for (int i = 0; i < 32; i++)
    {
        memset(iter_num[i], 0, BLOCK_SIZE);
        iter_num[i][0] = i + 1;
    }

    for (int  i = 0; i < 32; i++)
         L(iter_num[i], iter_C[i]); // (10)
}


void F(const uint8_t *in_key_1, const uint8_t *in_key_2,
       uint8_t *out_key_1, uint8_t *out_key_2,
       const uint8_t *iter_const) // (9)
{
    vect internal;
    memcpy(out_key_2, in_key_1, BLOCK_SIZE);
    X(in_key_1, iter_const, internal);
    S(internal, internal);
    L(internal, internal);
    X(internal, in_key_2, out_key_1);
}

vect iter_key[10]; // Итерационные ключи

void Expand_Key(const uint8_t *key_1, const uint8_t *key_2) // (11)
{
    int i;

    // Пара клчюей
    uint8_t iter_1[16];
    uint8_t iter_2[16];

    // следующая пара
    uint8_t iter_3[16];
    uint8_t iter_4[16];
    calc_itec_C();

    memcpy(iter_key[0], key_1, 16);
    memcpy(iter_key[1], key_2, 16);
    memcpy(iter_1, key_1, 16);
    memcpy(iter_2, key_2, 16);

    for (i = 0; i < 4; i++)
    {
        F(iter_1, iter_2, iter_3, iter_4, iter_C[0 + 8 * i]);
        F(iter_3, iter_4, iter_1, iter_2, iter_C[1 + 8 * i]);
        F(iter_1, iter_2, iter_3, iter_4, iter_C[2 + 8 * i]);
        F(iter_3, iter_4, iter_1, iter_2, iter_C[3 + 8 * i]);
        F(iter_1, iter_2, iter_3, iter_4, iter_C[4 + 8 * i]);
        F(iter_3, iter_4, iter_1, iter_2, iter_C[5 + 8 * i]);
        F(iter_1, iter_2, iter_3, iter_4, iter_C[6 + 8 * i]);
        F(iter_3, iter_4, iter_1, iter_2, iter_C[7 + 8 * i]);
        memcpy(iter_key[2 * i + 2], iter_1, 16);
        memcpy(iter_key[2 * i + 3], iter_2, 16);
    }
}

uint128_t LUT_enc[16][256];

void GenerateLUTTable()
{

    uint128_t x = 0;

    for (uint8_t i = 0; i < 16; ++i) {
        for (uint16_t j = 0; j < 256; ++j)
        {
            x = 0;
//            uint8_t * pointer_x = (uint8_t *)&x;
            ((uint8_t *)&x)[i] = apply_one_PI((uint8_t)j);
            L((uint8_t *)&x, (uint8_t *)&x);
            LUT_enc[i][j] = x;
        }
    }
}


void SL(const uint8_t *in_data, uint8_t *out_data)
{
    uint128_t x;
    memcpy(&x, in_data, BLOCK_SIZE * sizeof(uint8_t));
    x =     LUT_enc[ 0][((uint8_t *)&x)[ 0]] ^
            LUT_enc[ 1][((uint8_t *)&x)[ 1]] ^
            LUT_enc[ 2][((uint8_t *)&x)[ 2]] ^
            LUT_enc[ 3][((uint8_t *)&x)[ 3]] ^
            LUT_enc[ 4][((uint8_t *)&x)[ 4]] ^
            LUT_enc[ 5][((uint8_t *)&x)[ 5]] ^
            LUT_enc[ 6][((uint8_t *)&x)[ 6]] ^
            LUT_enc[ 7][((uint8_t *)&x)[ 7]] ^
            LUT_enc[ 8][((uint8_t *)&x)[ 8]] ^
            LUT_enc[ 9][((uint8_t *)&x)[ 9]] ^
            LUT_enc[10][((uint8_t *)&x)[10]] ^
            LUT_enc[11][((uint8_t *)&x)[11]] ^
            LUT_enc[12][((uint8_t *)&x)[12]] ^
            LUT_enc[13][((uint8_t *)&x)[13]] ^
            LUT_enc[14][((uint8_t *)&x)[14]] ^
            LUT_enc[15][((uint8_t *)&x)[15]];
    memcpy(out_data, &x, BLOCK_SIZE * sizeof(uint8_t));
}

void testLUTTable()
{
    uint8_t out_blk2[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t out_blk1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    S(out_blk1, out_blk1);
    L(out_blk1, out_blk1);
    SL(out_blk2, out_blk2);
    assert(memcmp(out_blk1, out_blk2, sizeof (uint8_t) * BLOCK_SIZE) == 0);
}


void Encript(const uint8_t *blk, uint8_t *out_blk)
{
    int i;
    memcpy(out_blk, blk, BLOCK_SIZE * sizeof(uint8_t));

    for(i = 0; i < 9; i++)
    {
        X(iter_key[i], out_blk, out_blk);
        SL(out_blk, out_blk);
//        S(out_blk, out_blk);
//        L(out_blk, out_blk);
    }
    X(out_blk, iter_key[9], out_blk);
}

void Decript(const uint8_t *blk, uint8_t *out_blk)
{
    int i;
    memcpy(out_blk, blk, BLOCK_SIZE);

    X(out_blk, iter_key[9], out_blk);
    for(i = 8; i >= 0; i--)
    {
        L_reverse(out_blk, out_blk);
        S_reverse(out_blk, out_blk);
        X(iter_key[i], out_blk, out_blk);
    }
}

void test()
{
    test_linear();
    test_non_linear();
    uint8_t a[16] = {
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
        0x00, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    uint8_t crypt[16];
    uint8_t decrypt[16];
    uint8_t iter_1[16] = {
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88
    };

    uint8_t iter_2[16] = {
        0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
        0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe
    };
    Expand_Key(iter_1, iter_2);
    Encript(a, crypt);
    Decript(crypt, decrypt);
    uint8_t true_crypt[16] = {0xcd, 0xed, 0xd4, 0xb9,
                              0x42, 0x8d, 0x46, 0x5a,
                              0x30, 0x24, 0xbc, 0xbe,
                              0x90, 0x9d, 0x67, 0x7f};
    assert(memcmp(crypt, true_crypt, 16 * sizeof (uint8_t)) == 0);
    assert(memcmp(a, decrypt, 16 * sizeof (uint8_t)) == 0);
}
void speed_test()
{
    uint8_t iter_1[16] = {
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88
    };

    uint8_t iter_2[16] = {
        0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
        0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe
    };
    Expand_Key(iter_1, iter_2);
    uint8_t a[16] = {
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
        0x00, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    uint8_t crypt[16];
    for(size_t i = 0; i < 1e8 / 128; ++i)
    {
        a[0] += 1;
        Encript(a, crypt);
    }
}



int main()
{
    GenerateMulTable();
    GenerateLUTTable();
//    testLUTTable();
//    test_non_linear();
//    test_linear();
//    test();

    unsigned int start_time =  clock();
    speed_test();
    unsigned int end_time = clock();
    unsigned int search_time = end_time - start_time;
    std::cout << search_time / 1e6<< "c" << std::endl;
    return 0;
}
