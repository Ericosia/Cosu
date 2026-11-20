#include <assert.h>
#include <stdbool.h>

#include "cosu.h"

#define ERROR_FMT "--[ERROR]: %s:%s.%d: "
#define LOG_ERROR(err_msg, ...) fprintf(stderr, ERROR_FMT err_msg "\n", \
	__FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define SHIFT_ARGS(arr, count) (assert((count) > 0), (count)--, *(arr)++)

int main(int argc, char *argv[])
{
    char *prog = SHIFT_ARGS(argv, argc);
    if (argc == 0) {
	LOG_ERROR("Not enough arguments. Usage: %s <ENV_FILE>", prog);
	return 1;
    }

    
    Osu osu = { 0 };
    get_client_creds(&osu.creds, argv[0]);

    CURL *handle = init_curl();

    if (get_access_token(handle, osu.creds, &osu.res) != 0) {
	LOG_ERROR("Failed to get access token");
	return 1;
    }

    printf(SV_FMT"\n", SV_ARGS(osu.res.access_token));


    if (get_user_scores(handle, &osu.res, &osu.scores, .mode = OSU,
		.user_id = sv_from_cstr("34263657"),
		.scores_type = "best"
		))
    {
	LOG_ERROR("Failed to load scores");
	return 1;
    }


    // printf("Scores: %s\n", osu.scores);
    return 0;
}

