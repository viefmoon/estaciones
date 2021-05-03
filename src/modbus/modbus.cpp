#include "modbus/modbus.h"
#include "json/json.h"
#include "macro.h"

uint8_t soilSensorCounter=0;
uint8_t leafSensorCounter=0;
uint8_t phSensorCounter=0;
uint8_t thpSensorCounter=0;
uint8_t npkSensorCounter=0;

uint8_t sensorAddresses[] = {0x11,0x10,0x33,0x50};

void MODBUS::swap_bytes(uint16_t *byte)
{
    uint8_t *ptr = (uint8_t*)byte;
    uint8_t lb = ptr[1];
    uint8_t hb = ptr[0];
    *byte = lb | (hb << 8U);
}

modbus_structs::pHSensorMeasure MODBUS::buffer_to_ph(uint8_t *buffer)
{
    modbus_structs::rawPH raw = *((modbus_structs::rawPH *)buffer);
    swap_bytes(&raw.ph);
    return {(float)raw.ph / 100.0F};
}

modbus_structs::LeafSensorMeasure MODBUS::  buffer_to_leaf(uint8_t *buffer)
{
    modbus_structs::rawLeaf raw = *((modbus_structs::rawLeaf *)buffer);
    MODBUS::swap_bytes(&raw.leaf_humidity);
    MODBUS::swap_bytes(&raw.leaf_temperature);
    // Leaf humidity is divided by 10 because it contains one decimal
    return {(float)raw.leaf_humidity / 10.0F, (float)(raw.leaf_temperature / 100.0F) - 20.0F };
}

modbus_structs::SoilSensorMeasure MODBUS::buffer_to_soil(uint8_t *buffer)
{
    modbus_structs::rawSoil raw = *((modbus_structs::rawSoil *)buffer);

    MODBUS::swap_bytes(&raw.temperature);
    MODBUS::swap_bytes(&raw.moisture);
    MODBUS::swap_bytes(&raw.ec);

    return {(float)raw.temperature / 100.0F, (float)raw.moisture / 100.0F, (float)raw.ec};
}

modbus_structs::ThpSensorMeasure MODBUS::buffer_to_thp(uint8_t *buffer)
{
    modbus_structs::rawThp raw = *((modbus_structs::rawThp *)buffer);

    MODBUS::swap_bytes(&raw.temperature);
    MODBUS::swap_bytes(&raw.humidity);
    MODBUS::swap_bytes(&raw.pressure);

    return {((float)raw.temperature / 100.0F)-40.0F, (float)raw.humidity / 100.0F, (float)raw.pressure / 10.0F};
}

modbus_structs::NpkSensorMeasure MODBUS::buffer_to_npk(uint8_t *buffer)
{
    modbus_structs::rawNpk raw = *((modbus_structs::rawNpk *)buffer);

    MODBUS::swap_bytes(&raw.nitrogen);
    MODBUS::swap_bytes(&raw.phosphorus);
    MODBUS::swap_bytes(&raw.potassium);

    return {(float)raw.nitrogen, (float)raw.phosphorus, (float)raw.potassium};
}

uint16_t MODBUS::calculate_crc16(uint8_t *buf, uint8_t len)
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

void MODBUS::RegisterPHMeasure(modbus_structs::pHSensorMeasure ph, uint8_t index)
{
    switch (index)
    {
    case 1:
        DeviceMeasures.addMeasure(ph.ph, F("pH1"));
        break;
    case 2:
        DeviceMeasures.addMeasure(ph.ph, F("pH2"));
        break;
    }
}

void MODBUS::RegisterLeafMeasure(modbus_structs::LeafSensorMeasure leaf, uint8_t index)
{
    switch (index)
    {
    case 1:
        DeviceMeasures.addMeasure(leaf.leaf_humidity, F("LH1"));
        DeviceMeasures.addMeasure(leaf.leaf_temperature, F("LT1"));
        break;
    case 2:
        DeviceMeasures.addMeasure(leaf.leaf_humidity, F("LH2"));
        DeviceMeasures.addMeasure(leaf.leaf_temperature, F("LT2"));
        break;
    }
}

