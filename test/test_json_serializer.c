#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "../include/json_serializer.h"

/* json_start_obj */

static void test_json_start_obj__unamed(void **state) {
  char json[256] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_obj_open(json, NULL, &rem_size);
  assert_string_equal("{", json);
  assert_ptr_equal(json + 1, buf);
  assert_int_equal(sizeof(json) - 1, rem_size);
}

static void test_json_start_obj__named(void **state) {
  char json[256] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);
  const char expected[] = "\"named\":{";

  buf = json_obj_open(json, "named", &rem_size);
  assert_string_equal(expected, json);
  assert_ptr_equal(json + sizeof(expected) - 1, buf);
}

static void test_json_start_obj__not_enough_space(void **state) {
  char json[1] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_obj_open(json, "named", &rem_size);
  assert_null(buf);
}

static void test_json_start_obj__propagate_null(void **state) {
  char json[256] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_obj_open(NULL, "named", &rem_size);
  assert_null(buf);
}

/* json_obj_close */

static void test_json_end_obj__close_empty_obj(void **state) {
  char json[256] = "{\0--------------------";
  char *buf = json + 1; // skip first {
  size_t rem_size = sizeof(json) - 1;

  buf = json_obj_close(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("{},", json);
}

static void test_json_end_obj__close_after_value(void **state) {
  char json[256] = "{\"test\":42,--------------------";
  char *buf = json + sizeof("{\"test\":42,") - 1; // skip the first part
  size_t rem_size = sizeof(json) - (buf - json);

  buf = json_obj_close(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("{\"test\":42},", json);
}

static void test_json_end_obj__not_enough_space(void **state) {
  char json[2] = "{\0";
  char *buf = json + 1; // skip first {
  size_t rem_size = 1;

  buf = json_obj_close(buf, &rem_size);
  assert_null(buf);
}

static void test_json_end_obj__propagate_error(void **state) {
  char json[2] = "{\0";
  char *buf = json + 1; // skip first {
  size_t rem_size = 1;

  buf = json_obj_close(NULL, &rem_size);
  assert_null(buf);
}

/* json_arr_open */

static void test_json_arr_open__unnamed(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_arr_open(json, NULL, &rem_size);
  assert_non_null(buf);
  assert_string_equal("[", json);
}

static void test_json_arr_open__named(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_arr_open(json, "array", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"array\":[", json);
}

static void test_json_arr_open__not_enough_space(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = 2;

  buf = json_arr_open(json, "array", &rem_size);
  assert_null(buf);
}

static void test_json_arr_open__propagate_error(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = 2;

  buf = json_arr_open(NULL, "array", &rem_size);
  assert_null(buf);
}

static void test_json_arr_close__close_empty(void **state) {
  char json[32] = "[\0----------";
  char *buf = json + 1; // skip first [
  size_t rem_size = sizeof(json) - 1;

  buf = json_arr_close(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("[],", json);
}

static void test_json_arr_close__close_after_value(void **state) {
  char json[32] = "[true,\0----------";
  char *buf = json + sizeof("[true,") - 1; // skip first part
  size_t rem_size = sizeof(json) - (buf - json);

  buf = json_arr_close(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("[true],", json);
}

static void test_json_arr_close__not_enough_space(void **state) {
  char json[32] = "[true,\0----------";
  char *buf = json + sizeof("[true,") - 1; // skip first part
  size_t rem_size = 1;

  buf = json_arr_close(buf, &rem_size);
  assert_null(buf);
}

static void test_json_arr_close__propagate_error(void **state) {
  char json[32] = "[true,\0----------";
  char *buf = json + sizeof("[true,") - 1; // skip first part
  size_t rem_size = 1;

  buf = json_arr_close(NULL, &rem_size);
  assert_null(buf);
}

/* json_true */

static void test_json_true__normal(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_true(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("true,", json);
}

static void test_json_true__not_enough_space(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = 1;

  buf = json_true(buf, &rem_size);
  assert_null(buf);
}

static void test_json_true__propagate_error(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_true(NULL, &rem_size);
  assert_null(buf);
}

/* json_false */

static void test_json_false__normal(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_false(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("false,", json);
}

static void test_json_false__not_enough_space(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = 1;

  buf = json_false(buf, &rem_size);
  assert_null(buf);
}

static void test_json_false__propagate_error(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_false(NULL, &rem_size);
  assert_null(buf);
}

/* json_bool */

static void test_json_bool__true(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_bool(buf, 42, &rem_size);
  assert_non_null(buf);
  assert_string_equal("true,", json);
}

static void test_json_bool__false(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_bool(buf, 0, &rem_size);
  assert_non_null(buf);
  assert_string_equal("false,", json);
}

static void test_json_bool__not_enough_space(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = 1;

  buf = json_bool(buf, 1, &rem_size);
  assert_null(buf);
}

static void test_json_bool__propagate_error(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_bool(NULL, 1, &rem_size);
  assert_null(buf);
}

/* json_null */

static void test_json_null__normal(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_null(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("null,", json);
}

static void test_json_null__not_enough_space(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = 1;

  buf = json_null(buf, &rem_size);
  assert_null(buf);
}

static void test_json_null__propagate_error(void **state) {
  char json[32] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_null(NULL, &rem_size);
  assert_null(buf);
}

/* json_str */

static void test_json_str__escape_double_quote(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with double quote (\") in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with double quote (\\\") in it\",", json);
}

static void test_json_str__escape_reverse_solidus(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with reverse solidus (\\) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with reverse solidus (\\\\) in it\",", json);
}

static void test_json_str__escape_solidus(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with solidus (/) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with solidus (\\/) in it\",", json);
}

static void test_json_str__escape_backspace(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with backspace (\b) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with backspace (\\b) in it\",", json);
}

static void test_json_str__escape_formfeed(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with formfeed (\f) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with formfeed (\\f) in it\",", json);
}

static void test_json_str__escape_linefeed(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with linefeed (\n) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with linefeed (\\n) in it\",", json);
}

static void test_json_str__escape_cr(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with cr (\r) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with cr (\\r) in it\",", json);
}

static void test_json_str__escape_tab(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with tab (\t) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with tab (\\t) in it\",", json);
}

