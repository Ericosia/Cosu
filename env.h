#ifndef ENV_H_
#define ENV_H_


#define DEFAULT_DA_CAP 256
// TODO: turn * to << op.
#define da_resize(da, total_size)\
    do {\
	if ((total_size) > (da)->capacity) {\
	    if ((da)->capacity == 0) { \
		(da)->capacity = DEFAULT_DA_CAP; \
	    } \
	    while ((total_size) > (da)->capacity) { \
		(da)->capacity *= 2; \
	    } \
	    (da)->data = realloc((da)->data, (da)->capacity * sizeof(*(da)->data));\
	    assert((da)->data && "Not enough ram to resize the dynamic array");\
	}\
    } while (0)


#define da_append(da, item) \
    do { \
	da_resize(da, (da)->count + 1);\
	(da)->data[(da)->count++] = (item);\
    } while (0)

#define da_append_many(da, new_items, new_items_count)                                      \
    do {                                                                                        \
        da_resize((da), (da)->count + (new_items_count));                                  \
        memcpy((da)->data + (da)->count, (new_items), (new_items_count) * sizeof(*(da)->data)); \
        (da)->count += (new_items_count);                                                       \
    } while (0)

#define DEFAULT_SB_CAP 8
#define DEFAULT_STR_CAP 1024
typedef struct {
    char *data;
    unsigned long count;
    unsigned long capacity;
} String_Builder, SB;


#define SV_FMT "%.*s"
#define SV_ARGS(sv) (int) (sv).count, (sv).data
typedef struct {
    char *data;
    unsigned long count;
} String_View, SV;


#define KV_FMT(delim) SV_FMT delim SV_FMT
#define KV_ARGS(kv) SV_ARGS((kv)->key), SV_ARGS((kv)->value)
#define LOG_KV(kv, delim) printf(KV_FMT(delim)"\n", KV_ARGS((kv)))
typedef struct {
   String_View  key;
   String_View  value;
} Key_Value, KV;


// dynamic-array that holds the .env KV pairs
typedef struct {
    KV *data;
    unsigned long count;
    unsigned long capacity;
} Envs;

// Main functions of the API.
int load_envs(Envs *envs, const char *file);
String_View *get_value(Envs *envs, const char *key);
unsigned long get_value_as_u64(Envs *envs, const char *key);

// Auxilary funcs
bool read_entire_file(const char *file, String_Builder *sb);
String_View sv_from_parts(char *data, unsigned long count);
String_View sv_from_cstr(char *data);
#endif // ENV_H_

