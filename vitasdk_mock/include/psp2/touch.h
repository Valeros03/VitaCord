#ifndef _PSP2_TOUCH_H_
#define _PSP2_TOUCH_H_
#include <stdint.h>
#define SCE_TOUCH_PORT_MAX_NUM 2
#define SCE_TOUCH_PORT_FRONT 0
#define SCE_TOUCH_PORT_BACK 1
#define SCE_TOUCH_SAMPLING_STATE_START 1
struct SceTouchReport { uint8_t id; uint8_t force; int16_t x; int16_t y; };
struct SceTouchData { uint64_t timeStamp; uint32_t status; uint32_t reportNum; struct SceTouchReport report[6]; };
void sceTouchSetSamplingState(int port, int state);
void sceTouchEnableTouchForce(int port);
void sceTouchPeek(int port, SceTouchData* pData, int nBufs);
#endif
