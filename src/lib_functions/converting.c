#include "../decimal.h"

int from_int_to_decimal(int src, decimal *dst) {
  int res_code = CONVERTER_OK;
  if (INT_MIN <= src && src <= INT_MAX && dst) {
    zeroing_decimal(dst);
    if (src < 0) {
      negate(*dst, dst);
      src *= -1;
    }
    dst->bits[0] = src;
  } else {
    res_code = CONVERTER_ERROR;
  }
  return res_code;
}

int from_decimal_to_int(decimal src, int *dst) {
  int res_code = CONVERTER_OK;
  if (dst) {
    long double counter = 0;
    int scale = get_scale(src);
    int bit = 0;
    while (bit < 96 && res_code == 0) {
      counter += get_bit(src, bit) * pow(2, bit) / pow(10, scale);
      bit++;
      if (counter > INT32_MAX) {
        *dst = 0;
        res_code = CONVERTER_ERROR;
      }
    }
    if (res_code == 0) {
      *dst = (int)counter;
      if (get_sign(src)) {
        *dst *= -1;
      }
    }
  } else {
    res_code = CONVERTER_ERROR;
  }
  return res_code;
}

int from_decimal_to_float(decimal src, float *dst) {
  int res_code = CONVERTER_OK;
  if (dst) {
    long double counter = 0;
    int bit = 0;

    extended_decimal value_extended = {0};
    decimal_to_extended_decimal(src, &value_extended);

    int scale = get_scale_extended(value_extended);
    scale = round_float_7(&value_extended);
    extended_decimal_to_decimal(value_extended, &src);

    while (bit < 96) {
      counter += get_bit(src, bit) * pow(2, bit);
      bit++;
    }
    counter /= pow(10, scale);
    *dst = (float)counter;
    if (get_sign(src)) {
      *dst *= -1;
    }
  } else {
    res_code = CONVERTER_ERROR;
  }

  return res_code;
}

int from_float_to_decimal(float src, decimal *dst) {
  int res_code = CONVERTER_OK;
  if (!dst) {
    res_code = CONVERTER_ERROR;
  } else if (isnan(src) || isinf(src) || fabs(src) >= pow(2, 96)) {
    res_code = CONVERTER_ERROR;
    decimal infinity_decimal = {
        {0xffffffff, 0xffffffff, 0xffffffff, 0x7fffffff}};
    copy_decimal(dst, infinity_decimal);
    int mini_decimal = *((int *)&src);
    int s = mini_decimal >> 31;
    if (s) {
      set_sign(dst, 1);
    }
  } else if (fabs(src) < pow(10, -28) && fabs(src) > 0) {
    res_code = CONVERTER_ERROR;
    zeroing_decimal(dst);
  } else {
    zeroing_decimal(dst);
    if (src != 0) {
      int sign = 0;
      if (src < 0) {
        sign = 1;
        src *= -1;
      }
      long int mantissa = (long int)src;
      long int scale = 0;
      while (src - ((float)mantissa / (long int)(pow(10, scale))) != 0) {
        scale++;
        mantissa = src * (long int)(pow(10, scale));
      }

      from_int_to_decimal(mantissa, dst);
      if (sign) {
        set_sign(dst, 1);
      }
      set_scale(dst, scale);
      extended_decimal dst_extended = {0};
      decimal_to_extended_decimal(*dst, &dst_extended);
      round_float_7(&dst_extended);
      extended_decimal_to_decimal(dst_extended, dst);
    }
  }

  return res_code;
}