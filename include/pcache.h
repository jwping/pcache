#ifndef __PCACHE_H__
#define __PCACHE_H__

#define PAGESIZE getpagesize()
#define BUFSIZE 128
#define PATHSIZE 512

struct PID_CACHE {
        char *pid;
        char *cmdline;
        double cache_size;

        struct PID_CACHE *next;
};

extern int get_cached(const char *path);
extern char *get_cmdline(char *pid);
extern struct PID_CACHE *get_cache_info(short cmdline);

#endif
