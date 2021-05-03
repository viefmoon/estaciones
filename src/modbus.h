#include <stdint.h>
#include <main.h>
//0x00 to 0x0F Reservados
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

////swap bytes 
#define PH_BYTE_LENGHT      0x0100
#define LEAF_BYTE_LENGHT    0x0200
#define SOIL_BYTE_LENGHT    0x0300

#define PH_START_ADDRESS    0x0100
#define LEAF_START_ADDRESS  0x0200
#define SOIL_START_ADDRESS  0x0000

extern uint8_t sensorAddresses[];

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
//*******************************************

//Enum
typedef enum
{
    MODBUS_SENSOR_SOIL,
    MODBUS_SENSOR_LEAF,
    MODBUS_SENSOR_PH
} Modbus_SensorTypes;

typedef enum
{
    MODBUS_CMD_READ = 3
} Modbus_SensorsCMD;
//*******************************************

extern void swap_bytes(uint16_t *);


extern pHSensorMeasure buffer_to_ph(uint8_t *);


extern LeafSensorMeasure buffer_to_leaf(uint8_t *);


extern SoilSensorMeasure buffer_to_soil(uint8_t *);

extern uint16_t calculate_crc16(uint8_t *, uint8_t );

extern void RegisterPHMeasure(pHSensorMeasure, uint8_t );


extern void RegisterLeafMeasure(LeafSensorMeasure , uint8_t );


extern void RegisterSoilMeasure(SoilSensorMeasure , uint8_t );


extern void send_command(Modbus_tx_frame , uint8_t *, uint8_t );

extern bool validate_checksum(uint8_t *, uint8_t );


extern uint8_t detect_type(uint8_t );

extern bool ModBus_MakeCMD(uint8_t , uint8_t);

extern void makeMeasures();

extern void registerMeasure(uint8_t , uint8_t);
