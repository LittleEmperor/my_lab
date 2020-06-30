#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>
#include <string.h>

/**
 * 将数据段remap到1块共享内存上，
 * 产生core文件时，默认输出elf文件中的初始值
 * 当进程的coredump_filter从默认的0x33改成0x7f时，才输出共享内存的值。
 *
 * 因为默认0x33时不输出共享内存到core文件中
 * */

char __tos_shared_start[1], __tos_shared_end[1];
int __attribute__((section(".my_shared"))) hello = 1;
int __attribute__((section(".my_shared"))) world = 1;

int main()
{
	printf("tos_shared_start:%x, tos_shared_end:%x\n", __tos_shared_start, __tos_shared_end);

	unsigned long size = __tos_shared_end - __tos_shared_start;
	int fd = -1;
	void *ptr;

	if (!size)
		return 0;

	fd = shm_open("hello_shm", O_RDWR|O_CREAT, 0777);
	if (fd < 0)
		printf("open shm failed\n");
	ftruncate(fd, size);
	printf("shm open ok\n");

	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		printf("mmap failed.\n");
	printf("mmap ok\n");

	memcpy(ptr, __tos_shared_start, size);
	printf("memcyp ok\n");
	munmap(ptr, size);
	printf("munmap ok\n");

	ptr = mmap(__tos_shared_start, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED|MAP_LOCKED, fd, 0);
	if (ptr == MAP_FAILED)
		printf("mremap failed.\n");
	printf("mremap ok\n");

	/*
	if (fd != -1) {
		close(fd);
		printf("shm close ok\n");
	}
	*/

	hello = 123;
	world = 456;
	getchar();
	fd = *(int *)0;

	return 0;
}

