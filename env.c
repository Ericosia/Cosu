#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "env.h"


#define ERROR_FMT "--[ERROR]: %s:%s.%d: "
#define LOG_ERROR(err_msg, ...) fprintf(stderr, ERROR_FMT err_msg "\n",\
	__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define _defer(_val) do { result = (_val); goto defer; } while (0)

bool read_file(const char *file, String_Builder *sb)
{
    bool result = true;
    FILE *fptr = fopen(file, "r");
    if (!fptr) 
	_defer(NULL);
    
    fseek(fptr, 0, SEEK_END);
    size_t buf_size = ftell(fptr);
    rewind(fptr);

    size_t total_size = sb->count + buf_size;
    if (total_size > sb->capacity) {
	sb->data = realloc(sb->data, total_size);

	assert(sb->data && "Not enough ram to allocate sb->data");
	sb->capacity = total_size;
    }

    size_t bytes_read = fread(sb->data + sb->count, sizeof(*sb->data), buf_size, fptr);
    if (bytes_read != buf_size) {
	LOG_ERROR("Partial read of the file's content!");
	_defer(false);
    }

    sb->count = total_size;

    _defer(true);

defer:
    if (!result) LOG_ERROR("Failed to read file %s: %s", file, strerror(errno));
    if (fptr)
	fclose(fptr);

    return result;
}

String_Builder *new_sb()
{
    String_Builder *sb = malloc(sizeof(*sb));
    assert(sb && "Not enough ram to allocate a string_builder");
    sb->capacity = DEFAULT_SB_CAP;
    sb->count    = 0;
    sb->data     = calloc(sb->capacity, sizeof(*(sb->data)));
    assert(sb->data && "Failed to allocate string_builder->data");

    return sb;
}


String_View sv_chop_by_delim(String_View *sv, char delim)
{
    size_t i = 0;
    while(sv->data[i] != delim && i < sv->count) {
	i++;
    }

    String_View new_sv = { sv->data, i };

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }

    return new_sv;
}

String_View sv_from_parts(char *data, size_t count)
{
    return (SV) {
	.data = data,
	.count = count
    };
}

String_View sv_from_cstr(char *data)
{
    return (SV) {
	.data = data,
	.count = strlen(data)
    };
}

String_View sv_trim_left(String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
	i++;
    }

    return sv_from_parts(sv.data + i, sv.count - i);
}

String_View sv_trim_right(String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
	i++;
    }

    return sv_from_parts(sv.data, sv.count - i);
}

String_View sv_trim(String_View sv)
{
    return sv_trim_right(sv_trim_left(sv));
}


// TODO: Added support to skip comments
// bool is_comment(char c)
// {
//     if (c == '#')
// 	return true;
//
//     return false;
// }



String_View *get_value(Envs *envs, const char *key)
{
    for (size_t i = 0; i < envs->count; i++) {
	KV *pair = &envs->data[i];
	if (strlen(key) == pair->key.count && !strncmp(key, pair->key.data, pair->key.count)) {
	    return &pair->value;
	}
    }

    return NULL;
}


#define _0_TO_9_AS_STR  "0123456789"

size_t get_value_as_u64(Envs *envs, const char *key)
{
    String_View *value = get_value(envs, key);
    size_t number = 0, result = 0;

    for (size_t i = 0; i < value->count; i++) {
	char *pos = strchr(_0_TO_9_AS_STR, value->data[i]);
	if (pos - _0_TO_9_AS_STR < 0) {
	    LOG_ERROR("TS IS NOT A NUMBER GANG");
	    _defer(0xFFFFFFF);
	}

	size_t digit = pos - _0_TO_9_AS_STR;
	number = number * 10 + digit;
    }

    _defer(number);

defer:
    return result;
}

int load_envs(Envs *envs, const char *file)
{
    String_Builder sb = { 0 };
    if (!read_file(file, &sb)) {
	LOG_ERROR("Failed to read file");
	return 1;
    }

    for (size_t i = 0; sb.count; i++) {
	String_View line = sv_trim(sv_chop_by_delim((String_View *) &sb, '\n'));

	KV pair = (KV){ sv_chop_by_delim(&line, '='), line };
	da_append(envs, pair);
    }

    return 0;
}



