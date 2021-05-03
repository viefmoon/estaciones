#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include <HardwareSerial.h>

//0x01 to 0x0F addresser available for temperature humidity sensor (PG7)
//0x10 to 0x1F addresses available for soil sensor (EC,Humidity,temperature)
//0x20 to 0x2F addresses available for temperature, humidity, pressure sensor (WSY-301) 
//0x30 to 0x3F addresses available for PH sensor (AgCi version)
//0x40 to 0x4F addresses available for PH sensor (Electrode Version)
//0x50 to 0x5F addresses available for NPK sensor
//0x60 to 0x6F addresses available for Water potential sensor (Pressure sensor on tensiometer)
//0x70 to 0x7F addresses available for UV radiation sensor
//0x80 to 0x8F addresses available for PAR radiation sensor
//0x90 to 0x9F addresses available for Total solar radiation
//0xA0 to 0xAF addresses available for temperature humidity sensor (metal case)
//0xB0 to 0xBF addresses available for Leaf sensor (Temperature, humidity)
//0xC0 to 0xC5 addresses available for rain volume sensor (pluviometer)
//0xC6 to 0xCA addresses available for wind speed sensor (anemometer)

#define MODBUS_CMD_READ_BYTE 0x03

#define RX_LENGHT_WITHOUT_DATA 5
#define MODBUS_TX_BUFFER_LENGHT 8

#define MaxAttempts 5

#define MODBUS_SERIAL Serial2
#define MODBUS_SERIAL_BAUDRATE 9600

////swap bytes 
#define PH_BYTE_LENGHT      0x0100
#define LEAF_BYTE_LENGHT    0x0200
#define SOIL_BYTE_LENGHT    0x0300
#define THP_BYTE_LENGHT     0x0300
#define NPK_BYTE_LENGHT     0x0300

#define PH_START_ADDRESS    0x0100
#define LEAF_START_ADDRESS  0x0200
#define SOIL_START_ADDRESS  0x0000
#define THP_START_ADDRESS   0x0000
#define NPK_START_ADDRESS   0x1E00

extern uint8_t sensorAddresses[];

extern uint8_t soilSensorCounter;
extern uint8_t leafSensorCounter;
extern uint8_t phSensorCounter;
extern uint8_t thpSensorCounter;
extern uint8_t npkSensorCounter;

namespace modbus_structs
{
    //Structs to save cmd
    typedef struct
    {
        uint8_t sensorId;
        uint8_t tx_functionCode;
        uint16_t registerStartAddress;
        uint16_t registerLength;
        uint16_t tx_crc;
    } Modbus_tx_frame;
    //*******************************************

    //Structs to save raw and values from sensors
    typedef struct
    {
        uint16_t ph;
    } rawPH;

    typedef struct
    {
        uint16_t leaf_humidity;
        uint16_t leaf_temperature;
    } rawLeaf;

    typedef struct
    {
        uint16_t moisture;
        uint16_t temperature;
        uint16_t ec;
    } rawSoil;

    typedef struct
    {
        uint16_t temperature;
        uint16_t humidity;
        uint16_t pressure;
    } rawThp;

    typedef struct
    {
        uint16_t nitrogen;
        uint16_t phosphorus;
        uint16_t potassium;
    } rawNpk;
    //*******************************************

    //Structs to save measures
    typedef struct
    {
        float ph;
    } pHSensorMeasure;

    typedef struct 
    {
        float humidity;
        float temperature;
        float ec;
    } SoilSensorMeasure;

    typedef struct
    {
        float leaf_humidity;
        float leaf_temperature;
    } LeafSensorMeasure;


    typedef struct
    {
        float temperature;
        float humidity;
        float pressure;
    } ThpSensorMeasure;

    typedef struct
    {
        float nitrogen;
        float phosphorus;
        float potassium;
    } NpkSensorMeasure;
    //*******************************************
} //End struct namespace

namespace modbus_enum
{
    //Enum 
    typedef enum
    {
        MODBUS_SENSOR_SOIL,
        MODBUS_SENSOR_LEAF,
        MODBUS_SENSOR_PH,
        MODBUS_SENSOR_THP,
        MODBUS_SENSOR_NPK
    } Modbus_SensorTypes;

    typedef enum
    {
        MODBUS_CMD_READ = 3
    } Modbus_SensorsCMD;
    //*******************************************
}//end enum namespace

class MODBUS
{
    public:
    void swap_bytes(uint16_t *);

    modbus_structs::pHSensorMeasure buffer_to_ph(uint8_t *);

    modbus_structs::LeafSensorMeasure buffer_to_leaf(uint8_t *);

    modbus_structs::SoilSensorMeasure buffer_to_soil(uint8_t *);

    modbus_structs::ThpSensorMeasure buffer_to_thp(uint8_t *);

    modbus_structs::NpkSensorMeasure buffer_to_npk(uint8_t *);

    uint16_t calculate_crc16(uint8_t *, uint8_t );

    void RegisterPHMeasure(modbus_structs::pHSensorMeasure, uint8_t );

    void RegisterLeafMeasure(modbus_structs::LeafSensorMeasure , uint8_t );

    void RegisterSoilMeasure(modbus_structs::SoilSensorMeasure , uint8_t );

    void RegisterThpMeasure(modbus_structs::ThpSensorMeasure , uint8_t );

    void RegisterNpkMeasure(modbus_structs::NpkSensorMeasure , uint8_t );

    bool validate_checksum(uint8_t *, uint8_t );

    uint8_t detect_type(uint8_t );

    bool ModBus_MakeCMD(uint8_t , uint8_t);

    void makeMeasures();

    void registerMeasure(uint8_t , uint8_t);
};

#endif