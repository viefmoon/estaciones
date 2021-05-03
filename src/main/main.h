#ifndef MAIN_H
#define MAIN_H

#define SERIAL_BAUDRATE 115200
#define RTC_CODE_MEASURE "s"

#define PostTime      10

extern void checktime();
extern void setAlarm();
extern bool PostData();

#endif