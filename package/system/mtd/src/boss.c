/*
 * boss.c
 *
 * Copyright (C) 2005 Mike Baker
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <endian.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "mtd.h"
#include "crc32.h"


#define MAGIC_RS353 "TELDAT ClosedEyeVisual\0" // Magic string for RS353 series
#define MAGIC_RS230 "BINTEC ChaosEndDragon\0" // Magic string for RS230 series

struct boss_header {
        char magic[23];
        uint32_t fw_version;
        uint8_t image_type;
        uint32_t image_version;
        uint32_t image_length;
        uint32_t unknown1; // May contains 0X00fa0524. gzip buffer size ?
        uint32_t unknown2; // Fill with 0
        uint32_t crc32;
        uint32_t unknown3; // Fill with 0
}__attribute__ ((packed));

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)           ((((X) & 0x000000FF) << 24) | (((X) & 0x0000FF00) << 8) | (((X) & 0x00FF0000) >> 8) | (((X) & 0xFF000000) >> 24))
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)           (X)
#else
#error unknown endianness!
#endif

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

int
mtd_fixboss(const char *mtd, size_t offset, size_t data_size)
{
	size_t data_offset;
	int fd;
	struct boss_header *boss;
	char *first_block;
	char *buf;
	ssize_t res;
	size_t block_offset;

	if (quiet < 2)
		fprintf(stderr, "Trying to fix BOSS header in %s at 0x%x...\n", mtd, offset);

	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}
	fprintf(stderr, "MTD device %s opened, erase size : %d\n", mtd, erasesize);

	data_offset = offset + sizeof(struct boss_header);

        block_offset = offset & ~(erasesize - 1);
        offset -= block_offset;

        if (data_offset + data_size > mtdsize) {
                fprintf(stderr, "Offset too large, device size 0x%x\n", mtdsize);
                exit(1);
        }

	first_block = malloc(erasesize);
	if (!first_block) {
		perror("malloc");
		exit(1);
	}

	res = pread(fd, first_block, erasesize, block_offset);
	if (res != erasesize) {
		perror("pread");
		exit(1);
	}

	boss = (struct boss_header *)(first_block + offset);
	data_size = erasesize - sizeof(struct boss_header); // Only first sector
	boss->image_length = data_size;
	if (strncmp(MAGIC_RS353, boss->magic, strlen(MAGIC_RS353)) != 0 &&
		strncmp(MAGIC_RS230, boss->magic, strlen(MAGIC_RS230)) != 0) {
		fprintf(stderr, "No BOSS magic found\n");
		exit(1);
	}

	fprintf(stderr, "BOSS header found ! Image Length : %08x\n", data_size);


	buf = malloc(data_size);
	if (!buf) {
		perror("malloc");
		exit(1);
	}

	res = pread(fd, buf, data_size, block_offset + sizeof(struct boss_header));
	if (res != data_size) {
		perror("pread");
		exit(1);
	}

	boss->crc32 = ~crc32buf(buf, data_size);
	if (mtd_erase_block(fd, block_offset)) {
		fprintf(stderr, "Can't erease block at 0x%x (%s)\n", 0, strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "New crc32: 0x%x, rewriting block\n", boss->crc32);

	if (pwrite(fd, first_block, erasesize, block_offset) != erasesize) {
		fprintf(stderr, "Error writing block (%s)\n", strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Done.\n");

	close (fd);
	sync();
	return 0;

}

