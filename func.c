#include "func.h"

void* dir_copy(void* arg) {
    Inf* inf = (Inf*)arg;
    pthread_t pthreads[128];
    int pthread_count = 0;

    int err;
    DIR* dir;
    struct stat dir_stat;
    for (int i = 0; i < 5; i++) {
        dir = opendir(inf->from);
        if (dir != NULL) {
            break;
        }
        if (errno == EMFILE) {
            sleep(1);
        }
    }
    if (dir == NULL) {
        fprintf(stderr, "fail openning source directory: %s\n", strerror(errno));
        free(inf);
        pthread_exit(NULL);
    }

    err = mkdir(inf->to, 0755);
    if (err == -1 && errno != EEXIST) {
        fprintf(stderr, "fail creating destination directory: %s\n", strerror(errno));
        closedir(dir);
        free(inf);
        pthread_exit(NULL);
    }

    struct dirent* dirent;
    while ((dirent = readdir(dir)) != NULL) {
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            continue;
        }

        Inf* nw_inf = (Inf*)malloc(sizeof(Inf));
        nw_inf->full_path_to = inf->full_path_to;
        nw_inf->is_new = inf->is_new;

        size_t from_size = strlen(inf->from) + strlen(dirent->d_name) + 2;
        nw_inf->from = (char*)malloc(from_size);
        snprintf(nw_inf->from, from_size, "%s/%s", inf->from, dirent->d_name);

        if (strncmp(inf->full_path_to, nw_inf->from, strlen(inf->full_path_to)) == 0) {
            
            if (nw_inf->is_new == 1) {
                continue;
            }
            nw_inf->is_new = 1;  
        }
        
        size_t to_size = strlen(inf->to) + strlen(dirent->d_name) + 2;
        nw_inf->to = (char*)malloc(to_size);
        snprintf(nw_inf->to, to_size, "%s/%s", inf->to, dirent->d_name);

        err = stat(nw_inf->from, &dir_stat);
        if (err == -1) {
            fprintf(stderr, "fail getting file status: %s\n", strerror(errno));

            printf("%s\n", nw_inf->from);

            free(nw_inf->from);
            free(nw_inf->to);
            free(nw_inf);
            continue;
        }

        if (S_ISDIR(dir_stat.st_mode)) {
            err = pthread_create(&pthreads[pthread_count++], NULL, dir_copy, (void*)nw_inf);
            if (err != 0) {
                fprintf(stderr, "thread: pthread_create() failed: %s\n", strerror(err));
                free(nw_inf->from);
                free(nw_inf->to);
                free(nw_inf);
            }
        }
        else if (S_ISREG(dir_stat.st_mode)) {
            err = pthread_create(&pthreads[pthread_count++], NULL, file_copy, nw_inf);
            if (err != 0) {
                fprintf(stderr, "thread: pthread_create() failed: %s\n", strerror(err));
                free(nw_inf->from);
                free(nw_inf->to);
                free(nw_inf);
            }
        }
        else {
            free(nw_inf->from);
            free(nw_inf->to);
            free(nw_inf);
        }

        if (pthread_count == 128) {
            for (int i = 0; i < pthread_count; i++) {
                err = pthread_join(pthreads[i], NULL);
                if (err != 0) {
                    fprintf(stderr, "thread: pthread_join() failed: %s\n", strerror(err));
                }
            }
            pthread_count = 0;
        }    
    }

    for (int i = 0; i < pthread_count; i++) {
        err = pthread_join(pthreads[i], NULL);
            if (err != 0) {
            fprintf(stderr, "thread: pthread_join() failed: %s\n", strerror(err));
        }
    }

	closedir(dir);
	free(inf->from);
	free(inf->to);
	free(inf);
	pthread_exit(NULL);
}


void* file_copy(void* arg) {
    Inf* inf = (Inf*)arg;
    int fd_from;

    fd_from = open(inf->from, O_RDONLY);
    if (fd_from == -1) {
        fprintf(stderr, "fail in opening source file '%s': %s\n", inf->from, strerror(errno));
        free(inf->from);
        free(inf->to);
        free(inf);
        pthread_exit(NULL);
    }

    int fd_to;
    fd_to = open(inf->to, O_WRONLY | O_CREAT, 0644);
    if (fd_to == -1) {
        fprintf(stderr, "fail in opening destination file '%s': %s\n", inf->to, strerror(errno));
        close(fd_from);
        free(inf->from);
        free(inf->to);
        free(inf);
        pthread_exit(NULL);
    }

    ssize_t bytes_read;
    char buffer[4096];
    while ((bytes_read = read(fd_from, buffer, sizeof(buffer))) > 0) {
        ssize_t write_bytes = write(fd_to, buffer, bytes_read);
        if (write_bytes != bytes_read) {
            fprintf(stderr, "fail writing to destination file '%s': %s\n", inf->to, strerror(errno));
			break;
        }
    }
    if (bytes_read == -1) {
		fprintf(stderr, "fail reading from source file '%s': %s\n", inf->from, strerror(errno));
	}

    close(fd_from);
	close(fd_to);
	free(inf->from);
	free(inf->to);
	free(inf);
	pthread_exit(NULL);
}
