# Cosu
<<<<<<< HEAD
Cosu is a C wrapper for the osu!api v2. It is a single header library, just copy paste the cosu.h file in your directory and define its implementation:
```C
#define COSU_IMPLEMENTATION
#include "cosu.h"
```

To compile the code, make sure you link with libcurl:
```bash
gcc -o example example.c -lcurl
```

## API
Currently Cosu only has the "user scores" endpoint, more will be added in the future.

### Get guest token
To get the guest token, you simply call this function:

```C
get_access_token(CURL *curl, const char *client_id, const char *client_secret);
```
This function will return a stringified json response that contains many things, among them the access token we need.
The response will be allocated, the user is responsible to free it.

The user can define their own helper function to get the token or use the already defined one: 

```C
get_token(char *res);
```

This function will allocate a new string and copy the token in it.


### Get user score
To get the user's scores, you use the function below:

```C
get_user_score(CURL *curl, const char *access_token, const char* user_id, const char* score_type, const int limit, const int offset);
```

This function will allocate a response that contains the "score_type" scores of user "user_id". The limit paramater is for the maximum number of scores to fetch, and the offset is for the pagination.
The user is responsible for freeing the response returned.


### Example

Example on how to use it in your source file: 

```C
#define COSU_IMPLEMENTATION
#include "cosu.h"

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

