#include "func.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Use: %s /source_dir /destination_dir\n", argv[0]);
        return -1;
    }

    pthread_t tid;
    int err;
    Inf* inf = (Inf*)malloc(sizeof(Inf));
    inf->is_new = 0;

    inf->from = realpath(argv[1], NULL);
    if (inf->from == NULL) {
		fprintf(stderr, "main: wrong source path: %s\n", strerror(errno));
		free(inf);
		return -1;
	}

    inf->to = realpath(argv[2], NULL);
    if (inf->to == NULL) {
        err = mkdir(argv[2], 0755);
		if (err == -1) {
			fprintf(stderr, "main: error in creating destination directory '%s': %s\n", argv[2], strerror(errno));
			free(inf);
			return -1;
		}

        inf->is_new = 1;

		inf->to = realpath(argv[2], NULL);
		if (inf->to == NULL) {
			fprintf(stderr, "main: error with destination path '%s' after creation: %s\n", argv[2], strerror(errno));
			free(inf);
			return -1;
		}
	}

    inf->full_path_to = inf->to;

    err = pthread_create(&tid, NULL, dir_copy, (void*)inf);
    if (err != 0) {
        fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
        free(inf);
        return -1;
    }
	
    err = pthread_join(tid, NULL);
    if (err != 0) {
        fprintf(stderr, "main: pthread_join() failed: %s\n", strerror(err));
        free(inf);
        return -1;
    }

    return 0;
}
