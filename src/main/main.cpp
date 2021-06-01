#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Stream.h>
#include <Adafruit_FONA.h>
#include "DS3231/DS3231_Simple.h"
#include "main/main.h"
#include <modbus/modbus.h>
#include "json/json.h"
#include "GPRS_GPS/GPRS_GPS.h"
#include "macro.h"
#include <Wire.h>
#include "Adafruit_ADS1X15/Adafruit_ADS1X15.h"

DS3231_Simple Clock;
DateTime MyDateAndTime;

Adafruit_ADS1015 ads;

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

void Blinky(){
  digitalWrite(LED_BUILTIN, HIGH);   
  delay(300);                      
  digitalWrite(LED_BUILTIN, LOW);   
  delay(300); 
  digitalWrite(LED_BUILTIN, HIGH);   
  delay(300);                      
  digitalWrite(LED_BUILTIN, LOW);   
  delay(300); 
  digitalWrite(LED_BUILTIN, HIGH);   
  delay(300);                      
  digitalWrite(LED_BUILTIN, LOW);   
  delay(600); 
  digitalWrite(LED_BUILTIN, HIGH);   
  delay(600);                      
  digitalWrite(LED_BUILTIN, LOW);   
}

bool postData(){
  //Serialize_Json();
  uint8_t postAttempts = 0;
  while (postAttempts < 3) {
    if (GPRS_MODULE.postJson(jsonRoot)) {
      return true;
    }
    else{
      postAttempts++;
    }
  }
  return false;
}

void CheckBattery(){

  float BattVoltage; 

  BattVoltage = (ads.readADC_SingleEnded(1)*7.4F)/1000.0F;

  DeviceMeasures.addMeasure( BattVoltage, F("BATT"));
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
      CheckBattery();
      bool postState = postData();
      if(postState){
        Sprintln(F("Post Success!"));
        Blinky();
      }
      else{
        Sprintln(F("Post Error"));
        GPRS_MODULE.RESET_SIM808();
      }
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

  pinMode(LED_BUILTIN, OUTPUT);

  setAlarm();

  ads.setGain(GAIN_TWOTHIRDS);     //DEFAULT TWOTHIRDS (para un rango de entrada de +/- 6.144V) (6.144V/32768) = .1875mV per bit
  ads.begin();

  while (!GPRS_MODULE.setup_SIM808()){
  }
}

void loop() {
  checktime(); 
}
