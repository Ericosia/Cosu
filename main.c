#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>

#include "env.h"

#define ERROR_FMT "--[ERROR]: %s:%s.%d: "
#define LOG_ERROR(err_msg, ...) fprintf(stderr, ERROR_FMT err_msg "\n", \
	__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define defer(_value) do { result = (_value); goto _defer; } while(0)


typedef struct {
    String_View *client_id;
    String_View *client_secret;
} Credentials, Creds;


typedef enum: uint8_t {
    BEARER = 0,
    ERROR
} TOKEN_TYPE;

typedef struct {
    TOKEN_TYPE type;
    size_t expires_in;
    String_View access_token;
    String_View refersh_token;
} Oauth_Res;

typedef struct {
    Credentials creds;
    Oauth_Res res;
    char **scores;
} Osu;


Credentials get_client_creds(char *file)
{
    
    Envs envs = { 0 };
    load_envs(&envs, file);

    return (Credentials) {
	get_value(&envs, "OSU_CLIENT_ID"),
	get_value(&envs, "OSU_CLIENT_SECRET"),
    };
}


size_t cb(char *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    Oauth_Res *res = (Oauth_Res *)userp;

    res->type = ERROR;

    printf("%s\n", buffer);
    return total_size;
}

CURL *init_curl(void)
{
    curl_global_init(CURL_GLOBAL_SSL);
    CURL *handle = curl_easy_init();
    if (!handle) {
	LOG_ERROR("Failed to initialize the curl handle!");
	return NULL;
    }
    return handle;
}

#define OSU_WEB_URL "https://osu.ppy.sh"
#define OSU_AUTH_ENDPOINT OSU_WEB_URL"/oauth/token"
#define OSU_AUTH_PARAMS_LENGTH(creds) sizeof("client_id=") + sizeof("&client_secret") + \
    sizeof("&grant_type=client_credentials&scope=public") + (creds).client_secret->count \
    + (creds).client_id->count

unsigned int get_access_token(CURL *handle, Credentials creds, Oauth_Res *res)
{
    char endpoint[OSU_AUTH_PARAMS_LENGTH(creds) + 1];
    snprintf(endpoint, OSU_AUTH_PARAMS_LENGTH(creds), "client_id="SV_FMT"&client_secret="SV_FMT"&grant_type=client_credentials&scope=public", SV_ARGS(*creds.client_id), SV_ARGS(*creds.client_secret));
    endpoint[OSU_AUTH_PARAMS_LENGTH(creds)] = '\0';

    curl_easy_setopt(handle, CURLOPT_URL, OSU_AUTH_ENDPOINT);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, endpoint);

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, res);

    CURLcode ret = curl_easy_perform(handle);

    if (ret != CURLE_OK) {
	LOG_ERROR("Failed to send a post request to "OSU_AUTH_ENDPOINT);
	return 1;
    }

    return 0;
}

#define SHIFT_ARGS(arr, count) (assert((count) > 0), (count)--, *(arr)++)
int main(int argc, char *argv[])
{
    char *prog = SHIFT_ARGS(argv, argc);
    if (argc == 0) {
	LOG_ERROR("Not enough arguments. Usage: %s <ENV_FILE>", prog);
	return 1;
    }

    
    Osu osu = { 0 };
    osu.creds = get_client_creds(argv[0]);
    printf("ID: "SV_FMT"\nSecret: "SV_FMT"\n", SV_ARGS(*osu.creds.client_id),
	    SV_ARGS(*osu.creds.client_secret));

    CURL *handle = init_curl();

    printf(OSU_AUTH_ENDPOINT"\n");
    if (get_access_token(handle, osu.creds, &osu.res) != 0) {
	LOG_ERROR("Failed to get access token");
	return 1;
    }

    printf("Token kind: %u\n", osu.res.type);
    return 0;
}

