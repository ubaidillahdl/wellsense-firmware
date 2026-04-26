#include <Arduino.h>
#include <Wire.h>

#include "Functions.h"
#include "Global.h"

SoftwareSerial sim800c(rxSim, txSim);
SSD1306AsciiWire oled;
SystemState systemState = SYS_CONNECTING;
ConnState connState = CON_WAIT_READY;

char buf[64];
bool sudahKirim = false;

uint32_t waktuKirim = millis();
uint32_t waktuPrint = 0;
uint8_t retryReg = 0;

void setup() {
      initSistem();
      initDisp();
}

void loop() {
      switch (systemState) {
            case SYS_CONNECTING:
                  prosesConnecting();
                  break;
            case SYS_IDLE:
                  break;
            case SYS_SAMPLING:
                  break;
            case SYS_SENDING:
                  break;
      }
}