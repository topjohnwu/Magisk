/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _INCLUDE_SYS__SYSTEM_PROPERTIES_H
#define _INCLUDE_SYS__SYSTEM_PROPERTIES_H

#ifndef _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#error you should #include <sys/system_properties.h> instead
#else
#include <sys/system_properties.h>

typedef struct prop_msg prop_msg;

#define PROP_AREA_MAGIC   0x504f5250
#define PROP_AREA_VERSION 0xfc6ed0ab
#define PROP_AREA_VERSION_COMPAT 0x45434f76

#define PROP_SERVICE_NAME "property_service"
#define PROP_FILENAME_MAX 1024
#define PROP_FILENAME "/dev/__properties__"

#define PA_SIZE         (128 * 1024)

#define SERIAL_VALUE_LEN(serial) ((serial) >> 24)
#define SERIAL_DIRTY(serial) ((serial) & 1)

__BEGIN_DECLS

struct prop_msg
{
    unsigned cmd;
    char name[PROP_NAME_MAX];
    char value[PROP_VALUE_MAX];
};

#define PROP_MSG_SETPROP 1

/*
** Rules:
**
** - there is only one writer, but many readers
** - prop_area.count will never decrease in value
** - once allocated, a prop_info's name will not change
** - once allocated, a prop_info's offset will not change
** - reading a value requires the following steps
**   1. serial = pi->serial
**   2. if SERIAL_DIRTY(serial), wait*, then goto 1
**   3. memcpy(local, pi->value, SERIAL_VALUE_LEN(serial) + 1)
**   4. if pi->serial != serial, goto 2
**
** - writing a value requires the following steps
**   1. pi->serial = pi->serial | 1
**   2. memcpy(pi->value, local_value, value_len)
**   3. pi->serial = (value_len << 24) | ((pi->serial + 1) & 0xffffff)
*/

#define PROP_PATH_RAMDISK_DEFAULT  "/default.prop"
#define PROP_PATH_SYSTEM_BUILD     "/system/build.prop"
#define PROP_PATH_VENDOR_BUILD     "/vendor/build.prop"
#define PROP_PATH_LOCAL_OVERRIDE   "/data/local.prop"
#define PROP_PATH_FACTORY          "/factory/factory.prop"

/*
** Map the property area from the specified filename.  This
** method is for testing only.
*/
int __system_property_set_filename(const char *filename);

/*
** Initialize the area to be used to store properties.  Can
** only be done by a single process that has write access to
** the property area.
*/
int __system_property_area_init();

/* Read the global serial number of the system properties
**
** Called to predict if a series of cached __system_property_find
** objects will have seen __system_property_serial values change.
** But also aids the converse, as changes in the global serial can
** also be used to predict if a failed __system_property_find
** could in-turn now find a new object; thus preventing the
** cycles of effort to poll __system_property_find.
**
** Typically called at beginning of a cache cycle to signal if _any_ possible
** changes have occurred since last. If there is, one may check each individual
** __system_property_serial to confirm dirty, or __system_property_find
** to check if the property now exists. If a call to __system_property_add
** or __system_property_update has completed between two calls to
** __system_property_area_serial then the second call will return a larger
** value than the first call. Beware of race conditions as changes to the
** properties are not atomic, the main value of this call is to determine
** whether the expensive __system_property_find is worth retrying to see if
** a property now exists.
**
** Returns the serial number on success, -1 on error.
*/
unsigned int __system_property_area_serial();

/* Add a new system property.  Can only be done by a single
** process that has write access to the property area, and
** that process must handle sequencing to ensure the property
** does not already exist and that only one property is added
** or updated at a time.
**
** Returns 0 on success, -1 if the property area is full.
*/
int __system_property_add(const char *name, unsigned int namelen,
			const char *value, unsigned int valuelen);

int __system_property_del(const char *name);

/* Update the value of a system property returned by
** __system_property_find.  Can only be done by a single process
** that has write access to the property area, and that process
** must handle sequencing to ensure that only one property is
** updated at a time.
**
** Returns 0 on success, -1 if the parameters are incorrect.
*/
int __system_property_update(prop_info *pi, const char *value, unsigned int len);

/* Read the serial number of a system property returned by
** __system_property_find.
**
** Returns the serial number on success, -1 on error.
*/
unsigned int __system_property_serial(const prop_info *pi);

/* Wait for any system property to be updated.  Caller must pass
** in 0 the first time, and the previous return value on each
** successive call. */
unsigned int __system_property_wait_any(unsigned int serial);

/*  Compatibility functions to support using an old init with a new libc,
 ** mostly for the OTA updater binary.  These can be deleted once OTAs from
 ** a pre-K release no longer needed to be supported. */
// const prop_info *__system_property_find_compat(const char *name);
// int __system_property_read_compat(const prop_info *pi, char *name, char *value);
// int __system_property_foreach_compat(
//         void (*propfn)(const prop_info *pi, void *cookie),
//         void *cookie);

/* Initialize the system properties area in read only mode.
 * Should be done by all processes that need to read system
 * properties.
 *
 * Returns 0 on success, -1 otherwise.
 */
int __system_properties_init();

__END_DECLS

#endif
#endif
