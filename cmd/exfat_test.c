// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <command.h>
#include <fs.h>
#include <malloc.h>
#include <memalign.h>

#define FS_TYPE_EXFAT 9

static int do_exfatcat(struct cmd_tbl *cmdtp, int flag,
		       int argc, char * const argv[])
{
	loff_t size, actread;
	void *buf;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	/*
	 * argv[1] = interface (mmc)
	 * argv[2] = dev:part (0:1)
	 * argv[3] = filename
	 */

	ret = fs_set_blk_dev(argv[1], argv[2], FS_TYPE_EXFAT);
	if (ret) {
		printf("Failed to set block device\n");
		return CMD_RET_FAILURE;
	}
	
	/* get file size */
	ret = fs_size(argv[3], &size);
	if (ret || size <= 0) {
		printf("Unable to get file size\n");
		return CMD_RET_FAILURE;
	}
	
	printf("%s fs_size = %lld ++\n", __func__,size);
	
	buf = memalign(ARCH_DMA_MINALIGN, ALIGN(size + 1, ARCH_DMA_MINALIGN));
	if (!buf) {
		printf("Out of memory\n");
		return CMD_RET_FAILURE;
	}
	memset(buf, 0, size + 1);

	ret = fs_set_blk_dev(argv[1], argv[2], FS_TYPE_EXFAT);
	if (ret) {
		printf("Failed to set block device\n");
		return CMD_RET_FAILURE;
	}
	
	ret = fs_read(argv[3], (ulong)buf, 0, size, &actread);
	if (ret || actread <= 0) {
		printf("File read failed, actread= %d, ret = %d\n", actread, ret);
		free(buf);
		return CMD_RET_FAILURE;
	}
	/* ensure null-terminated text */
	
	((char *)buf)[actread] = '\0';
	printf("%s\n", (char *)buf);

	free(buf);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	exfatcat, 4, 0, do_exfatcat,
	"print text file from exFAT filesystem",
	"<interface> <dev:part> <filename>\n"
	"example:\n"
	"  exfatcat mmc 0:1 test.txt"
);
