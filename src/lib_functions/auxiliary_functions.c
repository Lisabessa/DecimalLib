#include "../decimal.h"

int get_sign(decimal src) { return (src.bits[3] & MINUS) == MINUS; }

void set_sign(decimal *dest, int sign) {
  dest->bits[3] =
      (sign == 0) ? ((dest->bits[3] << 1) >> 1) : (int)(dest->bits[3] | MINUS);
}

int get_sign_extended(extended_decimal src) {
  return (src.bits[6] & MINUS) == MINUS;
}

void set_sign_extended(extended_decimal *dest, int sign) {
  dest->bits[6] = (sign == 0) ? ((dest->bits[6] << 1) >> 1)
                              : (uint64_t)(dest->bits[6] | MINUS);
}

int get_scale(decimal src) { return (src.bits[3] >> 16) & 0xff; }

void set_scale(decimal *dst, int scale) {
  int signed_value = dst->bits[3] & MINUS;
  dst->bits[3] = signed_value | (scale << 16);
}

int get_scale_extended(extended_decimal src) {
  return (src.bits[6] >> 16) & 0xff;
}

void set_scale_extended(extended_decimal *dst, int scale) {
  int signed_value = dst->bits[6] & MINUS;
  dst->bits[6] = signed_value | (scale << 16);
}

int get_bit(decimal src, int bit) {
  return (src.bits[bit / 32] >> (bit % 32)) & 1;
}

int get_bit_extended(extended_decimal src, int bit) {
  return (src.bits[bit / 32] >> (bit % 32)) & 1;
}

void set_bit(decimal *dst, int bit, int value) {
  dst->bits[bit / 32] =
      (dst->bits[bit / 32] & ~(1 << (bit % 32))) | (value << (bit % 32));
}

void set_bit_extended(extended_decimal *dst, int bit, int value) {
  dst->bits[bit / 32] =
      ((dst->bits[bit / 32] & ~(1 << (bit % 32)))) | (value << (bit % 32));
}

void zeroing_int96(int32_t *value) {
  for (uint8_t i = 0; i < 3; i++) {
    value[i] = 0;
  }
}

void zeroing_decimal(decimal *value) {
  zeroing_int96(value->bits);
  value->bits[3] = 0;
}

void copy_decimal(decimal *dest, decimal src) {
  copy_int96(dest->bits, src.bits);
  dest->bits[3] = src.bits[3];
}

void copy_decimal_extended(extended_decimal *dest,
                           extended_decimal src) {
  for (uint8_t i = 0; i < 6; i++) {
    dest->bits[i] = src.bits[i];
  }
  dest->bits[6] = src.bits[6];
}

void copy_int96(int32_t *dest, const int32_t *src) {
  for (uint8_t i = 0; i < 3; i++) {
    dest[i] = src[i];
  }
}

int is_zero_decimal(decimal src) {
  int res_code = 0;
  if (src.bits[0] == 0 && src.bits[1] == 0 && src.bits[2] == 0) {
    res_code = 1;
  }
  return res_code;
}

void decimal_to_extended_decimal(decimal src, extended_decimal *dst) {
  for (int i = 0; i < 3; i++) {
    dst->bits[i] = (uint32_t)src.bits[i];
  }
  dst->bits[6] = (uint32_t)src.bits[3];
}

void extended_decimal_to_decimal(extended_decimal src, decimal *dst) {
  for (int i = 0; i < 3; i++) {
    dst->bits[i] = src.bits[i];
  }
  dst->bits[3] = src.bits[6];
}

int decimal_point_right(
    extended_decimal *src) {  // comma shift right - decreazing scale
  uint64_t compression_part = 0;
  for (int i = 5; i >= 0; i--) {
    src->bits[i] += compression_part << 32;
    compression_part = src->bits[i] % 10;
    src->bits[i] /= 10;
  }
  set_scale_extended(src, get_scale_extended(*src) - 1);
  return compression_part;
}

int decimal_point_left(
    extended_decimal *src) {  // comma shift left - increazing scale
  int res_code = 0;               // shift done
  extended_decimal temporary_src = {0};
  copy_decimal_extended(&temporary_src, *src);
  for (int i = 0; i < 6; i++) {
    temporary_src.bits[i] *= 10;
  }
  if (!get_overflow(&temporary_src)) {
    copy_decimal_extended(src, temporary_src);
    set_scale_extended(src, get_scale_extended(*src) + 1);
  } else {
    res_code = 1;  // shift is forbidden
  }
  return res_code;
}

