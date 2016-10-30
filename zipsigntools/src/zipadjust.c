/*
 * Copyright (C) 2013 Jorrit "Chainfire" Jongma
 * Copyright (C) 2013 The OmniROM Project
 */
/*
 * This file is part of OpenDelta.
 *
 * OpenDelta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDelta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDelta. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <zlib.h>

#pragma pack(1)
struct local_header_struct {
	uint32_t signature;
	uint16_t extract_version;
	uint16_t flags;
	uint16_t compression_method;
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint32_t crc32;
	uint32_t size_compressed;
	uint32_t size_uncompressed;
	uint16_t length_filename;
	uint16_t length_extra;
	// filename
	// extra
};
typedef struct local_header_struct local_header_t;

#pragma pack(1)
struct data_descriptor_struct {
	uint32_t signature;
	uint32_t crc32;
	uint32_t size_compressed;
	uint32_t size_uncompressed;
};
typedef struct data_descriptor_struct data_descriptor_t;

#pragma pack(1)
struct central_header_struct {
	uint32_t signature;
	uint16_t version_made;
	uint16_t version_needed;
	uint16_t flags;
	uint16_t compression_method;
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint32_t crc32;
	uint32_t size_compressed;
	uint32_t size_uncompressed;
	uint16_t length_filename;
	uint16_t length_extra;
	uint16_t length_comment;
	uint16_t disk_start;
	uint16_t attr_internal;
	uint32_t attr_external;
	uint32_t offset;
	// filename
	// extra
	// comment
};
typedef struct central_header_struct central_header_t;

#pragma pack(1)
struct central_footer_struct {
	uint32_t signature;
	uint16_t disk_number;
	uint16_t disk_number_central_directory;
	uint16_t central_directory_entries_this_disk;
	uint16_t central_directory_entries_total;
	uint32_t central_directory_size;
	uint32_t central_directory_offset;
	uint16_t length_comment;
	// comment
};
typedef struct central_footer_struct central_footer_t;

#define MAGIC_LOCAL_HEADER 0x04034b50
#define MAGIC_DATA_DESCRIPTOR 0x08074b50
#define MAGIC_CENTRAL_HEADER 0x02014b50
#define MAGIC_CENTRAL_FOOTER 0x06054b50

static int xerror(char* message) {
	fprintf(stderr, "%s\n", message);
	return 0;
}

static int xseekread(int fd, off_t offset, void* buf, size_t bytes) {
	if (lseek(fd, offset, SEEK_SET) == (off_t)-1) return xerror("Seek failed");
	if (read(fd, buf, bytes) != bytes) return xerror("Read failed");
	return 1;
}

static int xseekwrite(int fd, off_t offset, void* buf, size_t bytes) {
	if (lseek(fd, offset, SEEK_SET) == (off_t)-1) return xerror("Seek failed");
	if (write(fd, buf, bytes) != bytes) return xerror("Write failed");
	return 1;
}

static int xfilecopy(int fdIn, int fdOut, off_t offsetIn, off_t offsetOut, size_t bytes) {
	if ((offsetIn != (off_t)-1) && (lseek(fdIn, offsetIn, SEEK_SET) == (off_t)-1)) return xerror("Seek failed");
	if ((offsetOut != (off_t)-1) && (lseek(fdOut, offsetOut, SEEK_SET) == (off_t)-1)) return xerror("Seek failed");
	
	int CHUNK = 256 * 1024;
	void* buf = malloc(CHUNK);
	if (buf == NULL) return xerror("malloc failed");
	size_t left = bytes;
	while (left > 0) {
		size_t wanted = (left < CHUNK) ? left : CHUNK;
		size_t r = read(fdIn, buf, wanted);
		if (r <= 0) return xerror("Read failed");
		if (write(fdOut, buf, r) != r) return xerror("Write failed");
		left -= r;
	}
	free(buf);
	
	return 1;
}

static int xdecompress(int fdIn, int fdOut, off_t offsetIn, off_t offsetOut, size_t bytes) {
	if ((offsetIn != (off_t)-1) && (lseek(fdIn, offsetIn, SEEK_SET) == (off_t)-1)) return xerror("Seek failed");
	if ((offsetOut != (off_t)-1) && (lseek(fdOut, offsetOut, SEEK_SET) == (off_t)-1)) return xerror("Seek failed");
	
	int CHUNK = 256 * 1024;
	
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];
	
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, -15);
	if (ret != Z_OK) return xerror("ret != Z_OK");
	
	do {
		strm.avail_in = read(fdIn, in, CHUNK);
		if (strm.avail_in < 0) {
			(void)inflateEnd(&strm);
			return xerror("Read failed");
		}
		if (strm.avail_in == 0) break;
		strm.next_in = in;
		
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			
			ret = inflate(&strm, Z_NO_FLUSH);
			if (ret == Z_STREAM_ERROR) return xerror("Stream error");
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return xerror("DICT/DATA/MEM error");
			}
			
			have = CHUNK - strm.avail_out;
			if (write(fdOut, out, have) != have) {
				(void)inflateEnd(&strm);
				return xerror("Write failed");
			}
		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);
	(void)inflateEnd(&strm);
	
	return ret == Z_STREAM_END ? 1 : 0;
}

int zipadjust(char* filenameIn, char* filenameOut, int decompress) {
	int ok = 0;
	
	int fin = open(filenameIn, O_RDONLY);
	if (fin > 0) {
		unsigned int size = lseek(fin, 0, SEEK_END);
		lseek(fin, 0, SEEK_SET);
		printf("%d bytes\n", size);

		char filename[1024];

		central_footer_t central_footer;
		uint32_t central_directory_in_position = 0;
		uint32_t central_directory_in_size = 0;
		uint32_t central_directory_out_size = 0;

		int i;
		for (i = size - 4; i >= 0; i--) {
			uint32_t magic = 0;
			if (!xseekread(fin, i, &magic, sizeof(uint32_t))) return 0;
			if (magic == MAGIC_CENTRAL_FOOTER) {
				printf("central footer @ %08X\n", i);
				if (!xseekread(fin, i, &central_footer, sizeof(central_footer_t))) return 0;

				central_header_t central_header;
				if (!xseekread(fin, central_footer.central_directory_offset, &central_header, sizeof(central_header_t))) return 0;
				if ( central_header.signature == MAGIC_CENTRAL_HEADER ) {
					central_directory_in_position = central_footer.central_directory_offset;
					central_directory_in_size = size - central_footer.central_directory_offset;
					printf("central header @ %08X (%d)\n", central_footer.central_directory_offset, central_footer.central_directory_size);
					break;
				}
			}
		}

		if (central_directory_in_position == 0) return 0;

		unsigned char* central_directory_in = (unsigned char*)malloc(central_directory_in_size);
		unsigned char* central_directory_out = (unsigned char*)malloc(central_directory_in_size);
		if (!xseekread(fin, central_directory_in_position, central_directory_in, central_directory_in_size)) return 0;
		memset(central_directory_out, 0, central_directory_in_size);

		unlink(filenameOut);
		int fout = open(filenameOut, O_CREAT | O_WRONLY, 0644);
		if (fout > 0) {

			uintptr_t central_directory_in_index = 0;
			uintptr_t central_directory_out_index = 0;

			central_header_t* central_header = NULL;
			local_header_t local_header;

			uint32_t out_index = 0;

			while (1) {
				central_header = (central_header_t*)&central_directory_in[central_directory_in_index];
				if (central_header->signature != MAGIC_CENTRAL_HEADER) break;

				filename[central_header->length_filename] = (char)0;
				memcpy(filename, &central_directory_in[central_directory_in_index + sizeof(central_header_t)], central_header->length_filename);
				printf("%s (%d --> %d) [%08X] (%d)\n", filename, central_header->size_uncompressed, central_header->size_compressed, central_header->crc32, central_header->length_extra + central_header->length_comment);

				local_header_t local_header;
				if (!xseekread(fin, central_header->offset, &local_header, sizeof(local_header_t))) return 0;

				// save and update to next index before we clobber the data
				uint16_t compression_method_old = central_header->compression_method;
				uint32_t size_compressed_old = central_header->size_compressed;
				uint32_t offset_old = central_header->offset;
				uint32_t length_extra_old = central_header->length_extra;
				central_directory_in_index += sizeof(central_header_t) + central_header->length_filename + central_header->length_extra + central_header->length_comment;

				// copying, rewriting, and correcting local and central headers so all the information matches, and no data descriptors are necessary
				central_header->offset = out_index;
				central_header->flags = central_header->flags & !8;
				if (decompress && (compression_method_old == 8)) {
					central_header->compression_method = 0;
					central_header->size_compressed = central_header->size_uncompressed;
				}
				central_header->length_extra = 0;
				central_header->length_comment = 0;
				local_header.compression_method = central_header->compression_method;
				local_header.flags = central_header->flags;
				local_header.crc32 = central_header->crc32;
				local_header.size_uncompressed = central_header->size_uncompressed;
				local_header.size_compressed = central_header->size_compressed;
				local_header.length_extra = 0;

				if (!xseekwrite(fout, out_index, &local_header, sizeof(local_header_t))) return 0;
				out_index += sizeof(local_header_t);
				if (!xseekwrite(fout, out_index, &filename[0], central_header->length_filename)) return 0;
				out_index += central_header->length_filename;

				if (decompress && (compression_method_old == 8)) {
					if (!xdecompress(fin, fout, offset_old + sizeof(local_header_t) + central_header->length_filename + length_extra_old, out_index, size_compressed_old)) return 0;
				} else {
					if (!xfilecopy(fin, fout, offset_old + sizeof(local_header_t) + central_header->length_filename + length_extra_old, out_index, size_compressed_old)) return 0;
				}
				out_index += local_header.size_compressed;

				memcpy(&central_directory_out[central_directory_out_index], central_header, sizeof(central_header_t) + central_header->length_filename);
				central_directory_out_index += sizeof(central_header_t) + central_header->length_filename;
			}
			
			central_directory_out_size = central_directory_out_index;
			central_footer.central_directory_size = central_directory_out_size;
			central_footer.central_directory_offset = out_index;
			central_footer.length_comment = 0;
			if (!xseekwrite(fout, out_index, central_directory_out, central_directory_out_size)) return 0;
			out_index += central_directory_out_size;
			if (!xseekwrite(fout, out_index, &central_footer, sizeof(central_footer_t))) return 0;

			printf("central header @ %08X (%d)\n", central_footer.central_directory_offset, central_footer.central_directory_size);
			printf("central footer @ %08X\n", out_index);

			close(fout);
			ok = 1;
		}

		free(central_directory_in);
		free(central_directory_out);
		close(fin);
	}
	
	return ok;
}
