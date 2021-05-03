#include <Arduino.h>
#include <HardwareSerial.h>
#include <Stream.h>
#include <Adafruit_FONA.h>
#include "DS3231/DS3231_Simple.h"
#include "main/main.h"
#include <modbus/modbus.h>
#include "json/json.h"
#include "GPRS_GPS/GPRS_GPS.h"
#include "macro.h"

DS3231_Simple Clock;
DateTime MyDateAndTime;

MODBUS Sensors;
SIM_808 GPRS_MODULE;

void time_to_String(){
  char arrayCharTime[25];
  if((MyDateAndTime.Minute<10)&&(MyDateAndTime.Hour<10)){
    sprintf(arrayCharTime, "20%d-%d-%dT0%d:0%d:00", MyDateAndTime.Year, MyDateAndTime.Month, MyDateAndTime.Day,MyDateAndTime.Hour,MyDateAndTime.Minute);
  }
  else if(MyDateAndTime.Hour<10){
    sprintf(arrayCharTime, "20%d-%d-%dT0%d:%d:00", MyDateAndTime.Year, MyDateAndTime.Month, MyDateAndTime.Day,MyDateAndTime.Hour,MyDateAndTime.Minute);
  }
  else if(MyDateAndTime.Minute<10){
    sprintf(arrayCharTime, "20%d-%d-%dT%d:0%d:00", MyDateAndTime.Year, MyDateAndTime.Month, MyDateAndTime.Day,MyDateAndTime.Hour,MyDateAndTime.Minute);
  }
  else{
  sprintf(arrayCharTime, "20%d-%d-%dT%d:%d:00", MyDateAndTime.Year, MyDateAndTime.Month, MyDateAndTime.Day,MyDateAndTime.Hour,MyDateAndTime.Minute);
  }
  DeviceMeasures.addAttribute(arrayCharTime, F(RTC_CODE_MEASURE));
}

bool postData(){
  //Serialize_Json();
  uint8_t postAttempts = 0;
  while (postAttempts < 3) {
    if (GPRS_MODULE.postJson(jsonRoot)) {
      Sprintln(F("Post Success!"));
      return true;
    }
    else{
      Sprintln(F("Post Error :( "));
      postAttempts++;
    }
  }
  return false;
}



void checktime(){
  uint8_t AlarmsFired = Clock.checkAlarms();
  if(AlarmsFired & 2)
  {
    MyDateAndTime = Clock.read();
    if(MyDateAndTime.Minute % PostTime  == 00){
      init_Json();
      time_to_String();
      GPRS_MODULE.updateGps();
      Sensors.makeMeasures();
      postData();
    }
  }
}

void setAlarm(){
  Clock.begin();
  Clock.disableAlarms();
  Clock.setAlarm(DS3231_Simple::ALARM_EVERY_MINUTE); 
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  MODBUS_SERIAL.begin(MODBUS_SERIAL_BAUDRATE);

  pinMode(SIM808_RST_PIN, OUTPUT);
  digitalWrite(SIM808_RST_PIN, LOW);

  setAlarm();

  while (!GPRS_MODULE.setup_SIM808()){
  }
}

void loop() {
  checktime(); 
}
