#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "pcache.h"

int get_cechad(const char *path)
{
        int fd = open(path, O_RDONLY);
        if (fd < 0){
                return -1;
        }

        struct stat file_stat;
        if ( fstat(fd, &file_stat) < 0 ){
                return -2;
        }

        void *fd_mmap = mmap(NULL, file_stat.st_size, PROT_NONE, MAP_SHARED, fd, 0);
        if ( fd_mmap == MAP_FAILED ){
                return -3;
        }

        unsigned char *mincore_vec = (unsigned char *)calloc(1, (file_stat.st_size+PAGESIZE-1)/PAGESIZE);
        if ( mincore(fd_mmap, file_stat.st_size, mincore_vec) != 0 ){
                return -4;
        }

        int cached = 0, page_index;
        for (page_index = 0; page_index <= file_stat.st_size/PAGESIZE; page_index++) {
            if (mincore_vec[page_index]&1) {
                ++cached;
            }
        }
        close(fd);

        return cached;
}

short number_judge(const char *str)
{
        int i;
        for(i = 0; i < strlen(str); i++){
                if ( *(str + i) < '0' || *(str + i) > '9'){
                        return 1;
                }
        }

        return 0;
}


char *get_cmdline(char *pid)
{
        char path[BUFSIZE];
        sprintf(path, "/proc/%s/cmdline", pid);

        int fd = open(path, O_RDONLY);
        if (fd < 0){
                fprintf(stderr, "open error: %s\n", strerror(errno));
                return (char *)-1;
        }
        unsigned short buf_size = 1;
        char tbuf[BUFSIZE];
        char *buf = (char *)malloc(buf_size * BUFSIZE);
        bzero(buf, buf_size * BUFSIZE);
        ssize_t r_size, n_size = 0;
        while ( r_size = read(fd, tbuf, sizeof(char) * BUFSIZE), r_size > 0 ){
                if ( ( n_size + r_size ) > ( buf_size * BUFSIZE ) ) {
                        buf = (char *)realloc(buf, ++buf_size * BUFSIZE);
                }
                memcpy(buf+n_size, tbuf, r_size);
                n_size += r_size;
        }

        int i;
        for(i = 0; i < n_size; i++){
                if (buf[i] == 0x00 && buf[i+1] != 0x00){
                        memset(&buf[i], ' ', 1);
                }
        }

        return buf;
}

struct PID_CACHE *get_cache_info(short cmdline)
{
        struct PID_CACHE *start_pc, *next_pc;
        start_pc = NULL;

        const char *path = "/proc/";
        char t_path[PATHSIZE], tp_path[PATHSIZE];

        DIR *dir = opendir(path);
        DIR *t_dir;

        struct dirent *pdir, *pt_dir;
        while( pdir = readdir(dir), pdir != NULL ){
                if ( pdir->d_type != DT_DIR || number_judge(pdir->d_name) || atoi(pdir->d_name) == getpid() ) continue;
                strcpy(t_path, path);
                strcat(t_path, pdir->d_name);
                strcat(t_path, "/fd/");

                long cache_size = 0;
                t_dir = opendir(t_path);
                while( pt_dir = readdir(t_dir), pt_dir != NULL ){
                        if ( pt_dir->d_type == DT_LNK ){
                                strcpy(tp_path, t_path);
                                strcat(tp_path, pt_dir->d_name);
                                int page_num;
                                if ( page_num = get_cechad(tp_path), page_num > 0 ) cache_size += page_num * PAGESIZE;
                        }
                }

                if ( cache_size > 0 ){
                        if ( start_pc == NULL ){
                                start_pc = next_pc = (struct PID_CACHE *)malloc(sizeof(struct PID_CACHE));
                        }else{
                                next_pc->next = (struct PID_CACHE *)malloc(sizeof(struct PID_CACHE));
                                next_pc = next_pc->next;
                        }
                        next_pc->pid = pdir->d_name;
                        next_pc->cmdline = cmdline == 1 ? get_cmdline(pdir->d_name) : NULL;
                        next_pc->cache_size = (double)cache_size;
                        next_pc->next = NULL;
                }
        }

        return start_pc;
}
