#include "../decimal.h"

int is_less(decimal value1, decimal value2) {
  if (is_zero_decimal(value1)) {
    zeroing_decimal(&value1);  // zeroing scale and sign if mantissa is empty
  }
  if (is_zero_decimal(value2)) {
    zeroing_decimal(&value2);
  }
  int res_code = 0;
  int sign1 = get_sign(value1);
  int sign2 = get_sign(value2);
  if (sign1 == sign2) {
    extended_decimal value1_extended = {0};
    extended_decimal value2_extended = {0};
    decimal_to_extended_decimal(value1, &value1_extended);
    decimal_to_extended_decimal(value2, &value2_extended);
    equalize_scales(&value1_extended, &value2_extended);
    if ((compare_int192(value1_extended, value2_extended) == 1 && !sign1) ||
        (compare_int192(value1_extended, value2_extended) == -1 && sign1)) {
      res_code = 1;
    }
  }
  if (sign1 && !sign2) {
    res_code = 1;
  }

  return res_code;
}

int is_less_or_equal(decimal value1, decimal value2) {
  return is_less(value1, value2) || is_equal(value1, value2);
}

int is_greater(decimal value1, decimal value2) {
  return !is_less_or_equal(value1, value2);
}

int is_greater_or_equal(decimal value1, decimal value2) {
  return !is_less(value1, value2);
}

int is_equal(decimal value1, decimal value2) {
  int res_code = 0;
  if (is_zero_decimal(value1)) {
    zeroing_decimal(&value1);  // zeroing scale and sign if mantissa is empty
  }
  if (is_zero_decimal(value2)) {
    zeroing_decimal(&value2);
  }
  int sign1 = get_sign(value1);
  int sign2 = get_sign(value2);

  if (sign1 == sign2) {
    extended_decimal value1_extended = {0};
    extended_decimal value2_extended = {0};

    decimal_to_extended_decimal(value1, &value1_extended);
    decimal_to_extended_decimal(value2, &value2_extended);

    equalize_scales(&value1_extended, &value2_extended);

    if ((compare_int192(value1_extended, value2_extended) == 0)) {
      res_code = 1;
    }
  }

  return res_code;
}

int is_not_equal(decimal value1, decimal value2) {
  return !is_equal(value1, value2);
}