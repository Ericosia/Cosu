#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#define ERROR_FMT "--[ERROR]: %s:%s.%d: "
#define LOG_ERROR(err_msg, ...) fprintf(stderr, ERROR_FMT err_msg "\n", \
	__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define defer(_value) do { result = (_value); goto _defer; } while(0)

#define OSU_WEB_URL "https://osu.ppy.sh"

typedef struct {
    char *data;
    size_t length;
} String_View, SV;

typedef struct {
    String_View key;
    String_View value;
} Key_Value, KV;

typedef struct {
    size_t client_id;
    String_View client_secret;
} Credentials, Creds;

typedef struct {
    Credentials creds;
    char *token;
    char **scores;
} Osu;



// return file contents
char* read_file(const char *file)
{
    char *result;
    FILE *fptr = fopen(file, "r");
    if (!fptr) {
	LOG_ERROR("Failed to open %s!", file);
	defer(NULL);
    }
 
    fseek(fptr, 0, SEEK_END);
    size_t buf_size = ftell(fptr);
    rewind(fptr);

    char *buf = calloc(buf_size, sizeof(*buf));
    assert(buf && "Failed to allocate buffer for file's content");

    size_t bytes_read = fread(buf, sizeof(*buf), buf_size, fptr);
    if (bytes_read != buf_size) {
	LOG_ERROR("Partial read of the file's content!");
	defer(buf);
    }

    defer(buf);

_defer:
    if (fptr)
	fclose(fptr);

    return result;
}


int env_read_u64(char *content, size_t *userp)
{

}


#define SHIFT_ARGS(arr, count) (assert((count) > 0), (count)--, *(arr)++)
int main(int argc, char *argv[])
{
    char *prog = SHIFT_ARGS(argv, argc);
    if (argc == 0) {
	LOG_ERROR("Not enough arguments. Usage: %s <ENV_FILE>", prog);
	return 1;
    }

    char *file_content = read_file(argv[0]);
    printf("File content:\n%s", file_content ? file_content : "NULL");

    Osu osu = { 0 };
    env_read_u64(file_content, &osu.creds.client_id);

    return 0;
}


#if 0

size_t cb(char *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    Osu *osu = (Osu *)userp;

    return total_size;
}

int init_curl(void)
{
    curl_global_init(CURL_GLOBAL_SSL);
    CURL *handle = curl_easy_init();
    if (!handle) {
	LOG_ERROR("Failed to initialize the curl handle!");
	return 1;
    }

    Osu osu = { "Hello, from cb", NULL };

    curl_easy_setopt(handle, CURLOPT_URL, OSU_WEB_URL);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &osu);

    // CURLcode ret = curl_easy_perform(handle);
    // printf("ret: %d\n", ret);

    return 0;
}

#endif
