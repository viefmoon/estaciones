#include "modbus.h"
#include "HardwareSerial.h"
#include "Stream.h"

uint8_t sensorAddresses[] = {0x01,0x03};

void swap_bytes(uint16_t *byte)
{
    uint8_t *ptr = (uint8_t*)byte;
    uint8_t lb = ptr[1];
    uint8_t hb = ptr[0];
    *byte = lb | (hb << 8U);
}

pHSensorMeasure buffer_to_ph(uint8_t *buffer)
{
    rawPH raw = *((rawPH *)buffer);
    swap_bytes(&raw.ph);
    return {(float)raw.ph / 100.0F};
}

LeafSensorMeasure buffer_to_leaf(uint8_t *buffer)
{
    rawLeaf raw = *((rawLeaf *)buffer);
    swap_bytes(&raw.leaf_humidity);
    swap_bytes(&raw.leaf_temperature);
    // Leaf humidity is divided by 10 because it contains one decimal
    return {(float)raw.leaf_humidity / 10.0F, (float)(raw.leaf_temperature / 100.0F) - 20.0F };
}

SoilSensorMeasure buffer_to_soil(uint8_t *buffer)
{
    rawSoil raw = *((rawSoil *)buffer);

    swap_bytes(&raw.temperature);
    swap_bytes(&raw.moisture);
    swap_bytes(&raw.ec);

    return {(float)raw.temperature / 100.0F, (float)raw.moisture / 100.0F, (float)raw.ec};
}

