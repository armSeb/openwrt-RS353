/*
 *  Bintec firmware parser. Based on mtdsplit-tplink parser.
 *  Copyright (C) 2017 Sebastien Decourriere <sebtx452@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>

#include "mtdsplit.h"

#define BINTEC_NR_PARTS		2
#define BOSS_HEADER_SIZE	0x34	/* 52 Bytes */
#define BOSS_MAGIC_RS353	"TELDAT ClosedEyeVisual"
#define BOSS_MAGIC_RS230	"BINTEC ChaosEndDragon"
#define BINTEC_MIN_ROOTFS_OFFS	0x80000 /* 512KiB */

struct boss_header {
        char magic[22];
        uint32_t fw_version;
        uint8_t image_type;
        uint32_t image_version;
        uint32_t image_length;
        uint32_t unknown1; // May contains 0X00fa0524. gzip buffer size ?
        uint32_t unknown2; // Fill with 0
        uint32_t crc32;
        uint32_t unknown3; // Fill with 0
} __attribute__ ((packed));

static int mtdsplit_parse_bintec(struct mtd_info *master,
				const struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct boss_header hdr;
	size_t hdr_len, retlen;
	size_t rootfs_offset;
	struct mtd_partition *parts;

	int err;

	hdr_len = sizeof(hdr);
	err = mtd_read(master, 0, hdr_len, &retlen, (void *) &hdr);
	if (err)
		return err;

	if (retlen != hdr_len)
		return -EIO;

	if(strncmp(BOSS_MAGIC_RS353, hdr.magic, strlen(BOSS_MAGIC_RS353))!=0 &&
		strncmp(BOSS_MAGIC_RS230, hdr.magic, strlen(BOSS_MAGIC_RS230))!=0)
		return -EINVAL;

	err = mtd_find_rootfs_from(master, BINTEC_MIN_ROOTFS_OFFS,
				   master->size, &rootfs_offset, NULL);

	if (err)
		return err;

	parts = kzalloc(BINTEC_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return BINTEC_NR_PARTS;
}

static struct mtd_part_parser mtdsplit_bintec_parser = {
	.owner = THIS_MODULE,
	.name = "bintec-fw",
	.parse_fn = mtdsplit_parse_bintec,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_bintec_init(void)
{
	register_mtd_parser(&mtdsplit_bintec_parser);

	return 0;
}

subsys_initcall(mtdsplit_bintec_init);
