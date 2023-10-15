#include "../include/json_serializer.h"

#include <stddef.h>

char *append(char *buf, const char *suffix, size_t *remaining_size) {
  if (!buf)
    return NULL;

  for (; *suffix; ++suffix) {
    /* no more space */
    if (*remaining_size == 0)
      return NULL;

    *buf = *suffix;

    ++buf;
    --(*remaining_size);
  }

  if (*remaining_size == 0)
    return NULL;

  /* end with a null byte */
  *buf = '\0';

  return buf;
}

/**
 * @brief Append while removing last character if required (,)
 */
char *append_close(char *buf, const char *suffix, size_t *remaining_size) {
  if (!buf)
    return NULL;

  if (buf[-1] == ',') {
    ++(*remaining_size);
    --buf;
  }
  return append(buf, suffix, remaining_size);
}

/**
 * @brief Append element and add ',' after.
 */
char *append_element(char *buf, const char *value, size_t *remaining_size) {
  if (!buf)
    return NULL;

  buf = append(buf, value, remaining_size);
  buf = append(buf, ",", remaining_size);

  return buf;
}

char *conv(char *buf, long num, int base, size_t *remaining_size) {
  if (!buf)
    return NULL;

  if (num == 0) {
    if (base <= 10)
      return append(buf, "0", remaining_size);
    return append(buf, "00", remaining_size);
  }

  /* conv */

  if (num < 0) {
    buf = append(buf, "-", remaining_size);
    num = -num;
  }

  char *start = buf;

  while (num) {
    if (*remaining_size == 0)
      return NULL;

    int part = (num % base);
    char digit = 0;

    if (part < 10) {
      digit = '0' + part;
    } else {
      digit = 'A' + (part - 10);
    }

    *buf = digit;
    ++buf;
    --(*remaining_size);

    num /= base;
  }

  char *end = buf - 1;

  /* reverse */

  while (start < end) {
    char tmp = *start;
    *start = *end;
    *end = tmp;

    ++start;
    --end;
  }

  return buf;
}

char *ltoa(char *buf, long num, size_t *remaining_size) {
  return conv(buf, num, 10, remaining_size);
}

char *hex(char *buf, long num, size_t *remaining_size) {
  return conv(buf, num, 16, remaining_size);
}

char const unicode_length[] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 3, 4};

char *escape_unicode(char *buf, const char **str, size_t *remaining_size) {
  if ((**str & 0xC0) != 0xC0)
    return NULL;

  buf = append(buf, "\\u", remaining_size);

  int unicode_idx = ((unsigned char)(**str) & 0xFF) >> 4;
  int unicode_len = unicode_length[unicode_idx];

  unsigned int unicode = 0;
  const char *ustr = *str;

  for (int i = 0; i < unicode_len; ++i) {
    if (*ustr == '\0')
      return NULL;

    unicode = (unicode << 8) | (unsigned char)(*ustr);

    ++ustr;
  }

  *str = *str + unicode_len - 1;

  unsigned int mask = (unicode <= 0x00EFBFBF) ? 0x000F0000 : 0x003F0000;
  unsigned int codepoint =
      ((unicode & 0x07000000) >> 6) | ((unicode & mask) >> 4) |
      ((unicode & 0x00003F00) >> 2) | (unicode & 0x0000003F);

  switch (unicode_len) {
  case 1:
    buf = hex(buf, 0, remaining_size);
    buf = hex(buf, codepoint & 0xFF, remaining_size);
    return buf;
  case 2:
    buf = hex(buf, (codepoint >> 8) & 0xFF, remaining_size);
    buf = hex(buf, codepoint & 0xFF, remaining_size);
    return buf;
  case 3:
    /* not really sure if this is possible to have a second part here */
    if ((codepoint >> 16) & 0xFF) {
      buf = hex(buf, 0, remaining_size);
      buf = hex(buf, (codepoint >> 16) & 0xFF, remaining_size);
      buf = append(buf, "\\u", remaining_size);
    }
    buf = hex(buf, (codepoint >> 8) & 0xFF, remaining_size);
    buf = hex(buf, codepoint & 0xFF, remaining_size);
    return buf;
  case 4:;
    /* convert utf-32 into two utf-16 code */
    unsigned int high = 0;
    unsigned int low = 0;

    if (codepoint < 0x10000) {
      low = codepoint;
    } else {
      unsigned int tmp = codepoint - 0x10000;
      high = (((tmp << 12) >> 22) + 0xD800);
      low = (((tmp << 22) >> 22) + 0xDC00);
    }

    buf = hex(buf, high >> 8 & 0xFF, remaining_size);
    buf = hex(buf, high & 0xFF, remaining_size);
    buf = append(buf, "\\u", remaining_size);
    buf = hex(buf, low >> 8 & 0xFF, remaining_size);
    buf = hex(buf, low & 0xFF, remaining_size);
    return buf;
  default:
    return NULL;
  }
}

