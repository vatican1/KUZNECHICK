#pragma once
#include "consts.h"
#include <cassert>

void L(const uint8_t *in_data, uint8_t *out_data);
void L_reverse(const uint8_t *in_data, uint8_t *out_data);
void GenerateMulTable();

void test_linear();
