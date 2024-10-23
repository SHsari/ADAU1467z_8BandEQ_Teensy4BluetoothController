#include <Arduino.h>

#include "8Band_Exported/SigmaStudioFW.h"
#include "8Band_Exported/USER_SETTINGS.h"

#include <memory>
#include <vector>

#include "filters/EQFilter.h"
#include "filters/Peak.h"
#include "filters/Shelf.h"
#include "filters/Pass.h"
#include "CoefMemADDRToArray.h"


#define DEFAULT_DEV_ADDRESS 0x0
#define DATA_LENGTH 8
#define btSerial Serial1
#define filterNumber 8

std::vector<std::unique_ptr<EQFilter>> filters;
Biquad biquads[8];
const Biquad defaultBiquad;

void handlePositionData(uint8_t index);
void handleTypeData(uint8_t index);
void handleXData(uint8_t index);
void handleYData(uint8_t index);
void handleZData(uint8_t index);
void handleBandData(uint8_t index);
void handleOnOffData(uint8_t index);

float readFloatFromSerial();
uint8_t readByteFromSerial();
void writeBiquadToDSP(int index);
void writeBiquadToDSP2(int index);
void writeDefaultBiquadToDSP(int index);
byte* floatToByteArray(float x);


void initFilters() {
    Serial.print("initFilters\n");
    defaultBiquad.normalize();
    initAddressArray();
    filters.push_back(std::make_unique<Peak>());
    filters.push_back(std::make_unique<Peak>());
    filters.push_back(std::make_unique<Peak>());
    filters.push_back(std::make_unique<Peak>());
    filters.push_back(std::make_unique<LowShelf>());
    filters.push_back(std::make_unique<Peak>());
    filters.push_back(std::make_unique<Peak>());
    filters.push_back(std::make_unique<HighShelf>());
    for(int i = 0; i < filterNumber; i++) {
        filters[i]->setBiquad(biquads[i]);
    }
}

void setSampleRate(double fs) {
    sampleRate = fs;
    for(int i=0; i<filterNumber; i++) {
        filters[i]->setCoefs();
        writeBiquadToDSP(i);
    }
}

void readBthSerial() {
    if (btSerial.available()) {

        uint8_t header = btSerial.read();
        uint8_t command = header >> 4;
        uint8_t index = header & 0x0F;
        if (index < filterNumber) {
            switch (command) {
                case 0b1110: // positionData
                    handlePositionData(index);
                    break;
                case 0b0001: // normXData
                    handleXData(index);
                    break;
                case 0b0010: // normYData
                    handleYData(index);
                    break;
                case 0b0011: // normZData
                    handleZData(index);
                    break;
                case 0b0100: // typeData
                    handleOnOffData(index);
                    break;
                case 0b0101: // bandData
                    handleBandData(index);
                    break;
                default:
                    Serial.print("Unknown command received:  ");
                    Serial.print(command, HEX);
                    Serial.print("   index:  ");
                    Serial.println(index, HEX);
                    return;
            }
        }
    }
}

void handlePositionData(uint8_t index) {
    float normX = readFloatFromSerial();
    float normY = readFloatFromSerial();
    Serial.print("index: "); Serial.print(index);
    Serial.print("  positionData->  x: "); Serial.print(normX);
    Serial.print(",   y: "); Serial.println(normY);
    filters[index]->setX(normX);
    filters[index]->setY(normY);
    filters[index]->setCoefs();
    writeBiquadToDSP2(index);
}

void handleOnOffData(uint8_t index) {
    Serial.print("index: "); Serial.print(index);
    Serial.print("  OnOffData: ");
    uint8_t oneByte = readByteFromSerial();
    if(oneByte == 0b1111) { // true
        writeBiquadToDSP(index);
        Serial.println(" On ");
    } else if(oneByte == 0b1000) {
        writeDefaultBiquadToDSP(index);
        Serial.println(" off ");
    } else {
        Serial.print("unexpected Value: ");
        Serial.println(oneByte, HEX);
    }
}

void handleXData(uint8_t index) {
    float normX = readFloatFromSerial();
    filters[index]-> setZ(normX);
    filters[index]-> setCoefs();
    Serial.print("index: "); Serial.print(index);
    Serial.print("  xData: ");
    Serial.println(normX);
    writeBiquadToDSP2(index);
}

void handleYData(uint8_t index) {
    float normY = readFloatFromSerial();
    filters[index]-> setY(normY);
    filters[index]-> setCoefs();
    Serial.print("index: "); Serial.print(index);
    Serial.print("  yData: ");
    Serial.println(normY);
    writeBiquadToDSP2(index);
}

void handleZData(uint8_t index) {
    float normZ = readFloatFromSerial();
    filters[index]-> setZ(normZ);
    filters[index]-> setCoefs();
    Serial.print("index: "); Serial.print(index);
    Serial.print("  zData: ");
    Serial.println(normZ);
    writeBiquadToDSP2(index);
}