static void test_json_str__escape_unicode_2(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with unicode (É) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with unicode (\\u00C9) in it\",", json);
}

static void test_json_str__escape_unicode_3(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with unicode (Ⴙ) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with unicode (\\u10B9) in it\",", json);
}

static void test_json_str__escape_unicode_4(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_str(buf, "string with unicode (👍) in it", &rem_size);
  assert_non_null(buf);
  assert_string_equal("\"string with unicode (\\uD83D\\uDC4D) in it\",", json);
}

/* json_number */

static void test_json_number__0(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, 0, &rem_size);
  assert_non_null(buf);
  assert_string_equal("0,", json);
}

static void test_json_number__minus_42(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, -42, &rem_size);
  assert_non_null(buf);
  assert_string_equal("-42,", json);
}

static void test_json_number__42(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, 42, &rem_size);
  assert_non_null(buf);
  assert_string_equal("42,", json);
}

static void test_json_number__maxint(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, INT32_MAX, &rem_size);
  assert_non_null(buf);
  assert_string_equal("2147483647,", json);
}

static void test_json_number__minint(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, INT32_MIN, &rem_size);
  assert_non_null(buf);
  assert_string_equal("-2147483648,", json);
}

static void test_json_number__maxlong(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, LONG_MAX, &rem_size);
  assert_non_null(buf);
  assert_string_equal("9223372036854775807,", json);
}

static void test_json_number__minlong_minus_one(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_number(buf, LONG_MIN + 1, &rem_size);
  assert_non_null(buf);
  assert_string_equal("-9223372036854775807,", json);
}

/* json_end */

