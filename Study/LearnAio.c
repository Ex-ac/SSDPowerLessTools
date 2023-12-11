#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
// #include <libaio.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/aio_abi.h>
#include <string.h>

inline int io_setup(unsigned nr, aio_context_t *ctxp) 
{ 
    return syscall(__NR_io_setup, nr, ctxp); 
} 

inline int io_destroy(aio_context_t ctx) 
{ 
	return syscall(__NR_io_destroy, ctx); 
} 

inline int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) 
{ 
	return syscall(__NR_io_submit, ctx, nr, iocbpp); 
}

inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr, struct io_event *events, struct timespec *timeout) 
{ 
	return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
} 

int main(int argc, char **argv)
{
	int ret;
	int fd = open("/dev/sda", O_RDWR, S_IRWXU);

	if (fd < 0)
	{
		perror("open file failed");
		return ret;
	}

	aio_context_t aio_context = 0;

	ret = io_setup(128, &aio_context);
	if (ret < 0)
	{
		perror("io_setup error aaa");
		return ret;
	}

	struct iocb io_control_block;
	struct iocb *p_io_control_blocks[1];
	struct io_event io_events[1];

	uint8_t data[4096];
	memset(data, 0x5A, 4096);

	memset(&io_control_block, 0, sizeof(io_control_block));
	io_control_block.aio_fildes =fd;
	io_control_block.aio_lio_opcode = IOCB_CMD_PWRITE;
	io_control_block.aio_buf = (uint64_t)(data);
	io_control_block.aio_offset = 0;
	io_control_block.aio_nbytes = 4096;

	p_io_control_blocks[0] = &io_control_block;
	ret = io_submit(aio_context, 1, p_io_control_blocks);
	if (ret < 0)
	{
		perror("io_submit error\n");
		return ret;
	}

	ret = io_getevents(aio_context, 1, 1, io_events, NULL);
	if (ret < 0)
	{
		perror("io_getevents error\n");
		return ret;
	}

	ret = io_destroy(aio_context);
	if (ret < 0)
	{
		perror("io_destroy error");
		return ret;
	}
	return 0;

}