void handleBandData(uint8_t index) {
    Serial.print("index: "); Serial.print(index);
    Serial.print("  Band Data, Type: ");
    uint8_t typeInt = readByteFromSerial();
    switch (typeInt) {
        case 0b0001: // Peak
            Serial.print("Peak");
            filters[index] = std::make_unique<Peak>();
            break;
        case 0b0010: // LowPass
            Serial.print("LowPass");
            filters[index] = std::make_unique<LowPass>();
            break;
        case 0b0011: // HighPass
            Serial.print("HighPass");
            filters[index] = std::make_unique<HighPass>();
            break;
        case 0b0100: // LowShelf
            Serial.print("LowShelf");
            filters[index] = std::make_unique<LowShelf>(); 
            break;         
        case 0b0101: // HighShelf
            Serial.print("HighShelf");
            filters[index] = std::make_unique<HighShelf>();
            break;
        default:
            Serial.print("Unknown Filter Type: ");
            Serial.println(typeInt, HEX);
            return;
    }
    filters[index]-> setBiquad(biquads[index]);
    float normX = readFloatFromSerial();
    float normY = readFloatFromSerial();
    float normZ = readFloatFromSerial();
    filters[index]-> setX(normX);
    filters[index]-> setY(normY);
    filters[index]-> setZ(normZ);
    Serial.print("   XYZ Data ->  x: "); Serial.print(normX);
    Serial.print(",   y: "); Serial.print(normY);
    Serial.print(",   z: "); Serial.println(normZ);
    filters[index]-> setCoefs();
    writeBiquadToDSP2(index);
}

float readFloatFromSerial() {
    union {
        uint8_t bytes[4];
        float value;
    } floatData;

    uint8_t i = 0;
    while (i < 4) {
        if (btSerial.available()) {
            floatData.bytes[i] = btSerial.read();
            i++;
        }
    }
    return floatData.value;
}

uint8_t readByteFromSerial() {
    uint8_t dataByte;
    uint8_t i = 0;
    while (i<1) {
        if(btSerial.available()) {
            dataByte = btSerial.read();
            i++;
        }
    }
    return dataByte;
}

void writeBiquadToDSP(int index) {
    Biquad * biquad = &biquads[index];
    CoefADDR * addr = &addrArray[index];

    SIGMA_WRITE_REGISTER_FLOAT(addr->a1, biquad->a1);
    SIGMA_WRITE_REGISTER_FLOAT(addr->a2, biquad->a2);
    SIGMA_WRITE_REGISTER_FLOAT(addr->b0, biquad->b0);
    SIGMA_WRITE_REGISTER_FLOAT(addr->b1, biquad->b1);
    SIGMA_WRITE_REGISTER_FLOAT(addr->b2, biquad->b2);
}

void writeBiquadToDSP2(int index) {
    Biquad * biquad = &biquads[index];
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD0_ADDR, biquad->b2);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD1_ADDR, biquad->b1);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD2_ADDR, biquad->b0);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD3_ADDR, biquad->a2);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD4_ADDR, biquad->a1);
    SIGMA_WRITE_REGISTER_INTEGER(MOD_SAFELOAD_ADDR_SAFELOAD_ADDR, addrArray[index].b2);
    SIGMA_WRITE_REGISTER_INTEGER(MOD_SAFELOAD_NUM_SAFELOAD_LOWER_ADDR, 5);
    SIGMA_WRITE_REGISTER_INTEGER(MOD_SAFELOAD_NUM_SAFELOAD_UPPER_ADDR, 0);
}
void writeDefaultBiquadToDSP(int index) {
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD0_ADDR, defaultBiquad.b2);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD1_ADDR, defaultBiquad.b1);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD2_ADDR, defaultBiquad.b0);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD3_ADDR, defaultBiquad.a2);
    SIGMA_WRITE_REGISTER_FLOAT(MOD_SAFELOAD_DATA_SAFELOAD4_ADDR, defaultBiquad.a1);
    SIGMA_WRITE_REGISTER_INTEGER(MOD_SAFELOAD_ADDR_SAFELOAD_ADDR, addrArray[index].b2);
    SIGMA_WRITE_REGISTER_INTEGER(MOD_SAFELOAD_NUM_SAFELOAD_LOWER_ADDR, 5);
    SIGMA_WRITE_REGISTER_INTEGER(MOD_SAFELOAD_NUM_SAFELOAD_UPPER_ADDR, 0);
}

byte* floatToByteArray(float x) {
    uint32_t y = SIGMASTUDIOTYPE_8_24_CONVERT(x);
    byte* byteArray = new byte[4];

    byteArray[0] = y & 0xFF;
    byteArray[1] = (y >> 8) & 0xFF;
    byteArray[2] = (y >> 16) & 0xFF;
    byteArray[3] = (y >> 24) & 0xFF;
    
    return byteArray;
}

/*
void writeBiquadToDSP(int index) {
    Serial.print("Write Biquad To DSP\n");
    Biquad * biquad = &biquads[index];
    CoefADDR * addr = &addrArray[index];
    byte * coefa1 = floatToByteArray(biquad->a1);
    byte * coefa2 = floatToByteArray(biquad->a2);
    byte * coefb0 = floatToByteArray(biquad->b0);
    byte * coefb1 = floatToByteArray(biquad->b1);
    byte * coefb2 = floatToByteArray(biquad->b2);

    SIGMA_WRITE_REGISTER_BLOCK(DEFAULT_DEV_ADDRESS, addr->a1, DATA_LENGTH, coefa1);
    SIGMA_WRITE_REGISTER_BLOCK(DEFAULT_DEV_ADDRESS, addr->a2, DATA_LENGTH, coefa2);
    SIGMA_WRITE_REGISTER_BLOCK(DEFAULT_DEV_ADDRESS, addr->b0, DATA_LENGTH, coefb0);
    SIGMA_WRITE_REGISTER_BLOCK(DEFAULT_DEV_ADDRESS, addr->b1, DATA_LENGTH, coefb1);
    SIGMA_WRITE_REGISTER_BLOCK(DEFAULT_DEV_ADDRESS, addr->b2, DATA_LENGTH, coefb2);
}*/