uint16_t calculate_crc16(uint8_t *buf, uint8_t len)
{  
	uint16_t crc = 0xFFFF;
	unsigned int i = 0;
	char bit = 0;

	for( i = 0; i < len; i++ )
	{
		crc ^= buf[i];

		for( bit = 0; bit < 8; bit++ )
		{
			if( crc & 0x0001 )
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}
	return crc;
}

void RegisterPHMeasure(pHSensorMeasure ph, uint8_t index)
{
    switch (index)
    {
    case 0:
        break;
    case 1:
        break;
    }
}

void RegisterLeafMeasure(LeafSensorMeasure leaf, uint8_t index)
{
    switch (index)
    {
    case 0:
        break;
    case 1:
        break;
    }
}

void RegisterSoilMeasure(SoilSensorMeasure soil, uint8_t index)
{
    switch (index)
    {
    case 0:
        break;
    case 1:
        break;
    }
}

void send_command(Modbus_tx_frame cmd, uint8_t *buffer, uint8_t length)
{
}

bool validate_checksum(uint8_t *buffer, uint8_t length)
{
    uint16_t crc = (buffer[length] << 8U) | buffer[length - 1];
    uint16_t calc_crc = 0xAB;//calculate_crc16(buffer, length - 1);
    return crc == calc_crc;
}

uint8_t detect_type(uint8_t addressSensor){
    uint8_t type_sensor;
    if ((addressSensor >= 0x01) && (addressSensor <= 0x09)){
        //soil sensor (EC,Humidity,temperature) 
        type_sensor = MODBUS_SENSOR_SOIL;
    } 
    if ((addressSensor >= 0x10) && (addressSensor <= 0x1F)){
        //soil sensor (EC,Humidity,temperature) 
        type_sensor = MODBUS_SENSOR_SOIL;
    } 
    else if ((addressSensor >= 0x20) && (addressSensor <= 0x2F)){
        //temperature, humidity, pressure sensor (WSY-301) 
    }
    else if ((addressSensor >= 0x30) && (addressSensor <= 0x3F)){
        //PH sensor (AgCi version) 
        type_sensor = MODBUS_SENSOR_PH;
    }
    else if ((addressSensor >= 0x40) && (addressSensor <= 0x4F)){
        //PH sensor (Electrode Version)
    }
    else if ((addressSensor >= 0x50) && (addressSensor <= 0x5F)){
        //NPK sensor
    }
    else if ((addressSensor >= 0x60) && (addressSensor <= 0x6F)){
        //Water potential sensor (Pressure sensor on tensiometer)
    }
    else if ((addressSensor >= 0x70) && (addressSensor <= 0x7F)){
        //UV radiation sensor
    }
    else if ((addressSensor >= 0x80) && (addressSensor <= 0x8F)){
        //PAR radiation sensor
    }
    else if ((addressSensor >= 0x90) && (addressSensor <= 0x9F)){
        //total radiation sensor
    }
    else if ((addressSensor >= 0xA0) && (addressSensor <= 0xAF)){
        //temperature humidity sensor (metal case)
    }
    else if ((addressSensor >= 0xB0) && (addressSensor <= 0xBF)){
        //leaf sensor (temp, humedity)
        type_sensor = MODBUS_SENSOR_LEAF;
    }
    else if ((addressSensor >= 0xC0) && (addressSensor <= 0xC5)){
        //rain volume sensor (pluviometer)
    }
    else if ((addressSensor >= 0xC6) && (addressSensor <= 0xCA)){
        //wind speed sensor (anemometer)
    }
    return type_sensor;
}

bool ModBus_MakeCMD(uint8_t address, uint8_t function_code)
{
    uint8_t sensor_type = detect_type(address);

    uint8_t dataBytes = 0;

    Modbus_tx_frame cmd = {address, 0x00, 0x0000, 0x0000, 0x0000};

    switch (function_code)
    {
    case MODBUS_CMD_READ:
        cmd.tx_functionCode = MODBUS_CMD_READ_BYTE;
        break;
    }

    switch (sensor_type)
    {
    case MODBUS_SENSOR_SOIL:
        cmd.registerStartAddress = SOIL_START_ADDRESS;
        cmd.registerLength = SOIL_BYTE_LENGHT;
        dataBytes = 6;
        break;
    case MODBUS_SENSOR_LEAF:
        cmd.registerStartAddress = LEAF_START_ADDRESS;
        cmd.registerLength = LEAF_BYTE_LENGHT;
        dataBytes = 4;
        break;
    case MODBUS_SENSOR_PH:
        cmd.registerStartAddress = PH_START_ADDRESS;
        cmd.registerLength = PH_BYTE_LENGHT;
        dataBytes = 2;
        break;
    }

    cmd.tx_crc = calculate_crc16((uint8_t *)&cmd, 6);

    if(dataBytes>0){
        if(MODBUS_SERIAL.write((uint8_t*)&cmd, sizeof(cmd)) == sizeof(cmd)){
            uint8_t buffer[RX_LENGHT_WITHOUT_DATA + dataBytes]; 
            MODBUS_SERIAL.setTimeout(2000);
            if ((MODBUS_SERIAL.readBytes(buffer, RX_LENGHT_WITHOUT_DATA + dataBytes))==(RX_LENGHT_WITHOUT_DATA + dataBytes)){
                //Serial.write(buffer, RX_LENGHT_WITHOUT_DATA + dataBytes);
                uint8_t *buffer_data = &buffer[3]; // the data come after 3 bytes |adress| |cmd| |byte length| |DATA|
                switch (sensor_type)
                {
                case MODBUS_SENSOR_SOIL:
                    SoilSensorMeasure soilMeasures = buffer_to_soil(buffer_data);
                    Serial.print("Soil EC: ");
                    Serial.println(soilMeasures.ec);
                    Serial.print("Soil humidity: ");
                    Serial.println(soilMeasures.humidity);
                    Serial.print("Soil Temperature: ");
                    Serial.println(soilMeasures.temperature);
                    break;
                case MODBUS_SENSOR_LEAF:
                    LeafSensorMeasure leafMeasures = buffer_to_leaf( buffer_data);
                    Serial.print("Leaf humidity: ");
                    Serial.println(leafMeasures.leaf_humidity);
                    Serial.print("Leaf temperature: ");
                    Serial.println(leafMeasures.leaf_temperature);
                    break;
                case MODBUS_SENSOR_PH:
                    pHSensorMeasure pHMeasures = buffer_to_ph( buffer_data);
                    Serial.print("pH: ");
                    Serial.println(pHMeasures.ph);
                    break;
                }
                MODBUS_SERIAL.flush();
            }
            else{
                Serial.println("Longitud de entrada incorrecta");
            }
        }
        else{
            Serial.println("El comando no se escribio correctamente");
        }
    }
    else{
        Serial.println("Sensor no registrado");
    }
}


void makeMeasures()
{
    for (uint8_t i = 0; i < (sizeof(sensorAddresses)/sizeof(uint8_t)); i++) {
        bool state = ModBus_MakeCMD(sensorAddresses[i], MODBUS_CMD_READ);
        if(!state){
            Serial.println("Medicion no realizada");
        }
    }
}


void registerMeasure(uint8_t no_sensor, uint8_t type)
{
    uint8_t * ptr_to_data;
    switch (type)
    {
    case MODBUS_SENSOR_LEAF:
        RegisterLeafMeasure(buffer_to_leaf(ptr_to_data), no_sensor);
        break;
    case MODBUS_SENSOR_PH:
        RegisterPHMeasure(buffer_to_ph(ptr_to_data), no_sensor);
        break;
    case MODBUS_SENSOR_SOIL:
        RegisterSoilMeasure(buffer_to_soil(ptr_to_data), no_sensor);
        break;
    default:
        break;
    }
}
