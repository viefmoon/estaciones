#ifndef MAIN_H
#define MAIN_H

#define SERIAL_BAUDRATE 115200
#define RTC_CODE_MEASURE "s"

<<<<<<< Updated upstream
#define PostTime      5
=======
#define PostTime      2
>>>>>>> Stashed changes

extern void checktime();
extern void setAlarm();
extern bool PostData();
extern void CheckBattery();
extern void Blinky();

#endif