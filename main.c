#include <stdlib.h>
#define COSU_IMPLEMENTATION
#include "cosu.h"
#include <stdio.h>
#include <curl/curl.h>

#define CLIENT_ID 			"44659"
#define CLIENT_SECRET 	"qNuab3Pn4qrmJnxpZTkJgnM9B7tktgZ3v8lxEOsC"

char *get_token(char *res)
{
		char *new = strstr(res, "access_token");
		new += strlen("access_token\":\"");

		char *token = calloc((strlen(new) - 2 + 1), sizeof(char));
		strncpy(token, new, strlen(new) - 2);
		
		return token;
}

int main(void)
{
		CURL *curl = init_curl();
		char *res_token = get_access_token(curl, );
		char *token = get_token(res_token);
		char *user_best_scores = get_user_score(curl, token, "14586541", "best", 1, 0);

		printf("%s\n", user_best_scores);

		FREE_CURL(curl);
		free(res_token);
		free(token);
		free(user_best_scores);


}
