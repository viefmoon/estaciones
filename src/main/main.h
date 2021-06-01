#ifndef MAIN_H
#define MAIN_H

#define SERIAL_BAUDRATE 115200
#define RTC_CODE_MEASURE "s"

#define PostTime      5

extern void checktime();
extern void setAlarm();
extern bool PostData();
extern void CheckBattery();
extern void Blinky();

#endif