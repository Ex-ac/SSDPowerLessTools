#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <libaio.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


#define INIT_DATA		false
#define SECTOR_SIZE		(512)
#define BUFFER_SIZE		(128 * 1024)
#define MAX_SECTOR_NUM		((uint64_t)(20 * 1024 * 1024 * 2))

// inline int io_setup(unsigned nr, aio_context_t *ctxp) 
// { 
//     return syscall(__NR_io_setup, nr, ctxp); 
// } 

// inline int io_destroy(aio_context_t ctx) 
// { 
// 	return syscall(__NR_io_destroy, ctx); 
// } 

// inline int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) 
// { 
// 	return syscall(__NR_io_submit, ctx, nr, iocbpp); 
// }

// inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr, struct io_event *events, struct timespec *timeout) 
// { 
// 	return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
// } 

int main(int argc, char **argv)
{
	int ret;
	
	int fd = open("/dev/sda", O_RDWR | O_DIRECT, S_IRWXU);

	if (fd < 0)
	{
		perror("open file failed");
		return ret;
	}

	io_context_t aio_context = 0;

	ret = io_setup(128, &aio_context);
	if (ret < 0)
	{
		perror("io_setup error aaa");
		return ret;
	}

	struct iocb io_control_block;
	struct iocb *p_io_control_blocks[16];
	struct io_event io_events[16];

	void *buffer;
	if (posix_memalign(&buffer, SECTOR_SIZE, BUFFER_SIZE))
	{
		perror("posix_memalign");
		return 5;
	}

	uint32_t *p_lba_data = (uint32_t *)(buffer);

#if (INIT_DATA)
	memset(data, 0xFF, 128 * 1024);
#else
	memset(buffer, 0x5A, BUFFER_SIZE);
#endif

	int start_lba = MAX_SECTOR_NUM;


	p_io_control_blocks[0] = &io_control_block;

	int count = 0;
	do 
	{
		io_prep_pwrite(&io_control_block, fd, buffer, BUFFER_SIZE, start_lba * SECTOR_SIZE);

		ret = io_submit(aio_context, 1, p_io_control_blocks);

		if (ret < 0)
		{
			fprintf(stderr, "IO.SUBMIT.ERR %d\n", ret);
			return ret;
		}

		ret = io_getevents(aio_context, 1, 1, io_events, NULL);
		if (ret < 0)
		{
			perror("io_getevents error");
			return ret;
		}

		fprintf(stdout, "IO.STAT %d %d %ld/%ld\n", count, start_lba, io_events[0].res, io_events[0].res2);

		if (io_events[0].res <= 0)
		{
			fprintf(stdout, "IO.ERROR %d %d %ld/%ld\n", count, start_lba, io_events[0].res, io_events[0].res2);
			break;
		}

		start_lba += 256000;
		
		// if (start_lba >= MAX_SECTOR_NUM)
		// {
		// 	start_lba = 0;
		// }

		count ++;
		// usleep(10);
		if (count == (20 * 1024 * 1024 / 128))
		{
			break;
		}
	} while (true);

	ret = io_destroy(aio_context);
	if (ret < 0)
	{
		perror("io_destroy error");
		return ret;
	}
	return 0;

}