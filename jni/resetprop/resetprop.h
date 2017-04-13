/* resetprop.h - API for resetprop
 */

#ifndef _RESETPROP_H_
#define _RESETPROP_H_

#ifdef __cplusplus
extern "C" {
#endif

int init_resetprop();
int setprop(const char *name, const char *value);
int setprop2(const char *name, const char *value, int trigger);
char *getprop(const char *name);
int deleteprop(const char *name);

#ifdef __cplusplus
}
#endif

#endif
