#ifndef JSON_SERIALIZER_H_
#define JSON_SERIALIZER_H_

#include <stddef.h>

/**
 * @brief JSON Serializer header.
 */

/**
 * @brief Close json. (remove the last ,)
 *
 * @param buf json write-out buffer.
 * @param remaining_size buf remaining size.
 *
 * @return pointer to the end of the new json-write out buffer.
 */
char *json_end(char *buf, size_t *remaining_size);

/**
 * @brief Open a json object.
 *
 * @param buf json write-out buffer.
 * @param name object key, NULL for unnamed object.
 * @param remaining_size buf remaining size.
 *
 * @return pointer to the end of the new json-write out buffer.
 */
char *json_obj_open(char *buf, const char *name, size_t *remaining_size);

/**
 * @brief Close a json object.
 *
 * @param buf json write-out buffer.
 * @param remaining_size buf remaining size.
 *
 * @return pointer to the end of the new json-write out buffer.
 */
char *json_obj_close(char *buf, size_t *remaining_size);

/**
 * @brief Open a json array.
 *
 * @param buf json write-out buffer.
 * @param name object key, NULL for unnamed object.
 * @param remaining_size buf remaining size.
 *
 * @return pointer to the end of the new json-write out buffer.
 */
char *json_arr_open(char *buf, const char *name, size_t *remaining_size);

/**
 * @brief Close a json array.
 *
 * @param buf json write-out buffer.
 * @param remaining_size buf remaining size.
 *
 * @return pointer to the end of the new json-write out buffer.
 */
char *json_arr_close(char *buf, size_t *remaining_size);

char *json_true(char *buf, size_t *remaining_size);
char *json_false(char *buf, size_t *remaining_size);
char *json_bool(char *buf, int boolean, size_t *remaining_size);
char *json_null(char *buf, size_t *remaining_size);

char *json_str(char *buf, const char *str, size_t *remaining_size);

char *json_number(char *buf, long number, size_t *remaining_size);

char *json_end(char *buf, size_t *remaining_size);

#endif /* ifndef JSON_SERIALIZER_H_ */
