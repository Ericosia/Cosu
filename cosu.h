#ifndef COSU_H_
#define COSU_H_

#include "env.h"

#include <curl/curl.h>

#define OSU_WEB_URL "https://osu.ppy.sh"
/* Supported endpoints. */
#define OSU_AUTH_ENDPOINT OSU_WEB_URL"/oauth/token"
#define OSU_USER_SCORES_ENDPOINT OSU_WEB_URL"/api/v2/users/"

// This is a private struct, modify at your own risk.
typedef struct credentials {
    String_View *client_id;
    String_View *client_secret;
} Credentials, Creds;

typedef enum: unsigned char {
    BEARER = 0,
    ERROR
} TOKEN_TYPE;

// structure for holding all the info from OSU_AUTH_ENDPOINT result
typedef struct {
    TOKEN_TYPE type;
// TODO: I think an ideal way to use the "expires_in" field is to also store the time when
//   we got the response and each time the user (i.e: I) uses any api function we check if
//   the current_time - got_time < expires_in. If yes then we make a new get_access_token()
//   request.
    unsigned long expires_in;
    String_View access_token;
    String_View refresh_token;
} Oauth_Res;

typedef struct {
    Credentials creds;
    Oauth_Res res;
    char *scores;
} Osu;

// Loads the osu client id/secrets from a file.
int get_client_creds(Credentials *creds, const char *file);
CURL *init_curl(void);

unsigned int get_access_token(CURL *handle, Credentials creds, Oauth_Res *res);



typedef enum: unsigned char {
    OSU, TAIKO, FRUITS, MANIA
} RULESET;

// Example: 
// "https://osu.ppy.sh/api/v2/users/1/scores/best?legacy_only=0&include_fails=0&mode=osu&limit=2&offset=1"
typedef struct {
    // URL params
    String_View user_id;
    // TODO: Inconsistency with the previous field.
    // Turn the score_type to sv and/or turn the both to an INT.
    const char *scores_type;

    // Query params (All optional btw).
    unsigned int limit;    // I think this one defaults to a 100 (undocumented).
    unsigned int offset;   // I also think this one defaults to 0.
    RULESET mode;          // Defaults to user's specificed mode.
    bool legacy_only;      // Defaults to false (i.e: to include lazer scores).
    bool include_fails;    // Defaults to false (i.e: to NOT include failed plays).
} Scores_Params;
#define get_user_scores(handle, res, scores, ...) \
    _get_user_scores((handle), (res), (Scores_Params) { \
	.legacy_only = false, .include_fails = false, .limit = 100, .offset = 1, \
	__VA_ARGS__ }, (scores))

unsigned int _get_user_scores(CURL *handle, Oauth_Res *oauth, Scores_Params params, char **scores);

#endif // COSU_H_

