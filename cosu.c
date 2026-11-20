#include <assert.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "env.h"
#include "jsmn.h"
#include "cosu.h"

#define ERROR_FMT "--[ERROR]: %s:%s.%d: "
#define LOG_ERROR(err_msg, ...) fprintf(stderr, ERROR_FMT err_msg "\n", \
	__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define defer(_value) do { result = (_value); goto _defer; } while(0)

#define ARRAY_LENGTH(arr) sizeof((arr)) / sizeof((arr)[0])


#define curl_post_req(handle, endpoint, params, cb, var) \
{\
    curl_easy_setopt((handle), CURLOPT_URL, (endpoint)); \
    curl_easy_setopt((handle), CURLOPT_POSTFIELDS, (params)); \
    curl_easy_setopt((handle), CURLOPT_WRITEFUNCTION, (cb)); \
    curl_easy_setopt((handle), CURLOPT_WRITEDATA, (var)); \
}

const char *ruleset_to_cstr(RULESET ruleset)
{
    switch(ruleset) {
	case OSU:       return "osu";
	case TAIKO:     return "taiko";
	case FRUITS:    return "fruits";
	case MANIA:     return "mania";
	default:        return NULL;
    }
}

int get_client_creds(Credentials *creds, const char *file)
{
    
    Envs envs = { 0 };
    if (load_envs(&envs, file)) {
	LOG_ERROR("Failed to load variables from %s file", file);
	return 1;
    }

    *creds = (Credentials) {
	get_value(&envs, "OSU_CLIENT_ID"),
	get_value(&envs, "OSU_CLIENT_SECRET"),
    };

    return 0;
}


size_t sv_to_u64(String_View sv)
{
    size_t val = 0;
    for (size_t i = 0; i < sv.count; i++) {
	char *pos = strchr("0123456789", sv.data[i]);
	if (pos - "0123456789" < 0) {
	    LOG_ERROR("Expected a number, found a non numerical value");
	    return 0xFFFFFFFF;
	}

	size_t digit = pos - "0123456789";
	val = val * 10 + digit;
    }

    return val;
}


// TODO: Rewrite to SV compatible function
int jsoneq(const char *json, jsmntok_t *token, const char *key)
{
    if (token->type == JSMN_STRING && (int)strlen(key) == token->end - token->start &&
      strncmp(json + token->start, key, token->end - token->start) == 0) {

	return 0;
    }
    return 1;
}

void fill_oauth(Oauth_Res *oauth, String_View json_response, jsmntok_t *tokens, size_t tokens_count)
{
    for (size_t i = 1; i < tokens_count; i++) {
	KV json_pair = {
	    sv_from_parts(json_response.data + tokens[i].start, tokens[i].end - tokens[i].start),
	    sv_from_parts(json_response.data + tokens[i + 1].start, 
		    tokens[i + 1].end - tokens[i + 1].start),
	};

	if (!jsoneq(json_response.data, tokens + i, "token_type")) {
	    if (!strncmp(json_pair.key.data, "Bearer", json_pair.key.count)) {
		oauth->type = BEARER;
	    } else {
		oauth->type = ERROR;
	    }
	    i++;
	} else if (!jsoneq(json_response.data, tokens + i, "expires_in")) {
	    oauth->expires_in = sv_to_u64(json_pair.value);
	    i++;
	} else if (!jsoneq(json_response.data, tokens + i, "access_token")) {
	    oauth->access_token = json_pair.value;
	    i++;
	} else if (!jsoneq(json_response.data, tokens + i, "refresh_token")) {
	    oauth->refresh_token = json_pair.value;
	    i++;
	} else {
	    LOG_ERROR("Unexpected json field found for oauth: ");
	    LOG_KV(&json_pair, " : ");
	}
    }
}

#define JSMN_MAX_TOKENS 128
int parse_json_response(Oauth_Res *oauth, String_View json_res)
{
    jsmn_parser parser = { 0 };
    jsmntok_t tokens[JSMN_MAX_TOKENS] = { 0 };

    jsmn_init(&parser);

    int jsmn_parsed_count = jsmn_parse(&parser, json_res.data, json_res.count, tokens,
	    ARRAY_LENGTH(tokens));

    if (jsmn_parsed_count < 0) {
	LOG_ERROR("Failed to parse the json reponse");
	return -1;
    }

    if (jsmn_parsed_count < 1 || tokens[0].type != JSMN_OBJECT) {
	LOG_ERROR("Root of the json response must be an object");
	return -1;
    }

    fill_oauth(oauth, json_res, tokens, jsmn_parsed_count);
    return 0;
}


