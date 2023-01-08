# pcache - 全局进程cache占用遍历

pcache 用于查看Linux系统下所有在运行进程的page cache占用（需要root身份）

> **背景：**
>
> 京东云上虚拟机每隔一段时间（两小时）会发生一次Load飙高，此云主机是16C的，负载正常不应该超过0.7倍的CPU核数，从监控看，异常的时间点负载达到了40甚至更高，而正常时间点下负载是在1~10之间波动。通过磁盘io监控查看，每次负载异常飙升时间点时都发生了大量的写盘操作。在数据盘io大量写的过程中，此时正好系统内剩余空闲只有0.4G，page cache占了40G，在异常时间点前，内核是在逐步进行异步的内存回收的(pgscank)，通常申请新内存，而系统的空闲内存不足，异步回收的内存满足不了新申请内存的需求时，便使内核触发了同步的直接内存回收（pgscand），这会表现为用户态申请内存的进程堵塞住，申请内存时间长，将导致负载上升。
>
> 
>
> **解决方案：**
>
> Linux操作系统在读文件中的某页的时候（pagesize=4k）,它会首先去查找缓存，看下缓存中是否有要读的那个页面。没有的话，就去后备设备（大多数是块设备）去读上来。读上来对应页面的内容也会缓存在内存中，下一次再读同一个页面的时候，因为缓存中已经有个这个文件，不需要磁盘操作，所以会提升速度。Linux提供了mincore系统调用，这个系统调用用来判断文件的某个页面是否驻留在内存中。可通open获得文件描述符，stat获取文件的长度，页面的大小是4K ，有了长度，我们就知道了，我们需要多少个int来存放结果。mmap建立映射关系，mincore获取文件页面的驻留情况，从起始地址开始，长度是filesize，结果保存在mincore_vec数组里。如果mincore[page_index]&1 == 1,那么表示该页面驻留在内存中，否则没有对应缓存，以此可以遍历进程表以及进程目录下的fd已打开的文件描述符来获取每个进程的page cache占用大小。



```shell
# ./bin/pcache -h
useagr: ./bin/pcache [cmdline]
-c: Show command line
-m: Convert to MB
-g: Convert to GB
```



## 构建

```shell
# 编译
# gcc -o obj/pcache.o -c src/pcache.c -Iinclude
# 链接
# gcc -o bin/pcache obj/pcache.o src/main.c -Iinclude

# 或直接编译链接为可直接文件
# gcc -o bin/pcache src/pcache.c src/main.c -Iinclude
```



## 使用

```shell
# ./pcache
pid: 427 - cache_size: 34324480.000000 Bytes
pid: 460 - cache_size: 319488.000000 Bytes
...
pid: 2378 - cache_size: 21712748544.000000 Bytes
...
pid: 32681 - cache_size: 4096.000000 Bytes

[root@k8s-node-vmdjxo-ooykshc1tv jwping]# ./pcache -g
pid: 427 - cache_size: 0.031967 GB
pid: 460 - cache_size: 0.000298 GB
...
pid: 2378 - cache_size: 20.228130 GB
...
pid: 32681 - cache_size: 0.000004 GB
```
