#ifndef MAGISKMANAGER_ZIPADJUST_H_H
#define MAGISKMANAGER_ZIPADJUST_H_H

int zipadjust(int decompress);

extern size_t insize, outsize, alloc;
extern unsigned char *fin, *fout;

#endif //MAGISKMANAGER_ZIPADJUST_H_H
