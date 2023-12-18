#include <gtest/gtest.h>
#include <cstddef>
#include "Disk.h"

TEST(DiskTest, BaseTest)
{
	printf("create disk without verify file\n");
	Disk_t *pDisk = Disk_Create("test.bin", NULL, 5 * 1024 * 1024 * 2);
	ASSERT_NE((void *)(pDisk), nullptr);

	if (pDisk != NULL)
	{
		printf("Create success\n");
		printf("Now destroy it\n");
		Disk_Destroy(pDisk);
	}


	printf("create disk with verify file\n");
	pDisk = Disk_Create("test.bin", "test.bin.VD", 20 * 1024 * 1024 * 2);
	ASSERT_NE((void *)(pDisk), nullptr);

	if (pDisk != NULL)
	{
		printf("Create success\n");
		printf("Now destroy it\n");
		Disk_Destroy(pDisk);
	}
}