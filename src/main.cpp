#include <Arduino.h>
#include <Wire.h>

#include "Functions.h"
#include "Global.h"

// System
SystemState systemState = SYS_CONNECTING;

// Display
SSD1306AsciiWire oled;
String statusTeks = "";
uint8_t progressLevel = 0;
bool first = true,
     dataUpdate = false;

// Network
SoftwareSerial sim800c(rxSim, txSim);
ConnState connState = CON_WAIT_READY;
SendState sendState = SEND_TCP_OPEN;
uint32_t waktuKirim = millis(),
         waktuPrint = millis();
uint8_t bufIdx = 0,
        retryReg = 0;
bool sudahKirim = false;
char buf[64];

// Sensor
MAX30105 particleSensor;
LpfState filterRed,
    filterIR;
DesimasiState desimRed,
    desimIR;
DataSensor wadah;
HasilVitals dataVitals;
int32_t rawRed,
    filteredRed,
    rawIR,
    filteredIR;
uint8_t bufferIdx = 0;
bool dataReady = false;

// main
uint32_t waktuMulai = millis();
uint8_t sisaDetik = 0;
bool butuhRetryCepat = true;

void setup() {
      initSistem();
      initDisp();
      initSensorMAX();
}

void loop() {
      switch (systemState) {
            case SYS_CONNECTING: {
                  prosesConnecting();
            } break;

            case SYS_IDLE: {
                  uint16_t durasiTunggu = butuhRetryCepat ? 5000 : 10000;
                  if (adaTangan()) {
                        if (millis() - waktuMulai >= durasiTunggu) {
                              bangunSesi();
                              bufferIdx = 0;
                              butuhRetryCepat = true;
                              waktuMulai = millis();
                              systemState = SYS_SAMPLING;
                        } else {
                              sisaDetik = (durasiTunggu - (millis() - waktuMulai)) / 1000;
                              statusTeks = "WAIT: ";
                              statusTeks += sisaDetik + 1;
                              statusTeks += "s";
                        }
                  } else {
                        prosesStandby();
                        statusTeks = "READY";
                        progressLevel = 0;
                  }
            } break;

            case SYS_SAMPLING: {
                  if (adaTangan()) {
                        statusTeks = "SAMPLING";
                        progressLevel = 1;

                        if (dataReady) prosesSampling();
                        if (bufferIdx >= PANJANG_BUFFER) {
                              detachInterrupt(digitalPinToInterrupt(interruptPin));
                              normalisasiBuffer();

                              waktuMulai = millis();
                              systemState = SYS_SENDING;
                        }
                  } else {
                        detachInterrupt(digitalPinToInterrupt(interruptPin));
                        waktuMulai = millis();
                        systemState = SYS_IDLE;
                        butuhRetryCepat = true;
                        break;
                  }
            } break;

            case SYS_SENDING: {
                  prosesKirimData();
            } break;
      }

      if (systemState != SYS_CONNECTING && systemState != SYS_SAMPLING) updateTampilan();
}