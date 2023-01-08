#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include "pcache.h"

int main(int argc, char *argv[])
{
        int ch;
        short cmdline = 0, mb = 0, gb = 0;

        while(ch = getopt(argc, argv, "cmgh"), ch != -1){
                switch(ch){
                        case 'c':
                                cmdline = 1;
                                break;
                        case 'm':
                                mb = 1;
                                break;
                        case 'g':
                                gb = 1;
                                break;
                        case 'h':
                                fprintf(stderr, "useagr: %s [cmdline]\n"
                                        "-c: Show command line\n"
                                        "-m: Convert to MB\n"
                                        "-g: Convert to GB\n", argv[0]);
                                return -1;
                        case '?':
                                fprintf(stderr, "Unknown option: %c\n",(char)optopt);
                                return -2;
                }
        }

        struct PID_CACHE *link_pc = get_cache_info(cmdline);
        while( link_pc ){
                printf("pid: %s - ", link_pc->pid);
                if ( cmdline != 0 )
                        printf("cmdline: %s - ", link_pc->cmdline);
                printf("cache_size: %lf %s\n", mb ? link_pc->cache_size/1048576  : gb ? link_pc->cache_size/1073741824 : link_pc->cache_size, mb ? "MB" : gb ? "GB" : "Bytes" );

                link_pc = link_pc->next;
        }

        return 0;
}
