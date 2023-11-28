#pragma once
#include <stdint.h>

void S(const uint8_t *in_data, uint8_t *out_data);
void S_reverse(const uint8_t *in_data, uint8_t *out_data);

uint8_t apply_one_PI(uint8_t in);

void test_non_linear();
