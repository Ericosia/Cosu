#ifndef COSU_H
#define COSU_H

#include <curl/curl.h>

CURL *init_curl();
char *get_access_token(CURL *curl, const char *client_id, const char *client_secret);
char *get_user_score(CURL *curl, const char *access_token, const char *user_id, const char *score_type, 
				const int limit, const int offset);

#endif // COSU_H

#ifdef COSU_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FREE_CURL(curl) curl_easy_cleanup(curl)

static size_t handle_response(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    char **store = (char **)userp;
    char *temp = realloc(*store, (*store ? strlen(*store) : 0) + realsize + 1);

    if (temp == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    *store = temp;
    strncat(*store, (const char *)contents, realsize);
    return realsize;
}

CURL *init_curl ()
{
		CURL *curl = NULL;
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		if (!curl) {
				fprintf(stderr, "ERROR: Failed to init curl!");
				curl_global_cleanup();
		}

		return curl;
}

char *get_access_token(CURL *curl, const char *client_id, const char *client_secret)
{
		CURLcode curl_res;

		char endpoint[1024] = {0};
		snprintf(endpoint, 1024, "client_id=%s&client_secret=%s&grant_type=client_credentials&scope=public",
						client_id, client_secret);

		char *res = calloc(1, sizeof(char));


		curl_easy_setopt(curl, CURLOPT_URL, "https://osu.ppy.sh/oauth/token");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, endpoint);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(endpoint));

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
		
		curl_res = curl_easy_perform(curl);

		if (curl_res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_res));
				res = NULL;
		}
		
		return res;
}

char *get_user_score(CURL *curl, const char *access_token, const char *user_id, const char *score_type, 
				const int limit, const int offset) 
{
		CURLcode curl_res; 
		char auth_header[1024] = {0};
		snprintf(auth_header, 1024, "Authorization: Bearer %s", access_token);
		
		char *res = calloc(1, sizeof(char));

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, auth_header);

		char url[1024*1024] = {0};
		snprintf(url, 1024*1024, "https://osu.ppy.sh/api/v2/users/%s/scores/%s?limit=%d&offset=%d", 
						user_id, score_type, limit, offset);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

		curl_res = curl_easy_perform(curl);

		if (curl_res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_res));
				res = NULL;	
		} 

		curl_slist_free_all(headers);
		return res;
}


#endif // COSU_IMPLEMENTATION