char *escape_str(char *buf, const char *str, size_t *remaining_size) {
  if (!buf)
    return NULL;

  for (; *str; ++str) {
    if (!buf || *remaining_size == 0)
      return NULL;

    switch (*str) {
    case '\"':
      buf = append(buf, "\\\"", remaining_size);
      break;
    case '\\':
      buf = append(buf, "\\\\", remaining_size);
      break;
    case '/':
      buf = append(buf, "\\/", remaining_size);
      break;
    case '\b':
      buf = append(buf, "\\b", remaining_size);
      break;
    case '\f':
      buf = append(buf, "\\f", remaining_size);
      break;
    case '\n':
      buf = append(buf, "\\n", remaining_size);
      break;
    case '\r':
      buf = append(buf, "\\r", remaining_size);
      break;
    case '\t':
      buf = append(buf, "\\t", remaining_size);
      break;
    default:
      // check for unicode character
      if ((*str & 0xC0) == 0xC0) {
        buf = escape_unicode(buf, &str, remaining_size);
        break;
      }

      *buf = *str;
      ++buf;
      --(*remaining_size);
    }
  }

  return buf;
}

char *string(char *buf, const char *key, size_t *remaining_size) {
  if (!buf)
    return NULL;

  buf = append(buf, "\"", remaining_size);
  buf = escape_str(buf, key, remaining_size);
  buf = append(buf, "\"", remaining_size);

  return buf;
}

char *key(char *buf, const char *key, size_t *remaining_size) {
  if (!buf)
    return NULL;

  buf = string(buf, key, remaining_size);
  buf = append(buf, ":", remaining_size);

  return buf;
}

char *json_obj_open(char *buf, const char *name, size_t *remaining_size) {
  if (!buf)
    return NULL;

  if (name)
    buf = key(buf, name, remaining_size);

  return append(buf, "{", remaining_size);
}

char *json_obj_close(char *buf, size_t *remaining_size) {
  if (!buf)
    return NULL;

  return append_close(buf, "},", remaining_size);
}

char *json_arr_open(char *buf, const char *name, size_t *remaining_size) {
  if (!buf)
    return NULL;

  if (name)
    buf = key(buf, name, remaining_size);

  return append(buf, "[", remaining_size);
}

char *json_arr_close(char *buf, size_t *remaining_size) {
  return append_close(buf, "],", remaining_size);
}

char *json_true(char *buf, size_t *remaining_size) {
  return append_element(buf, "true", remaining_size);
}

char *json_false(char *buf, size_t *remaining_size) {
  return append_element(buf, "false", remaining_size);
}

char *json_bool(char *buf, int boolean, size_t *remaining_size) {
  if (boolean) {
    return json_true(buf, remaining_size);
  }

  return json_false(buf, remaining_size);
}

char *json_null(char *buf, size_t *remaining_size) {
  return append_element(buf, "null", remaining_size);
}

char *json_str(char *buf, const char *str, size_t *remaining_size) {
  if (!buf)
    return NULL;

  buf = string(buf, str, remaining_size);
  buf = append(buf, ",", remaining_size);

  return buf;
}

char *json_number(char *buf, long number, size_t *remaining_size) {
  if (!buf)
    return NULL;

  buf = ltoa(buf, number, remaining_size);
  return append(buf, ",", remaining_size);
}

char *json_end(char *buf, size_t *remaining_size) {
  return append_close(buf, "", remaining_size);
}
