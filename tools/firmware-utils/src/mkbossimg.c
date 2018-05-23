/*
 * Copyright (C) 2009-2013 Sebastien Decourriere <sebtx452@gmail.com>
 * Based on mkbrnimg.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <arpa/inet.h>

#define BPB 8 /* bits/byte */
static uint32_t crc32[1<<BPB];

// Image types
#define IMG_TYPE_BOSS 0x00 // Compressed Boss
#define IMG_TYPE_UBOSS 0x01 // Uncompressed Boss (uBoss)
#define IMG_TYPE_LOGIC 0x02 // Logic aka First Bootloader
#define IMG_TYPE_BOOTMON 0x03 // Bootmonitor aka Second Bootloader
#define IMG_TYPE_FIRMWARE 0x04 // Firmware (Unknown)

#define IMG_VER 0x00000000	// Should always be 0x00

#define UNKNOWN_1 0X00FA0524
#define UNKNOWN_2 0x00000000
#define UNKNOWN_3 0x00000000

#define FW_VERSION 0x002A0000 // Version 42 to be sure the web ui accepts the upgrade

#define MAGIC_RS230 "BINTEC ChaosEndDragon\0" // Magic string for RS230 series
#define MAGIC_RS353 "TELDAT ClosedEyeVisual\0" // Magic string for RS353 series

enum devices {
	RS230 = 1,
	RS353 = 2,
};

char *output_file = "bossImage.img";

struct boss_header {
	char magic[23];
	uint32_t fw_version;
	uint8_t	image_type;
	uint32_t image_version;
	uint32_t image_length;
	uint32_t unknown1; // May contains 0X00fa0524. gzip buffer size ?
	uint32_t unknown2; // Fill with 0
	uint32_t crc32;
	uint32_t unknown3; // Fill with 0
}__attribute__ ((packed));

static uint32_t crc32_poly = 0x2083b8ed;

static void init_crc32()
{
	const uint32_t poly = ntohl(crc32_poly);
	int n;

	for (n = 0; n < 1<<BPB; n++) {
		uint32_t crc = n;
		int bit;

		for (bit = 0; bit < BPB; bit++)
			crc = (crc & 1) ? (poly ^ (crc >> 1)) : (crc >> 1);
		crc32[n] = crc;
	}
}

static uint32_t crc32buf(unsigned char *buf, size_t len)
{
	uint32_t crc = 0xFFFFFFFF;

	for (; len; len--, buf++)
		crc = crc32[(uint8_t)crc ^ *buf] ^ (crc >> BPB);
	return ~crc;
}

static void usage(const char *mess)
{
	fprintf(stderr, "Error: %s\n", mess);
	fprintf(stderr, "Usage: mkbossimg  [rs230 | rs353] image_file\n");
	fprintf(stderr, "\n");
	exit(1);
}


static void appendfile(int outfd, char *path, int bt_type, int kernel) {
	int fd;
	size_t len;
	char *input_file;
	uint32_t crc;

	// mmap input_file
	if ((fd = open(path, O_RDONLY))  < 0
	|| (len = lseek(fd, 0, SEEK_END)) < 0
	|| (input_file = mmap(0, len, PROT_READ, MAP_SHARED, fd, 0)) == (void *) (-1)
	|| close(fd) < 0)
	{
		fprintf(stderr, "Error mapping file '%s': %s\n", path, strerror(errno));
		exit(1);
	}

	/* Ignore the fake BOSS header */
	len -= sizeof(struct boss_header);
	input_file += sizeof(struct boss_header);

	struct boss_header p;
	if (bt_type==RS230)
		strncpy(p.magic, MAGIC_RS230, sizeof(p.magic));
	if (bt_type==RS353)
		strncpy(p.magic, MAGIC_RS353, sizeof(p.magic));
	p.fw_version = htonl(FW_VERSION);
	p.image_version = htonl(IMG_VER);
	p.unknown1 = htonl(UNKNOWN_1);
	p.unknown2 = htonl(UNKNOWN_2);
	p.unknown3 = htonl(UNKNOWN_3);
	if (input_file[0] == (char)0x1F && input_file[1] == (char)0x8B) {
		printf("File type: Compressed BOSS Image\n");
		p.image_type = IMG_TYPE_BOSS;
	} else {
		fprintf(stderr, "Invalid image type !!!%02X\n", input_file[1]);
		exit(1);
	}
	init_crc32();
	crc = crc32buf(input_file, len);
	fprintf(stderr, "crc32 for '%s' is %08x and length %08x.\n", path, crc, (unsigned int)len);

	p.image_length = htonl(len);
	p.crc32 = htonl(crc);
	write(outfd, &p, sizeof(struct boss_header));
	write(outfd, input_file, len);
	munmap(input_file, len);
}

int main(int argc, char **argv)
{
	int outfd;
	int i;
	int bt_type = 0;

	if (argc < 3)
		usage("wrong number of arguments");

	if (strcmp(argv[1], "rs230") == 0)
		bt_type = RS230;
	if (strcmp(argv[1], "rs353") == 0)
		bt_type = RS353;
	if(bt_type == 0) {
		fprintf(stderr, "Invalid router type %s !\n", argv[1]);
		exit(1);
	}

	if ((outfd = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
	{
		fprintf(stderr, "Error opening '%s' for writing: %s\n", output_file, strerror(errno));
		exit(1);
	}

	for (i=2; i<argc; i++) {
		appendfile(outfd, argv[i], bt_type, i == 0);
	}

	close(outfd);
	return EXIT_SUCCESS;
}
