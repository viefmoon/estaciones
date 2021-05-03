#include "Adafruit_FONA.h"
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <main.h>

//Gprs post hostname server
#define GPRS_POST_SERVER "datospruebas.agricos.mx" 
//Gprs post hostname port
#define GPRS_POST_SERVER_PORT "80"
//Gprs post hostname endpoint
#define GPRS_POST_SERVER_API "/api/Measure"
//Macro to create the url to post the data with gprs
#define GPRS_POST_URL GPRS_POST_SERVER ":" GPRS_POST_SERVER_PORT GPRS_POST_SERVER_API
//Gprs fona baudrate for serial communication

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

extern void GSM_CheckStatus();
extern void setup_SIM808();
extern void RESET_SIM808();
extern bool postJson(JsonDocument &json);
extern LocationData location( );
extern void updateGps();