size_t oauth_cb(char *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    Oauth_Res *oauth = (Oauth_Res *)userp;

    String_View json_response = sv_from_parts(buffer, total_size);
    int ret = parse_json_response(oauth, json_response);
    if (ret == -1) {
	LOG_ERROR("Failed to parse the oauth reponse");
	return 1;
    }

    return total_size;
}


#define OSU_AUTH_PARAMS_LENGTH(creds) sizeof("client_id=") + sizeof("&client_secret") + \
    sizeof("&grant_type=client_credentials&scope=public") + (creds).client_secret->count \
    + (creds).client_id->count


unsigned int get_access_token(CURL *handle, Credentials creds, Oauth_Res *res)
{
    unsigned int result;
    char params[OSU_AUTH_PARAMS_LENGTH(creds) + 1];
    snprintf(params, OSU_AUTH_PARAMS_LENGTH(creds), "client_id="SV_FMT"&client_secret="SV_FMT"&grant_type=client_credentials&scope=public", SV_ARGS(*creds.client_id), SV_ARGS(*creds.client_secret));

    curl_post_req(handle, OSU_AUTH_ENDPOINT, params, oauth_cb, res);

    CURLcode ret = curl_easy_perform(handle);

    if (ret != CURLE_OK) {
	LOG_ERROR("Failed to send a post request to "OSU_AUTH_ENDPOINT);
	defer(1);
    }
    defer(0);
_defer:
    curl_easy_reset(handle);
    return result;
}


size_t user_scores_cb(char *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t buf_count = size * nmemb;
    char **scores = (char **)userp;
    *scores = buffer;

    printf(SV_FMT, (int)buf_count, buffer);
    return buf_count;
}


#define OSU_USER_SCORES_PARAMS_LENGTH \
    sizeof("legacy_only=") + sizeof("&include_fails=") + sizeof("&mode=") + \
    sizeof("&limit=") + sizeof("&offset=")


// NOTE: I can make this even more generic (?!) in the future.
void build_user_scores_url(String_Builder *url, Scores_Params params, char *query, size_t query_len)
{
    da_append_many(url, OSU_USER_SCORES_ENDPOINT, 32);
    da_append_many(url, params.user_id.data, params.user_id.count);
    da_append_many(url, "/scores/", 8);
    da_append_many(url, params.scores_type, strlen(params.scores_type));
    da_append_many(url, "?", 1);
    da_append_many(url, query, query_len);
    da_append_many(url, "\0", 1);
}


// TODO: defer curl_slist_free_all(headers).
// TODO: Add LOG_DEBUG() for when each function.
unsigned int _get_user_scores(CURL *handle, Oauth_Res *oauth, Scores_Params params, char **scores)
{
    unsigned int result;
    const char *mode = ruleset_to_cstr(params.mode);
    char legacy_only = params.legacy_only ? '1' : '0';
    char include_fails = params.include_fails ? '1' : '0';

    // We get the length of the 2 integers
    int l_o_count = snprintf(NULL, 0, "%d%d", params.limit, params.offset);
    size_t total_params_len = strlen(mode) + sizeof(legacy_only) +
	sizeof(include_fails) + l_o_count + OSU_USER_SCORES_PARAMS_LENGTH;

    char query_buf[total_params_len + 1];
    snprintf(query_buf, total_params_len + 1, 
	    "legacy_only=%c&include_fails=%c&mode=%s&limit=%u&offset=%u",
	    legacy_only, include_fails, mode, params.limit, params.offset);

    // We build our full url with the query params.
    // https://osu.ppy.sh/api/v2/users/params.user_id/scores/params.scores_type?{query_buf}
    String_Builder complete_endpoint = { 0 };
    build_user_scores_url(&complete_endpoint, params, query_buf, ARRAY_LENGTH(query_buf));

    // We get the headers for the request reedy.
    const char auth_prefix[] = "Authorization: Bearer ";
    size_t auth_header_len = sizeof(auth_prefix) + oauth->access_token.count;
    char auth_header[auth_header_len];
    snprintf(auth_header, auth_header_len, "%s"SV_FMT, 
	    auth_prefix, SV_ARGS(oauth->access_token));

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(handle, CURLOPT_URL, complete_endpoint.data);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, user_scores_cb);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, scores);
    CURLcode ret = curl_easy_perform(handle);

    free(complete_endpoint.data);


    if (ret != CURLE_OK) {
	LOG_ERROR("Failed to fetch user "SV_FMT" scores", SV_ARGS(params.user_id));
	defer(1);
    }

    defer(0);
_defer:
    curl_easy_reset(handle);
    curl_slist_free_all(headers);
    return result;
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
