#ifndef GPRS_GPS_H
#define GPRS_GPS_H

#include <ArduinoJson.h>

#define SIM808_SERIAL Serial1
#define SIM808_SERIAL_BAUDRATE 19200
#define SIM808_RST_PIN 30

//Gprs post hostname server
#define GPRS_POST_SERVER "pomas.agricos.mx" 
//Gprs post hostname port
#define GPRS_POST_SERVER_PORT "80"
//Gprs post hostname endpoint
#define GPRS_POST_SERVER_API "/api/Measure"
//Macro to create the url to post the data with gprs
#define GPRS_POST_URL GPRS_POST_SERVER ":" GPRS_POST_SERVER_PORT GPRS_POST_SERVER_API
//Gprs fona baudrate for serial communication

#define GPS_LONGITUDE_NAME_JSON "t"
#define GPS_LATITUDE_NAME_JSON "g"
#define GPS_LOCATION_KEY_JSON "lt"


typedef enum {
    HTTP_ERROR = 0,
    HTTP_OK = 200,
    HTTP_NO_CONTENT = 204,
    HTTP_BAD_REQUEST = 400,
    HTTP_NOT_FOUND = 404,
    HTTP_NETWORK_ERROR = 601,
    HTTP_DNS_ERROR = 603,
} HTTP_STATUS_CODE_e;

struct LocationData
{
  float longitude;
  float latitude;
  float altitude;
  //String toString();
};

typedef struct LocationData LocationData_t;

class SIM_808
{
  public:
  void GSM_CheckStatus();
  bool setup_SIM808();
  void RESET_SIM808();
  bool postJson(JsonDocument &json);
  void updateGps();
  void addLocationDevice(LocationData_t);
};
#endif