void MODBUS::RegisterSoilMeasure(modbus_structs::SoilSensorMeasure soil, uint8_t index)
{
    switch (index)
    {
    case 1:
        DeviceMeasures.addMeasure(soil.ec, F("sec1"));
        DeviceMeasures.addMeasure(soil.humidity, F("sh1"));
        DeviceMeasures.addMeasure(soil.temperature, F("st_1"));
        break;
    case 2:
        DeviceMeasures.addMeasure(soil.ec, F("sec2"));
        DeviceMeasures.addMeasure(soil.humidity, F("sh2"));
        DeviceMeasures.addMeasure(soil.temperature, F("st_2"));
        break;
    }
}

void MODBUS::RegisterThpMeasure(modbus_structs::ThpSensorMeasure thp, uint8_t index)
{
    switch (index)
    {
    case 1:
        DeviceMeasures.addMeasure(thp.temperature, F("at1"));
        DeviceMeasures.addMeasure(thp.humidity, F("ahh1"));
        DeviceMeasures.addMeasure(thp.pressure, F("app1"));
        break;
    case 2:
        DeviceMeasures.addMeasure(thp.temperature, F("T2"));
        DeviceMeasures.addMeasure(thp.humidity, F("H2"));
        DeviceMeasures.addMeasure(thp.pressure, F("AP2"));
        break;
    }
}

void MODBUS::RegisterNpkMeasure(modbus_structs::NpkSensorMeasure npk, uint8_t index)
{
    switch (index)
    {
    case 1:
        DeviceMeasures.addMeasure(npk.nitrogen, F("nit1"));
        DeviceMeasures.addMeasure(npk.phosphorus, F("pho1"));
        DeviceMeasures.addMeasure(npk.potassium, F("pot1"));
        break;
    case 2:
        DeviceMeasures.addMeasure(npk.nitrogen, F("nit2"));
        DeviceMeasures.addMeasure(npk.phosphorus, F("pho2"));
        DeviceMeasures.addMeasure(npk.potassium, F("pot2"));
        break;
    }
}

bool MODBUS::validate_checksum(uint8_t *buffer, uint8_t length)
{
    uint16_t crc = buffer[length-1]<< 8U | (buffer[length-2]);
    uint16_t calc_crc = calculate_crc16(buffer, length - 2);
    // Serial.println(crc,HEX);
    // Serial.println(calc_crc,HEX);
    return crc == calc_crc;
}

uint8_t MODBUS::detect_type(uint8_t addressSensor){
    uint8_t type_sensor;
    if ((addressSensor >= 0x01) && (addressSensor <= 0x09)){
        //soil sensor (EC,Humidity,temperature) 
    } 
    if ((addressSensor >= 0x10) && (addressSensor <= 0x1F)){
        //soil sensor (EC,Humidity,temperature) 
        type_sensor = modbus_enum::MODBUS_SENSOR_SOIL;
    } 
    else if ((addressSensor >= 0x20) && (addressSensor <= 0x2F)){
        //temperature, humidity, pressure sensor (WSY-301) 
        type_sensor = modbus_enum::MODBUS_SENSOR_THP;
    }
    else if ((addressSensor >= 0x30) && (addressSensor <= 0x3F)){
        //PH sensor (AgCi version) 
        type_sensor = modbus_enum::MODBUS_SENSOR_THP;
    }
    else if ((addressSensor >= 0x40) && (addressSensor <= 0x4F)){
        //PH sensor (Electrode Version)
    }
    else if ((addressSensor >= 0x50) && (addressSensor <= 0x5F)){
        type_sensor = modbus_enum::MODBUS_SENSOR_NPK;
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
        type_sensor = modbus_enum::MODBUS_SENSOR_LEAF;
    }
    else if ((addressSensor >= 0xC0) && (addressSensor <= 0xC5)){
        //rain volume sensor (pluviometer)
    }
    else if ((addressSensor >= 0xC6) && (addressSensor <= 0xCA)){
        //wind speed sensor (anemometer)
    }
    return type_sensor;
}

