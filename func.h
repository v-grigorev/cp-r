#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct Inf_st Inf;
struct Inf_st {
    char* from;
    char* to;

    char* full_path_to;
    int is_new;
};

void* dir_copy(void* arg);
void* file_copy(void* arg);