#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <MAX30105.h>
#include <SoftwareSerial.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define I2C_ADDRESS 0x3C
#define RX_SIM 4
#define TX_SIM 5
#define INTERRUPT 3
#define VIBRATOR 7
#define BUTTON 6

#define PANJANG_BUFFER 150
#define SERVER_IP "125.164.81.29"
#define SERVER_PORT "5005"
#define DEVICE_TOKEN "WS-866501012348821"

// System
enum SystemState {
      SYS_CONNECTING,
      SYS_IDLE,
      SYS_SAMPLING,
      SYS_SENDING,
};
enum HalamanState {
      HAL_VITALS,
      HAL_JAM,
};
extern SystemState systemState;
extern HalamanState halamanState;

// Display
extern SSD1306AsciiWire oled;
extern String statusTeks;
extern uint8_t progressLevel;
extern bool first,
    dataUpdate;

// Network
extern SoftwareSerial sim800c;
enum ConnState {
      CON_RESTART_MODEM,
      CON_WAIT_READY,
      CON_WAIT_ATE0,
      CON_CHECK_STATUS,
      CON_CIPSHUT,
      CON_CHECK_SIM,
      CON_CHECK_SIGNAL,
      CON_CHECK_REG,
      CON_CHECK_GPRS,
      CON_CHECK_APN,
      CON_SET_APN,
      CON_ACTIVATE,
      CON_CHECK_IP,
      CON_DONE,
};
enum SendState {
      SEND_TCP_OPEN,
      SEND_HEARTBEAT,
      SEND_WAIT_PONG,
      SEND_REQUEST,
      SEND_TRANSMIT,
      SEND_WAIT_REPLY,
      SEND_CLOSE,
      SEND_DONE,
};
extern ConnState connState;
extern SendState sendState;
extern uint32_t waktuKirim,
    waktuPrint;
extern uint8_t bufIdx,
    retryReg;
extern uint8_t jamHH,
    jamMM,
    jamSS;
extern bool sudahKirim;
extern bool sudahRender;
extern char buf[64];

// Sensor
extern MAX30105 particleSensor;
struct LpfState {
      int32_t x1 = 0;
      int32_t y1 = 0;
};
struct DesimasiState {
      int32_t s1Sum, s2Sum, s3Sum;
      uint8_t s1Count, s2Count, s3Count;
};
struct DataSensor {
      int16_t bufferIR[PANJANG_BUFFER];
      int16_t bufferRed[PANJANG_BUFFER];
};
struct HasilVitals {
      uint8_t hr;
      uint8_t spo2;
      uint8_t sbp;
      uint8_t dbp;
      uint8_t hb;
      uint16_t std;
};
extern LpfState filterRed,
    filterIR;
extern DesimasiState desimRed,
    desimIR;
extern DataSensor wadah;
extern HasilVitals dataVitals;
extern int32_t rawRed,
    filteredRed,
    rawIR,
    filteredIR;
extern uint8_t bufferIdx;
extern bool dataReady;

// main
extern uint32_t waktuMulai;
extern uint8_t sisaDetik;
extern bool butuhRetryCepat;

#endif