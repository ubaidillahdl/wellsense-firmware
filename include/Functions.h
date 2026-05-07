#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "Global.h"

// System.cpp
void initSistem();
void vibrate();
void bacaTombol();

// Network.cpp
void kirimAT(const char *perintah);
void bacaAT();
void prosesConnecting();
void prosesKirimData();
bool pecahDataFeedback(char *buf);
void resetKeConnecting();

// Display.cpp
void initDisp();
void updateTampilan();
void tampilVitals();
void status();

// Sensor.cpp
void initSensorMAX();
void handleInterrupt();
bool adaTangan();
void bangunSesi();
void prosesStandby();
void prosesSampling();
bool updateDesimasi(DesimasiState &d, int32_t inputVal, int16_t &outputVal);
void normalisasiBuffer();

#endif