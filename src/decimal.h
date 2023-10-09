#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define MINUS 0x80000000
#define MAX4 0xffffffff

typedef struct {
  int bits[4];
} decimal;

typedef struct {
  uint64_t bits[7];
} extended_decimal;

enum arithmetic_result_codes {
  ARITHMETIC_OK,
  ARITHMETIC_NUMBER_IS_TOO_LARGE_OR_EQUAL_TO_INFINITY,
  ARITHMETIC_NUMBER_IS_TOO_SMALL_OR_EQUAL_TO_NEGATIVE_INFINITY,
  ARITHMETIC_DIVISION_BY_ZERO
};

enum converter_result_codes { CONVERTER_OK, CONVERTER_ERROR };

enum function_result_codes { FUNCTION_OK, FUNCTION_ERROR };

int negate(decimal value, decimal *result);
int truncate(decimal value, decimal *result);
int floor_decimal(decimal value, decimal *result);
int round_decimal(decimal value, decimal *result);

int mul(decimal value1, decimal value2, decimal *result);
int div(decimal value_1, decimal value_2, decimal *result);
int add(decimal value_1, decimal value_2, decimal *result);
int sub(decimal value_1, decimal value_2, decimal *result);

int from_int_to_decimal(int src, decimal *dst);
int from_decimal_to_int(decimal src, int *dst);
int from_decimal_to_float(decimal src, float *dst);
int from_float_to_decimal(float src, decimal *dst);

int is_greater_or_equal(decimal value1, decimal value2);
int is_equal(decimal value1, decimal value2);
int is_not_equal(decimal value1, decimal value2);
int is_less(decimal value1, decimal value2);
int is_less_or_equal(decimal value1, decimal value2);
int is_greater(decimal value1, decimal value2);

//------------------ AUXILIARY FUNCTIONS -------------------
int get_sign(decimal src);
void set_sign(decimal *dest, int sign);
int get_sign_extended(extended_decimal src);
void set_sign_extended(extended_decimal *dest, int sign);
int get_scale(decimal src);
void set_scale(decimal *dst, int scale);
int get_scale_extended(extended_decimal src);
void set_scale_extended(extended_decimal *dst, int scale);
int get_bit(decimal src, int bit);
int get_bit_extended(extended_decimal src, int bit);
void set_bit(decimal *dst, int bit, int value);
void set_bit_extended(extended_decimal *dst, int bit, int value);
void zeroing_int96(int32_t *value);
void zeroing_decimal(decimal *value);
void copy_decimal(decimal *dest, decimal src);
void copy_int96(int32_t *dest, const int32_t *src);
int is_zero_decimal(decimal src);
void decimal_to_extended_decimal(decimal src, extended_decimal *dst);
void extended_decimal_to_decimal(extended_decimal src, decimal *dst);
int decimal_point_right(extended_decimal *src);
int decimal_point_left(extended_decimal *src);
int get_overflow(extended_decimal *src);
int normalize_extended_decimal(extended_decimal *src);
void addition_extended_decimal(extended_decimal value1,
                               extended_decimal value2,
                               extended_decimal *result);
void subtraction_extended_decimal(extended_decimal value1,
                                  extended_decimal value2,
                                  extended_decimal *result);
int equalize_scales(extended_decimal *value1, extended_decimal *value2);
int compare_int192(extended_decimal value1, extended_decimal value2);
int round_float_7(extended_decimal *dst);