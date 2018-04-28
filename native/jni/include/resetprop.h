/* resetprop.h - API for resetprop
 */

#ifndef _RESETPROP_H_
#define _RESETPROP_H_

int prop_exist(const char *name);
int setprop(const char *name, const char *value);
int setprop2(const char *name, const char *value, const int trigger);
char *getprop(const char *name);
char *getprop2(const char *name, int persist);
int deleteprop(const char *name);
int deleteprop2(const char *name, const int persist);
int read_prop_file(const char* filename, const int trigger);
void getprop_all(void (*callback)(const char *, const char *, void *), void *cookie);

#endif
