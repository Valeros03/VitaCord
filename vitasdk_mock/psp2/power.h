#ifndef _PSP2_POWER_H_
#define _PSP2_POWER_H_
#ifdef __cplusplus
extern "C" {
#endif
int scePowerGetBatteryLifePercent(void);
int scePowerIsBatteryCharging(void);
int scePowerIsLowBattery(void);
#ifdef __cplusplus
}
#endif
#endif
