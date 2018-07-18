/*
   Copyright 2016, Pierre-Hugues Husson <phh@phh.me>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <strings.h>
#include <stdint.h>
#include <pwd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/types.h>
#include <selinux/selinux.h>
#include <arpa/inet.h>
#include "binds.h"

int bind_foreach(bind_cb cb, void* arg) {
    int res = 0;
    char *str = NULL;
	int fd = open("/data/su/binds", O_RDONLY);
    if(fd<0)
        return 1;

	off_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

    str = malloc(size);
    if(read(fd, str, size) != size)
        goto error;

    char *base = str;
    while(base < str+size) {
        char *parse_src, *parse_dst;
        int uid;

        char *ptr = memchr(base, 0, size-(base-str));
        if(ptr == NULL)
            goto error;
        sscanf(base, "%d", &uid);

        parse_src = strchr(base, ':');
        if(!parse_src)
            goto error;
        parse_src++;

        parse_dst = strchr(parse_src, ':');
        if(!parse_dst)
            goto error;
        *parse_dst = 0; // Split parse_src string
        parse_dst++;

		cb(arg, uid, parse_src, parse_dst);

        base = ptr+1;
    }

    res = 1;
error:
    if(str) free(str);
    close(fd);
    return res;
}

int bind_uniq_dst(const char *dst) {
    static int _res;
    static const char *_dst;

	_res = 1;
    _dst = dst;
    auto void cb(void *arg, int uid, const char *src, const char *dst) {
        if(strcmp(dst, _dst) == 0)
            _res = 0;
    }
    if(!bind_foreach(cb, NULL))
        return 0;
    return _res;
}

void bind_ls(int uid) {
    static int _uid;
	_uid=uid;
    auto void cb(void *arg, int uid, const char *src, const char *dst) {
		if(_uid == 0 || _uid == 2000 || _uid == uid) {
			fprintf(stderr, "%d %s => %s\n", uid, src, dst);
		}
    }
    bind_foreach(cb, NULL);
}

int bind_remove(const char *path, int uid) {
	static int _found = 0;
    static const char *_path;
	static int _fd;
	static int _uid;


	_path = path;
	_found = 0;
	_uid = uid;

	unlink("/data/su/bind.new");
	_fd = open("/data/su/bind.new", O_WRONLY|O_CREAT, 0600);
	if(_fd<0)
		return 0;

    auto void cb(void *arg, int uid, const char *src, const char *dst) {
		//The one we want to drop
        if(strcmp(dst, _path) == 0 &&
				(_uid == 0 || _uid == 2000 || _uid == uid)) {
			_found = 1;
			return;
		}
		char *str = NULL;
		int len = asprintf(&str, "%d:%s:%s", uid, src, dst);
		write(_fd, str, len+1); //len doesn't include final \0 and we want to write it
		free(str);
    }
    bind_foreach(cb, NULL);
	close(_fd);
	unlink("/data/su/bind");
	rename("/data/su/bind.new", "/data/su/bind");
	return _found;
}

int init_foreach(init_cb icb, void* arg) {
    int res = 0;
    char *str = NULL;
	int fd = open("/data/su/init", O_RDONLY);
    if(fd<0)
        return 1;

	off_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

    str = malloc(size);
    if(read(fd, str, size) != size)
        goto error;

    char *base = str;
    while(base < str+size) {
        char *parsed;
        int uid;

        char *ptr = memchr(base, 0, size-(base-str));
        if(ptr == NULL)
            goto error;
        sscanf(base, "%d", &uid);

        parsed = strchr(base, ':');
        if(!parsed)
            goto error;
        parsed++;


		icb(arg, uid, parsed);

        base = ptr+1;
    }

    res = 1;
error:
    if(str) free(str);
    close(fd);
    return res;
}

int init_uniq(const char *path) {
    static int _res;
    static const char *_path;

	_res = 1;
    _path = path;
    auto void cb(void *arg, int uid, const char *path) {
        if(strcmp(path, _path) == 0)
            _res = 0;
    }
    if(!init_foreach(cb, NULL))
        return 0;
    return _res;
}

int init_remove(const char *path, int uid) {
	static int _found = 0;
    static const char *_path;
	static int fd;
	static int _uid;

	_path = path;
	_found = 0;
	_uid = uid;

	unlink("/data/su/init.new");
	fd = open("/data/su/init.new", O_WRONLY|O_CREAT, 0600);
	if(fd<0)
		return 0;

    auto void cb(void *arg, int uid, const char *path) {
		//The one we want to drop
        if(strcmp(path, _path) == 0 &&
				(_uid == 0 || _uid == 2000 || uid == _uid)) {
			_found = 1;
			return;
		}
		char *str = NULL;
		int len = asprintf(&str, "%d:%s", uid, path);
		write(fd, str, len+1); //len doesn't include final \0 and we want to write it
		free(str);
    }
    init_foreach(cb, NULL);
	close(fd);
	unlink("/data/su/init");
	rename("/data/su/init.new", "/data/su/init");
	return _found;
}

void init_ls(int uid) {
	static int _uid;
	_uid = uid;
    auto void cb(void *arg, int uid, const char *path) {
		if(_uid == 2000 || _uid == 0 || _uid == uid)
			fprintf(stderr, "%d %s\n", uid, path);
    }
    init_foreach(cb, NULL);
}
