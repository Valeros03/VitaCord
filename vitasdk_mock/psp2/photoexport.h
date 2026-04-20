#ifndef _PSP2_PHOTOEXPORT_H_
#define _PSP2_PHOTOEXPORT_H_
#ifdef __cplusplus
extern "C" {
#endif
int scePhotoExportFromFile(
    const char* path,
    void* param,
    void* work_mem,
    void* cancel_cb,
    void* user_data,
    char* exportedPath,
    unsigned int exportedPathSize
);
#ifdef __cplusplus
}
#endif
#endif
