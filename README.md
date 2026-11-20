# Cosu
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> cdcc0a3 (Readme and commented the example.c)
Cosu is a C wrapper for the osu!api v2. It is a single header library, just copy paste the cosu.h file in your directory and define its implementation:
```C
#define COSU_IMPLEMENTATION
#include "cosu.h"
```

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 3ae3a6d (added compilation to readme)
To compile the code, make sure you link with libcurl:
```bash
gcc -o example example.c -lcurl
```

## API
Currently Cosu only has the "user scores" endpoint, more will be added in the future.
=======
## API
<<<<<<< HEAD
Currently Cosu only has the "user scores" endpoint, more and more will be added in the future.
>>>>>>> cdcc0a3 (Readme and commented the example.c)
=======
Currently Cosu only has the "user scores" endpoint, more will be added in the future.
>>>>>>> 4fee86d (fixed README)

### Get guest token
To get the guest token, you simply call this function:

```C
<<<<<<< HEAD
<<<<<<< HEAD
get_access_token(CURL *curl, const char *client_id, const char *client_secret);
=======
    get_access_token(CURL *curl, const char *client_id, const char *client_secret);
>>>>>>> cdcc0a3 (Readme and commented the example.c)
=======
get_access_token(CURL *curl, const char *client_id, const char *client_secret);
>>>>>>> 4fee86d (fixed README)
```
This function will return a stringified json response that contains many things, among them the access token we need.
The response will be allocated, the user is responsible to free it.

The user can define their own helper function to get the token or use the already defined one: 

```C
<<<<<<< HEAD
<<<<<<< HEAD
get_token(char *res);
=======
    get_token(char *res);
>>>>>>> cdcc0a3 (Readme and commented the example.c)
=======
get_token(char *res);
>>>>>>> 4fee86d (fixed README)
```

This function will allocate a new string and copy the token in it.


### Get user score
To get the user's scores, you use the function below:

```C
<<<<<<< HEAD
<<<<<<< HEAD
get_user_score(CURL *curl, const char *access_token, const char* user_id, const char* score_type, const int limit, const int offset);
=======
    get_user_score(CURL *curl, const char *access_token, const char* user_id, const char* score_type, const int limit, const int offset);
>>>>>>> cdcc0a3 (Readme and commented the example.c)
=======
get_user_score(CURL *curl, const char *access_token, const char* user_id, const char* score_type, const int limit, const int offset);
>>>>>>> 4fee86d (fixed README)
```

This function will allocate a response that contains the "score_type" scores of user "user_id". The limit paramater is for the maximum number of scores to fetch, and the offset is for the pagination.
The user is responsible for freeing the response returned.


<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 4fee86d (fixed README)
### Example

Example on how to use it in your source file: 

```C
#define COSU_IMPLEMENTATION
#include "cosu.h"

<<<<<<< HEAD
<<<<<<< HEAD
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
```

=======
osu v2 api wrapper in C
>>>>>>> 82fc4ec (gitignore and example.c)
=======


>>>>>>> cdcc0a3 (Readme and commented the example.c)
=======
#include <curl/curl.h> // Necessary lib


=======
>>>>>>> aecf4f8 (modified example.c and readme)
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
```

>>>>>>> 4fee86d (fixed README)