int get_overflow(extended_decimal *src) {
  uint64_t overflow = 0;
  for (int j = 0; j < 6; j++) {
    src->bits[j] += overflow;
    overflow = src->bits[j] >> 32;
    src->bits[j] &= MAX4;
  }
  return overflow;
}

int normalize_extended_decimal(
    extended_decimal *src) {  // normalization of decimal = compression
                                  // extended decimal before converting
  int res_code = ARITHMETIC_OK;
  int global_compression_part = 0, compression_part = 0,
      temporary_compression_part = 0;
  int sign = get_sign_extended(*src);
  while ((src->bits[3] != 0 || get_scale_extended(*src) > 28) &&
         !(get_scale_extended(*src) > 100)) {
    global_compression_part += compression_part;
    compression_part = decimal_point_right(src);
  }

  extended_decimal temporary_decimal = {0};
  copy_decimal_extended(&temporary_decimal, *src);
  if (!(src->bits[2] == 0 && src->bits[1] == 0 && src->bits[0] == 0)) {
    int flag = 1;
    while (flag) {
      temporary_compression_part = decimal_point_right(&temporary_decimal);
      if (temporary_compression_part == 0 && get_scale_extended(*src) != 0) {
        compression_part = decimal_point_right(src);
      } else {
        flag = 0;
      }
    }
  }

  if (compression_part > 5 ||
      (compression_part == 5 && global_compression_part > 0) ||
      (compression_part == 5 && global_compression_part == 0 &&
       (src->bits[0] & 1))) {
    src->bits[0]++;
  }

  uint8_t scale = get_scale_extended(*src);
  if (scale > 100 && !sign) {
    res_code = ARITHMETIC_NUMBER_IS_TOO_LARGE_OR_EQUAL_TO_INFINITY;
  } else if ((scale == 28 && src->bits[2] == 0 && src->bits[1] == 0 &&
              src->bits[0] == 0) ||
             (scale > 100 && sign)) {
    res_code = ARITHMETIC_NUMBER_IS_TOO_SMALL_OR_EQUAL_TO_NEGATIVE_INFINITY;
  }

  return res_code;
}

void addition_extended_decimal(extended_decimal value1,
                               extended_decimal value2,
                               extended_decimal *result) {
  int reservation = 0;
  int res_bit = 0;
  for (int i = 0; i < 192; i++) {
    res_bit =
        get_bit_extended(value1, i) + get_bit_extended(value2, i) + reservation;
    if (res_bit > 1) {
      reservation = 1;
      res_bit -= 2;
    } else {
      reservation = 0;
    }
    set_bit_extended(result, i, res_bit);
  }
}

void subtraction_extended_decimal(extended_decimal value1,
                                  extended_decimal value2,
                                  extended_decimal *result) {
  int reservation = 0;
  int res_bit = 0;
  for (int i = 0; i < 192; i++) {
    res_bit =
        get_bit_extended(value1, i) - get_bit_extended(value2, i) - reservation;
    if (res_bit < 0) {
      reservation = 1;
      res_bit += 2;
    } else {
      reservation = 0;
    }
    set_bit_extended(result, i, res_bit);
  }
}

int equalize_scales(extended_decimal *value1,
                    extended_decimal *value2) {
  int scale1 = get_scale_extended(*value1);
  int scale2 = get_scale_extended(*value2);
  int res_code = 0;
  if (scale1 < scale2) {
    while (scale1 != scale2 && !res_code) {
      res_code = decimal_point_left(value1);
      scale1 = get_scale_extended(*value1);
    }
  } else if (scale2 < scale1) {
    while (scale1 != scale2 && !res_code) {
      res_code = decimal_point_left(value2);
      scale2 = get_scale_extended(*value2);
    }
  }
  return res_code;
}

int round_float_7(extended_decimal *dst) {
  int scale = get_scale_extended(*dst);
  if (scale > 7) {
    int compression_part = 0;
    for (int i = 7; i < scale; i++) {
      compression_part = decimal_point_right(dst);
    }
    if (compression_part >= 5) {
      dst->bits[0] += 1;
    }
    scale = 7;
  }
  return scale;
}

int compare_int192(extended_decimal value1,
                   extended_decimal value2) {  // 1 = < | -1 = >
  int res = 0;
  for (int i = 191; i >= 0 && res == 0; i--) {
    int bit_v1 = get_bit_extended(value1, i);
    int bit_v2 = get_bit_extended(value2, i);
    if (bit_v1 && !bit_v2) {
      res = -1;
    } else if (bit_v2 && !bit_v1) {
      res = 1;
    }
  }
  return res;
}