bool MODBUS::ModBus_MakeCMD(uint8_t address, uint8_t function_code)
{
    uint8_t sensor_type = MODBUS::detect_type(address);
    uint8_t dataBytes = 0;

    modbus_structs::Modbus_tx_frame cmd = {address, 0x00, 0x0000, 0x0000, 0x0000};

    switch (function_code)
    {
    case modbus_enum::MODBUS_CMD_READ:
        cmd.tx_functionCode = MODBUS_CMD_READ_BYTE;
        break;
    }

    switch (sensor_type)
    {
    case modbus_enum::MODBUS_SENSOR_SOIL:
        soilSensorCounter++;
        cmd.registerStartAddress = SOIL_START_ADDRESS;
        cmd.registerLength = SOIL_BYTE_LENGHT;
        dataBytes = 6;
        break;
    case modbus_enum::MODBUS_SENSOR_LEAF:
        leafSensorCounter++;
        cmd.registerStartAddress = LEAF_START_ADDRESS;
        cmd.registerLength = LEAF_BYTE_LENGHT;
        dataBytes = 4;
        break;
    case modbus_enum::MODBUS_SENSOR_PH:
        phSensorCounter++;
        cmd.registerStartAddress = PH_START_ADDRESS;
        cmd.registerLength = PH_BYTE_LENGHT;
        dataBytes = 2;
        break;
    case modbus_enum::MODBUS_SENSOR_THP:
        thpSensorCounter++;
        cmd.registerStartAddress = THP_START_ADDRESS;
        cmd.registerLength = THP_BYTE_LENGHT;
        dataBytes = 6;
        break;
    case modbus_enum::MODBUS_SENSOR_NPK:
        npkSensorCounter++;
        cmd.registerStartAddress = NPK_START_ADDRESS;
        cmd.registerLength = NPK_BYTE_LENGHT;
        dataBytes = 6;
        break;
    }

    cmd.tx_crc = MODBUS::calculate_crc16((uint8_t *)&cmd, 6);

    if(dataBytes>0){
        uint8_t attempts=0;
        bool Check_CRC = false;
        while (Check_CRC == false && attempts < MaxAttempts) {
            uint8_t buffer[RX_LENGHT_WITHOUT_DATA + dataBytes]; 
            attempts++;
            MODBUS_SERIAL.write((uint8_t*)&cmd, sizeof(cmd));
            MODBUS_SERIAL.setTimeout(1000);
            if ((MODBUS_SERIAL.readBytes(buffer, RX_LENGHT_WITHOUT_DATA + dataBytes))==(RX_LENGHT_WITHOUT_DATA + dataBytes)){

                uint8_t *buffer_data = &buffer[3]; // the data come after 3 bytes |adress| |cmd| |byte length| |DATA|

                Check_CRC = validate_checksum(buffer, RX_LENGHT_WITHOUT_DATA + dataBytes);
                if(Check_CRC){
                    switch (sensor_type)
                    {
                    case modbus_enum::MODBUS_SENSOR_SOIL:
                        {
                        modbus_structs::SoilSensorMeasure soilMeasures = MODBUS::buffer_to_soil(buffer_data);
                        // Serial.print("Soil EC: ");
                        // Serial.println(soilMeasures.ec);
                        // Serial.print("Soil humidity: ");
                        // Serial.println(soilMeasures.humidity);
                        // Serial.print("Soil Temperature: ");
                        // Serial.println(soilMeasures.temperature);
                        RegisterSoilMeasure(soilMeasures, soilSensorCounter);
                        return true;
                        }
                        break;
                    case modbus_enum::MODBUS_SENSOR_LEAF:
                        {
                        modbus_structs::LeafSensorMeasure leafMeasures = MODBUS::buffer_to_leaf( buffer_data);
                        // Serial.print("Leaf humidity: ");
                        // Serial.println(leafMeasures.leaf_humidity);
                        // Serial.print("Leaf temperature: ");
                        // Serial.println(leafMeasures.leaf_temperature);
                        RegisterLeafMeasure(leafMeasures, leafSensorCounter);
                        return true;
                        }
                        break;
                    case modbus_enum::MODBUS_SENSOR_PH:
                        {
                        modbus_structs::pHSensorMeasure pHMeasures = MODBUS::buffer_to_ph( buffer_data);
                        // Serial.print("pH: ");
                        // Serial.println(pHMeasures.ph);
                        RegisterPHMeasure(pHMeasures, phSensorCounter);
                        return true;
                        }
                        break;
                    case modbus_enum::MODBUS_SENSOR_THP:
                        {
                        modbus_structs::ThpSensorMeasure thpMeasures = MODBUS::buffer_to_thp(buffer_data);
                        // Serial.print("Atm. Temperature: ");
                        // Serial.println(thpMeasures.humidity);
                        // Serial.print("Atm. humidity: ");
                        // Serial.println(thpMeasures.temperature);
                        // Serial.print("Atm. pressure: ");
                        // Serial.println(thpMeasures.pressure);
                        RegisterThpMeasure(thpMeasures, thpSensorCounter);
                        return true;
                        }
                        break;
                    case modbus_enum::MODBUS_SENSOR_NPK:
                        {
                        modbus_structs::NpkSensorMeasure npkMeasures = MODBUS::buffer_to_npk(buffer_data);
                        // Serial.print("Atm. Temperature: ");
                        // Serial.println(thpMeasures.humidity);
                        // Serial.print("Atm. humidity: ");
                        // Serial.println(thpMeasures.temperature);
                        // Serial.print("Atm. pressure: ");
                        // Serial.println(thpMeasures.pressure);
                        RegisterNpkMeasure(npkMeasures, npkSensorCounter);
                        return true;
                        }
                        break;
                    }
                }
                else{
                    Sprintln(F("BAD CRC "));
                }
            }
            else{
                Sprintln(F("Error en la respuesta"));
            }
            
        }
    }
    else{
        Sprintln(F("Sensor no registrado"));
    }
    return false;
}


