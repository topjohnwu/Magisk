#ifndef IMG_H
#define IMG_H

int create_img(const char *img, int size);
int resize_img(const char *img, int size);
char *mount_image(const char *img, const char *target);
int umount_image(const char *target, const char *device);
int merge_img(const char *source, const char *target);
int trim_img(const char *img, const char *mount, char *loop);

#endif //IMG_H
