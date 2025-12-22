// gcc -o example example.c -lcurl

#include <stdlib.h>
#define COSU_IMPLEMENTATION
#include "cosu.h"
#include <stdio.h>
#include <curl/curl.h> // Must be included 

#define CLIENT_ID 			"123456"
#define CLIENT_SECRET 	"osu OAUTH client secret"

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
		CURL *curl = init_curl(); // function to get the "curl" ready for get/post requests.
		char *res_token = get_access_token(curl, CLIENT_ID, CLIENT_SECRET); // get the json for the guest token
																																				// response is allocated and must be freed by the user.
		char *token = get_token(res_token); // get the access_token from the json response.
																				// this is a helper function and the user can define their own. 
		char *user_best_scores = get_user_score(curl, token, "14586541", "best", 1, 0); // get the user's top play as json.
																																										// user is responsible to freeing this.
																																										
		printf("%s\n", user_best_scores);

		FREE_CURL(curl);
		free(res_token);
		free(token);
		free(user_best_scores);


}