void MODBUS::makeMeasures()
{
    for (uint8_t i = 0; i < (sizeof(sensorAddresses)/sizeof(uint8_t)); i++) {
        bool state = MODBUS::ModBus_MakeCMD(sensorAddresses[i], modbus_enum::MODBUS_CMD_READ);
        if(state){
            Sprintln(F("Exito, sensor: "));
            //Serial.println(sensorAddresses[i],HEX);
        }
        else{
            Sprintln(F("Error, sensor: "));
            //Serial.println(sensorAddresses[i],HEX);
        }
    }

    soilSensorCounter=0;
    leafSensorCounter=0;
    phSensorCounter=0;
    thpSensorCounter=0;
    npkSensorCounter=0;
}


void MODBUS::registerMeasure(uint8_t no_sensor, uint8_t type)
{
    uint8_t * ptr_to_data;
    switch (type)
    {
    case modbus_enum::MODBUS_SENSOR_LEAF:
        MODBUS::RegisterLeafMeasure(MODBUS::buffer_to_leaf(ptr_to_data), no_sensor);
        break;
    case modbus_enum::MODBUS_SENSOR_PH:
        MODBUS::RegisterPHMeasure(MODBUS::buffer_to_ph(ptr_to_data), no_sensor);
        break;
    case modbus_enum::MODBUS_SENSOR_SOIL:
        MODBUS::RegisterSoilMeasure(MODBUS::buffer_to_soil(ptr_to_data), no_sensor);
        break;
    case modbus_enum::MODBUS_SENSOR_THP:
        MODBUS::RegisterThpMeasure(MODBUS::buffer_to_thp(ptr_to_data), no_sensor);
        break;
    case modbus_enum::MODBUS_SENSOR_NPK:
        MODBUS::RegisterNpkMeasure(MODBUS::buffer_to_npk(ptr_to_data), no_sensor);
        break;
    default:
        break;
    }
}
