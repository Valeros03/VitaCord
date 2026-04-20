#ifndef _PSP2_IO_STAT_H_
#define _PSP2_IO_STAT_H_

struct SceIoStat {
    int st_mode;
};

#ifdef __cplusplus
extern "C" {
#endif
int sceIoGetstat(const char *file, struct SceIoStat *stat);
int sceIoMkdir(const char *dir, int mode);
#ifdef __cplusplus
}
#endif

#endif
