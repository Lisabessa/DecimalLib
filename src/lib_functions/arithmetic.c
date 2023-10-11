#include "../decimal.h"

int mul(decimal value_1, decimal value_2, decimal *result) {
  int res_code = ARITHMETIC_OK;
  zeroing_decimal(result);
  if (!(is_zero_decimal(value_1) || is_zero_decimal(value_2))) {
    extended_decimal value1_extended = {0};
    decimal_to_extended_decimal(value_1, &value1_extended);
    extended_decimal result_extended = {0};
    for (int i = 0; i < 96; i++) {
      if (get_bit(value_2, i)) {
        addition_extended_decimal(result_extended, value1_extended,
                                  &result_extended);
      }
      for (int j = 0; j < 6; j++) {
        value1_extended.bits[j] *= 2;
      }
      get_overflow(&value1_extended);
    }
    set_scale_extended(&result_extended,
                       get_scale(value_1) + get_scale(value_2));
    set_sign_extended(&result_extended, get_sign(value_1) ^ get_sign(value_2));
    if (normalize_extended_decimal(&result_extended) == 0) {
      extended_decimal_to_decimal(result_extended, result);
      if (is_zero_decimal(*result)) {
        zeroing_decimal(result);  // zeroing scale and sign if mantissa is empty
      }
    } else {
      res_code = ARITHMETIC_NUMBER_IS_TOO_LARGE_OR_EQUAL_TO_INFINITY;
    }
  }
  return res_code;
}

int div(decimal value_1, decimal value_2, decimal *result) {
  int res_code = ARITHMETIC_OK;
  zeroing_decimal(result);
  if (is_zero_decimal(value_2)) {
    res_code = ARITHMETIC_DIVISION_BY_ZERO;
  } else if (!is_zero_decimal(value_1)) {
    int sign1 = get_sign(value_1);
    int sign2 = get_sign(value_2);
    set_sign(&value_1, 0);
    set_sign(&value_2, 0);
    int scale2 = get_scale(value_2);
    set_scale(&value_2, 0);

    extended_decimal value1_extended = {0};
    decimal_to_extended_decimal(value_1, &value1_extended);
    extended_decimal result_extended = {0};

    while (value1_extended.bits[5] <= MAX4 / 10) {
      res_code = decimal_point_left(&value1_extended);
    }

    int length_number = 191;
    while (!get_bit_extended(value1_extended, length_number)) {
      length_number -= 1;
    }

    int length_denominator = 95;
    while (!get_bit(value_2, length_denominator)) {
      length_denominator -= 1;
    }

    decimal buffer = {0};
    for (int i = 0; i <= length_denominator; i++) {
      set_bit(&buffer, i,
              get_bit_extended(value1_extended,
                               length_number - length_denominator + i));
    }

    for (int i = 0; i <= length_number - length_denominator; i++) {
      for (int i = 0; i < 6; i++) {
        result_extended.bits[i] *= 2;
      }
      get_overflow(&result_extended);
      if (is_greater_or_equal(buffer, value_2)) {
        result_extended.bits[0] += 1;
        sub(buffer, value_2, &buffer);
      }
      if (i != length_number - length_denominator) {
        if ((buffer.bits[2] >> 31) & 1) {
          res_code = ARITHMETIC_NUMBER_IS_TOO_LARGE_OR_EQUAL_TO_INFINITY;
        }
        uint8_t bit_cur = 0, bit_next;
        for (int8_t i = 0; i < 3; i++) {
          bit_next = (buffer.bits[i] >> 31) & 1;
          buffer.bits[i] <<= 1;
          buffer.bits[i] |= bit_cur;
          bit_cur = bit_next;
        }
        set_bit(&buffer, 0,
                get_bit_extended(value1_extended,
                                 length_number - length_denominator - i - 1));
      }
    }
    set_scale_extended(&result_extended,
                       get_scale_extended(value1_extended) - scale2);
    set_sign_extended(&result_extended, sign1 ^ sign2);
    if (get_overflow(&result_extended) == 0 &&
        normalize_extended_decimal(&result_extended) == 0) {
      extended_decimal_to_decimal(result_extended, result);
      if (is_zero_decimal(*result)) {
        zeroing_decimal(result);  // zeroing scale and sign if mantissa is empty
      }
    } else {
      res_code = ARITHMETIC_NUMBER_IS_TOO_LARGE_OR_EQUAL_TO_INFINITY;
    }
  }
  return res_code;
}

int add(decimal value_1, decimal value_2, decimal *result) {
  int res_code = ARITHMETIC_OK;
  zeroing_decimal(result);
  int sign1 = get_sign(value_1);
  int sign2 = get_sign(value_2);
  extended_decimal value_1_extended = {0};
  extended_decimal value_2_extended = {0};
  extended_decimal result_extended = {0};
  decimal_to_extended_decimal(value_1, &value_1_extended);
  decimal_to_extended_decimal(value_2, &value_2_extended);
  res_code = equalize_scales(&value_1_extended, &value_2_extended);
  if (!res_code && sign1 == sign2) {
    addition_extended_decimal(value_1_extended, value_2_extended,
                              &result_extended);
    set_sign_extended(&result_extended, sign1);
  } else if (!res_code && sign1) {
    set_sign(&value_1, 0);
    if (is_less(value_1, value_2)) {
      subtraction_extended_decimal(value_2_extended, value_1_extended,
                                   &result_extended);
      set_sign_extended(&result_extended, sign2);
    } else {
      subtraction_extended_decimal(value_1_extended, value_2_extended,
                                   &result_extended);
      set_sign_extended(&result_extended, sign1);
    }
  } else if (!res_code && sign2) {
    set_sign(&value_2, 0);
    if (is_less(value_2, value_1)) {
      subtraction_extended_decimal(value_1_extended, value_2_extended,
                                   &result_extended);
      set_sign_extended(&result_extended, sign1);
    } else {
      subtraction_extended_decimal(value_2_extended, value_1_extended,
                                   &result_extended);
      set_sign_extended(&result_extended, sign2);
    }
  }
  if (!res_code) {
    set_scale_extended(&result_extended, get_scale_extended(value_1_extended));
    if (normalize_extended_decimal(&result_extended) == 0) {
      extended_decimal_to_decimal(result_extended, result);
      if (is_zero_decimal(*result)) {
        zeroing_decimal(result);  // zeroing scale and sign if mantissa is empty
      }
    } else {
      res_code = ARITHMETIC_NUMBER_IS_TOO_LARGE_OR_EQUAL_TO_INFINITY;
    }
  }
  return res_code;
}

int sub(decimal value_1, decimal value_2, decimal *result) {
  set_sign(&value_2, !get_sign(value_2));
  int res_code = add(value_1, value_2, result);
  return res_code;
}