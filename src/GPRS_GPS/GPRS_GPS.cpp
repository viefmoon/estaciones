#include "GPRS_GPS/GPRS_GPS.h"
#include <HardwareSerial.h>
#include <Adafruit_FONA.h>
#include <ArduinoJson.h>
#include "json/json.h"
#include "macro.h"

const uint16_t BUFF_SIZE = 1536;
char data[BUFF_SIZE];
LocationData _location;
Adafruit_FONA SIM808 = Adafruit_FONA(SIM808_RST_PIN);


void SIM_808::RESET_SIM808()
{
    digitalWrite(SIM808_RST_PIN, LOW);
    delay(2000);
    digitalWrite(SIM808_RST_PIN, HIGH);
    delay(1500);
    digitalWrite(SIM808_RST_PIN, LOW);
    delay(5000);
    digitalWrite(SIM808_RST_PIN, HIGH);
    delay(1500);
    digitalWrite(SIM808_RST_PIN, LOW);
}

void SIM_808::GSM_CheckStatus()
{
    uint8_t n = SIM808.getNetworkStatus();
    Sprintln(F("Fona network status: "));
    if (n == 0){
        Sprintln(F("Not registered: "));}
    if (n == 1){
        Sprintln(F("Registered (home)"));}
    if (n == 2){
        Sprintln(F("Not registered (searching)"));}
    if (n == 3){
        Sprintln(F("Denied"));}
    if (n == 4){
        Sprintln(F("Unknown"));}
    if (n == 5){
        Sprintln(F("Registered roaming"));
    }

    char ccid_buff[22];
    SIM808.getSIMCCID(ccid_buff);
    Sprint(F("Fona SIM CCID: "));
    Sprint(ccid_buff);
}


bool SIM_808::setup_SIM808()
{
    RESET_SIM808();
    SIM808.setGPRSNetworkSettings(F("internet.itelcel.com"), F("webgprs"), F("webgprs2002"));

    SIM808_SERIAL.begin(SIM808_SERIAL_BAUDRATE);

    if (!SIM808.begin(SIM808_SERIAL))
    {
        Sprintln(F("SIM808 do not started"));
        RESET_SIM808();
        return false;
    }
    else
    {   
        Sprintln(F("SIM808 started"));
        GSM_CheckStatus();
    }

    //enable GPS
    if (!SIM808.enableGPS(true))
    {
        Sprintln(F("Can't begin GPS"));
        RESET_SIM808();
        return false;
    }
    else{
        Sprintln(F("GPS begined"));
    }
    
    return true;
}

bool SIM_808::postJson(JsonDocument &json)
{
    bool postStatus = false;

    uint16_t statuscode;
    uint16_t length = 0;

    SIM808.setGPRSNetworkSettings(F("internet.itelcel.com"), F("webgprs"), F("webgprs2002"));
    serializeJson(json, data, BUFF_SIZE);
    SIM_808::GSM_CheckStatus();
    SIM808.enableGPRS(true);
    //uint8_t rssi = SIM808.getRSSI();
    uint8_t gprsState = SIM808.GPRSstate();

    Sprintln(F("------ HTTP POST ------")); 
    Sprintln(F("      HOST: "));
    Sprintln(GPRS_POST_URL);
    Sprintln(F("      BODY: "));
    Sprintln(data);
    Sprintln(F("      GPRS: "));
    Sprintln(gprsState);
    // Sprintln(F("      RSSI: "));
    // Sprintln(rssi);

    postStatus = SIM808.HTTP_POST_start(GPRS_POST_URL,F("application/json"),(uint8_t *)data,strlen(data),&statuscode,&length);

    SIM808.HTTP_POST_end();

    if (!postStatus)
    {
        Sprintln(F("      POST: ERROR"));
    }

    switch (statuscode)
    {
    case HTTP_ERROR:
        Sprintln(F("    STATUS: CAN'T SEND REQUEST"));
        break;
    case HTTP_OK:
        Sprintln(F("    STATUS: OK"));
        break;
    case HTTP_NO_CONTENT:
        Sprintln(F("    STATUS: OK, NO CONTENT"));
        break;
    case HTTP_BAD_REQUEST:
        Sprintln(F("    STATUS: BAD REQUEST"));
        break;
    case HTTP_NOT_FOUND:
        Sprintln(F("    STATUS: NOT FOUND 404"));
        break;
    case HTTP_NETWORK_ERROR:
        Sprintln(F("    STATUS: NETWORK ERROR"));
        //RESET_SIM808();
        break;
    case HTTP_DNS_ERROR:
        Sprintln(F("    STATUS: HOST ERROR"));
        break;
    default:
        Sprint(F("    STATUS: Unkown status code ("));
        Sprint(statuscode);
        Sprintln(F(")"));
        RESET_SIM808();
        break;
    }
    return postStatus || statuscode == 200;
}

void SIM_808::addLocationDevice(LocationData_t location)
{
    //char str_gps[30];
    char str_lon[15];
    char str_lat[15];

    dtostrf(location.longitude, 3, 4, str_lon);
    dtostrf(location.latitude, 3, 4, str_lat);
    //sprintf(str_gps, "%s;%s", str_lat, str_lon);

    DeviceMeasures.addAttribute(location.longitude, F(GPS_LONGITUDE_NAME_JSON));
    DeviceMeasures.addAttribute(location.latitude, F(GPS_LATITUDE_NAME_JSON));
}

void SIM_808::updateGps()
{
    // Serial.println(F("Update GPS.."));
    SIM808.enableGPS(true);
    float lat = 0, lon = 0, spd = 0, head = 0, alt = 0;
    bool rslt = SIM808.getGPS(&lat, &lon, &spd, &head, &alt);
    if (!rslt)
    {
        Sprint(F("Error updating GPS status"));
        Sprint(F("Adding default location"));
        _location.longitude = -103.39556;
        _location.latitude = 20.572337;
        _location.altitude = 1517;

        SIM_808::addLocationDevice(_location);
    }
    else
    {
        // Serial.print("GPS lat:");
        // Serial.println(lat, 6);
        // Serial.print("GPS long:");
        // Serial.println(lon, 6);
        // Serial.print("GPS alt:");
        // Serial.println(alt, 6);
        _location.longitude = lon;
        _location.latitude = lat;
        _location.altitude = alt;

        SIM_808::addLocationDevice(_location);
    }

}


