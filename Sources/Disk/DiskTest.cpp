#include <gtest/gtest.h>
#include "Disk.h"

TEST(DiskTest, BaseTest)
{
	Disk_t *pDisk = Disk_Create("/dev/sda", NULL, 20 * 1024 * 1024 * 2);

	if (pDisk != NULL)
	{
		printf("Create success\n");
		printf("Now destroy it\n");
		Disk_Destroy(pDisk);
	}
}