static void test_json_end__normal(void **state) {
  char json[64] = "42,";
  char *buf = json + 3;
  size_t rem_size = sizeof(json) - 3;

  buf = json_end(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("42", json);
  assert_int_equal(sizeof(json) - 2, rem_size);
}

static void test_json_end__empty(void **state) {
  char json[64] = "--------------";
  char *buf = json;
  size_t rem_size = sizeof(json);

  // should null terminate and return an empty string
  buf = json_end(buf, &rem_size);
  assert_non_null(buf);
  assert_string_equal("", json);
  assert_int_equal(sizeof(json), rem_size);
}

static void test_json_end__propagate_error(void **state) {
  char json[64] = "--------------";
  char *buf = json;
  size_t rem_size = sizeof(json);

  // should null terminate and return an empty string
  buf = json_end(NULL, &rem_size);
  assert_null(buf);
}

/* integration */

static void test_json__empty_object(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_obj_open(buf, NULL, &rem_size);
  assert_non_null(buf);
  buf = json_obj_close(buf, &rem_size);
  assert_non_null(buf);
  buf = json_end(buf, &rem_size);
  assert_non_null(buf);

  assert_string_equal("{}", json);
}

static void test_json__empty_array(void **state) {
  char json[64] = {0};
  char *buf = json;
  size_t rem_size = sizeof(json);

  buf = json_arr_open(buf, NULL, &rem_size);
  assert_non_null(buf);
  buf = json_arr_close(buf, &rem_size);
  assert_non_null(buf);
  buf = json_end(buf, &rem_size);
  assert_non_null(buf);

  assert_string_equal("[]", json);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_json_start_obj__unamed),
      cmocka_unit_test(test_json_start_obj__named),
      cmocka_unit_test(test_json_start_obj__not_enough_space),
      cmocka_unit_test(test_json_start_obj__propagate_null),

      cmocka_unit_test(test_json_end_obj__close_empty_obj),
      cmocka_unit_test(test_json_end_obj__close_after_value),
      cmocka_unit_test(test_json_end_obj__not_enough_space),
      cmocka_unit_test(test_json_end_obj__propagate_error),

      cmocka_unit_test(test_json_arr_open__unnamed),
      cmocka_unit_test(test_json_arr_open__named),
      cmocka_unit_test(test_json_arr_open__not_enough_space),
      cmocka_unit_test(test_json_arr_open__propagate_error),

      cmocka_unit_test(test_json_arr_close__close_empty),
      cmocka_unit_test(test_json_arr_close__close_after_value),
      cmocka_unit_test(test_json_arr_close__not_enough_space),
      cmocka_unit_test(test_json_arr_close__propagate_error),

      cmocka_unit_test(test_json_true__normal),
      cmocka_unit_test(test_json_true__not_enough_space),
      cmocka_unit_test(test_json_true__propagate_error),

      cmocka_unit_test(test_json_false__normal),
      cmocka_unit_test(test_json_false__not_enough_space),
      cmocka_unit_test(test_json_false__propagate_error),

      cmocka_unit_test(test_json_bool__true),
      cmocka_unit_test(test_json_bool__false),
      cmocka_unit_test(test_json_bool__not_enough_space),
      cmocka_unit_test(test_json_bool__propagate_error),

      cmocka_unit_test(test_json_null__normal),
      cmocka_unit_test(test_json_null__not_enough_space),
      cmocka_unit_test(test_json_null__propagate_error),

      cmocka_unit_test(test_json_str__escape_double_quote),
      cmocka_unit_test(test_json_str__escape_reverse_solidus),
      cmocka_unit_test(test_json_str__escape_solidus),
      cmocka_unit_test(test_json_str__escape_backspace),
      cmocka_unit_test(test_json_str__escape_formfeed),
      cmocka_unit_test(test_json_str__escape_linefeed),
      cmocka_unit_test(test_json_str__escape_cr),
      cmocka_unit_test(test_json_str__escape_tab),

      cmocka_unit_test(test_json_str__escape_unicode_2),
      cmocka_unit_test(test_json_str__escape_unicode_3),
      cmocka_unit_test(test_json_str__escape_unicode_4),

      cmocka_unit_test(test_json_number__0),
      cmocka_unit_test(test_json_number__minus_42),
      cmocka_unit_test(test_json_number__42),
      cmocka_unit_test(test_json_number__maxint),
      cmocka_unit_test(test_json_number__minint),
      cmocka_unit_test(test_json_number__maxlong),
      cmocka_unit_test(test_json_number__minlong_minus_one),

      cmocka_unit_test(test_json_end__normal),
      cmocka_unit_test(test_json_end__empty),
      cmocka_unit_test(test_json_end__propagate_error),

      cmocka_unit_test(test_json__empty_object),
      cmocka_unit_test(test_json__empty_array),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
