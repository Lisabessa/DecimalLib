#include "../decimal.h"

int negate(decimal value, decimal *result) {
  copy_decimal(result, value);
  set_sign(result, !get_sign(value));
  return FUNCTION_OK;
}

int truncate(decimal value, decimal *result) {
  extended_decimal value_extended = {0};
  decimal_to_extended_decimal(value, &value_extended);
  int scale = get_scale_extended(value_extended);
  while (scale != 0) {
    decimal_point_right(&value_extended);
    scale = get_scale_extended(value_extended);
  }
  extended_decimal_to_decimal(value_extended, result);
  if (is_zero_decimal(*result)) {
    zeroing_decimal(result);
  }
  return FUNCTION_OK;
}

int floor_decimal(decimal value, decimal *result) {
  int scale = get_scale(value);
  int sign = get_sign(value);
  truncate(value, result);

  extended_decimal result_extended = {0};
  decimal_to_extended_decimal(*result, &result_extended);

  if (scale != 0 && sign) {
    result_extended.bits[0] += 1;
  }

  get_overflow(&result_extended);
  extended_decimal_to_decimal(result_extended, result);
  return FUNCTION_OK;
}

int round_decimal(decimal value, decimal *result) {
  extended_decimal value_extended = {0};
  decimal_to_extended_decimal(value, &value_extended);
  int scale = get_scale_extended(value_extended);
  int compression_part = 0;
  while (scale != 0) {
    compression_part = decimal_point_right(&value_extended);
    scale = get_scale_extended(value_extended);
  }
  if (compression_part >= 5) {
    value_extended.bits[0] += 1;
  }
  get_overflow(&value_extended);
  extended_decimal_to_decimal(value_extended, result);
  if (is_zero_decimal(*result)) {
    zeroing_decimal(result);  // zeroing scale and sign if mantissa is empty
  }
  return FUNCTION_OK;
}