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

#ifndef BINDSH
#define BINDS_H

typedef void (*bind_cb)(void *arg, int uid, const char *src, const char *dst);
extern int bind_foreach(bind_cb cb, void* arg);
extern int bind_uniq_dst(const char *dst);
extern int bind_remove(const char *path, int uid);
extern void bind_ls(int uid);

typedef void (*init_cb)(void *arg, int uid, const char *path);
extern int init_foreach(init_cb cb, void* arg);
extern int init_uniq(const char *dst);
extern int init_remove(const char *path, int uid);
extern void init_ls(int uid);


#endif /* BINDS_H */
