#include "GPRS_GPS/GPRS_GPS.h"
#include <HardwareSerial.h>
#include <Adafruit_FONA.h>
#include <ArduinoJson.h>
#include "json/json.h"

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
    Serial.println("Fona network status: ");
    if (n == 0){
        Serial.println("Not registered");}
    if (n == 1){
        Serial.println("Registered (home)");}
    if (n == 2){
        Serial.println("Not registered (searching)");}
    if (n == 3){
        Serial.println("Denied");}
    if (n == 4){
        Serial.println("Unknown");}
    if (n == 5){
        Serial.println("Registered roaming");
    }

    char ccid_buff[22];
    SIM808.getSIMCCID(ccid_buff);
    Serial.print("Fona SIM CCID: ");
    Serial.println(ccid_buff);
}


void SIM_808::setup_SIM808()
{
    RESET_SIM808();
    SIM808.setGPRSNetworkSettings(F("internet.itelcel.com"), F("webgprs"), F("webgprs2002"));
    bool is_SIM808_Begined = false;

    SIM808_SERIAL.begin(SIM808_SERIAL_BAUDRATE);
    is_SIM808_Begined  = SIM808.begin(SIM808_SERIAL);

    if (!is_SIM808_Begined)
    {
        Serial.println("SIM808 do not started");
    }
    else
    {   
        Serial.println("SIM808 started");
        SIM808.enableRTC(true);
        if (!SIM808.enableGPRS(true))
        {
            Serial.println("Can't begin GPRS");
        }
        GSM_CheckStatus();
    }
    //enable GPS
    if (!SIM808.enableGPS(true))
    {
        Serial.println("Can't begin GPS");
    }
    else{
        Serial.println("GPS begined");
    }
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
    uint8_t rssi = SIM808.getRSSI();
    uint8_t gprsState = SIM808.GPRSstate();

    Serial.println(" ------ HTTP POST ------");
    Serial.println("      HOST: ");
    Serial.println(GPRS_POST_URL);
    Serial.println("      BODY: ");
    Serial.println(data);
    Serial.println("      GPRS: ");
    Serial.println(gprsState);
    Serial.println("      RSSI: ");
    Serial.println(rssi);

    postStatus = SIM808.HTTP_POST_start(GPRS_POST_URL,F("application/json"),(uint8_t *)data,strlen(data),&statuscode,&length);

    delay(200);

    SIM808.HTTP_POST_end();

    if (!postStatus)
    {
        Serial.println("      POST: ERROR");
    }

    switch (statuscode)
    {
    case HTTP_ERROR:
        Serial.println("    STATUS: CAN'T SEND REQUEST");
        break;
    case HTTP_OK:
        Serial.println("    STATUS: OK");
        break;
    case HTTP_NO_CONTENT:
        Serial.println("    STATUS: OK, NO CONTENT");
        break;
    case HTTP_BAD_REQUEST:
        Serial.println("    STATUS: BAD REQUEST");
        break;
    case HTTP_NOT_FOUND:
        Serial.println("    STATUS: NOT FOUND 404");
        break;
    case HTTP_NETWORK_ERROR:
        Serial.println("    STATUS: NETWORK ERROR");
        //RESET_SIM808();
        break;
    case HTTP_DNS_ERROR:
        Serial.println("    STATUS: HOST ERROR");
        break;
    default:
        Serial.print("    STATUS: Unkown status code (");
        Serial.print(statuscode);
        Serial.println(")");
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
        Serial.println(F("Error updating GPS status"));
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


