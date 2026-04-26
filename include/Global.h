#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <MAX30105.h>
#include <SoftwareSerial.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
#define rxSim 4
#define txSim 5

enum SystemState {
      SYS_CONNECTING,
      SYS_IDLE,
      SYS_SAMPLING,
      SYS_SENDING,
};

enum ConnState {
      CON_RESTART_MODEM,
      CON_WAIT_READY,
      CON_WAIT_ATE0,
      CON_CHECK_STATUS,
      CON_CIPSHUT,
      CON_CHECK_SIGNAL,
      CON_CHECK_SIM,
      CON_CHECK_REG,
      CON_CHECK_GPRS,
      CON_CHECK_APN,
      CON_SET_APN,
      CON_ACTIVATE,
      CON_CHECK_IP,
      CON_DONE,
};

extern SystemState systemState;
extern ConnState connState;
extern SoftwareSerial sim800c;
extern SSD1306AsciiWire oled;

extern uint32_t waktuPrint;
extern uint32_t waktuKirim;
extern uint8_t retryReg;

extern char buf[64];
extern bool sudahKirim;

#endif