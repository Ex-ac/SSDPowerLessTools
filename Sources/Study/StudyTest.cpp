
#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

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
#include <gtest/gtest.h>


#define INIT_DATA		false
#define SECTOR_SIZE		(512)
#define BUFFER_SIZE		(128 * 1024)
#define MAX_SECTOR_NUM		((uint64_t)(6 * 1024 * 1024 * 2))

#define FAILED()		ASSERT_EQ(0, 1)

TEST(StudyTest, BaseTest)
{
	int ret;

	int fd = open("test.bin", O_RDWR | O_CREAT | O_DIRECT, S_IRWXU);

	if (fd < 0)
	{
		perror("open file failed");
		FAILED();
	}

	io_context_t aio_context = 0;

	ret = io_setup(128, &aio_context);
	if (ret < 0)
	{
		perror("io_setup error aaa");
		FAILED();
	}

	struct iocb io_control_block;
	struct iocb *p_io_control_blocks[16];
	struct io_event io_events[16];

	void *buffer;
	if (posix_memalign(&buffer, SECTOR_SIZE, BUFFER_SIZE))
	{
		perror("posix_memalign");
		FAILED();
	}

	uint64_t *p_lba_data = (uint64_t *)(buffer);

#if (INIT_DATA)
	memset(data, 0xFF, 128 * 1024);
#else
	memset(buffer, 0x5A, BUFFER_SIZE);
#endif

	unsigned int start_lba = 0;

	p_io_control_blocks[0] = &io_control_block;
	long long offset;
	int count = 0;
	do
	{

		offset = start_lba * SECTOR_SIZE;
		*p_lba_data = (uint64_t)(start_lba);
		io_prep_pwrite(&io_control_block, fd, buffer, BUFFER_SIZE, offset);

		ret = io_submit(aio_context, 1, p_io_control_blocks);

		if (ret < 0)
		{
			fprintf(stderr, "IO.SUBMIT.ERR %d %lld/%ld\n", ret, io_control_block.u.c.offset, io_control_block.u.c.nbytes);
			lseek(fd, 0, SEEK_SET);
			memset(buffer, 0xFF, BUFFER_SIZE);
			write(fd, buffer, BUFFER_SIZE);
			FAILED();
		}

		ret = io_getevents(aio_context, 1, 1, io_events, NULL);
		if (ret < 0)
		{
			perror("io_getevents error");
			FAILED();
		}

		fprintf(stdout, "IO.STAT %d %d %ld/%ld\n", count, start_lba, io_events[0].res, io_events[0].res2);

		if (io_events[0].res <= 0)
		{
			fprintf(stdout, "IO.ERROR %d %d %ld/%ld\n", count, start_lba, io_events[0].res, io_events[0].res2);
			break;
		}

		start_lba += BUFFER_SIZE / SECTOR_SIZE;

		if (start_lba >= MAX_SECTOR_NUM)
		{
			start_lba = 0;
		}

		count++;
		// usleep(10);
		if (count == 10 * (MAX_SECTOR_NUM / (BUFFER_SIZE / SECTOR_SIZE)))
		{
			break;
		}
	} while (true);

	ret = io_destroy(aio_context);
	if (ret < 0)
	{
		perror("io_destroy error");
		FAILED();
	}

	close(fd);

	EXPECT_EQ(1, 1);
}
