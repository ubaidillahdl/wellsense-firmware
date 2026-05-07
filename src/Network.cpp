#include "Functions.h"
#include "Global.h"

void kirimAT(const char* perintah) {
      while (sim800c.available()) sim800c.read();
      memset(buf, 0, sizeof(buf));
      buf[0] = '\0';
      sim800c.println(perintah);
}

void bacaAT() {
      uint8_t len = strlen(buf);
      while (sim800c.available()) {
            char c = sim800c.read();
            if (len < sizeof(buf) - 1) {
                  buf[len++] = c;
                  buf[len] = '\0';
            }
      }
}

void logOled(const __FlashStringHelper* pesan) {
      oled.println(pesan);
}

void logOledInt(const __FlashStringHelper* pesan, uint8_t angka) {
      oled.print(pesan);
      oled.println(angka);
}

void logOledRetry(const __FlashStringHelper* pesan, uint8_t angka) {
      oled.print(F("("));
      oled.print(angka);
      oled.print(F(") "));
      oled.println(pesan);
}

void prosesConnecting() {
      bacaAT();

      switch (connState) {
            case CON_RESTART_MODEM: {
                  if (!sudahKirim) {
                        Serial.println(F("(-) Modem RST"));
                        logOled(F("(-) MODEM RST"));

                        kirimAT("AT+CFUN=1,1");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        connState = CON_WAIT_READY;
                  }
            } break;

            case CON_WAIT_READY: {
                  if (millis() - waktuPrint >= 500) {
                        waktuPrint = millis();
                        Serial.println(F("(-) Menunggu Modem"));
                        logOled(F("(-) MENUNGGU MODEM"));
                  }
                  if (strstr(buf, "RDY")) {
                        sudahKirim = false;
                        waktuKirim = millis();
                        connState = CON_WAIT_ATE0;
                  } else if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();
                        connState = CON_WAIT_ATE0;
                  }
            } break;

            case CON_WAIT_ATE0: {
                  if (!sudahKirim) {
                        kirimAT("ATE0");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 3000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "OK")) {
                              Serial.println(F("(+) Modem Siap"));
                              logOled(F("(+) MODEM SIAP"));
                              connState = CON_CHECK_STATUS;
                        } else {
                              connState = CON_WAIT_READY;
                        }
                  }
            } break;

            case CON_CHECK_STATUS: {
                  if (!sudahKirim) {
                        kirimAT("AT+CIPSTATUS");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 3000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "IP GPRSACT")) {
                              Serial.println(F("(+) GPRS Aktif"));
                              logOled(F("(+) GPRS AKTIF"));
                              connState = CON_CHECK_IP;
                        } else if (strstr(buf, "IP STATUS")) {
                              Serial.println(F("(-) IP Aktif"));
                              logOled(F("(-) IP AKTIF"));
                              connState = CON_CHECK_APN;
                        } else if (strstr(buf, "PDP DEACT")) {
                              Serial.println(F("(!) GPRS Putus"));
                              logOled(F("(!) GPRS PUTUS"));
                              connState = CON_CIPSHUT;
                        } else {
                              connState = CON_CHECK_SIM;
                        }
                  }
            } break;

            case CON_CIPSHUT: {
                  if (!sudahKirim) {
                        kirimAT("AT+CIPSHUT");
                        sudahKirim = true;
                        waktuKirim = millis();

                        Serial.println(F("(!) GPRS Nonaktif"));
                        logOled(F("(!) GPRS NONAKTIF"));
                  }
                  if (millis() - waktuKirim > 3000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        connState = CON_CHECK_SIM;
                  }
            } break;

            case CON_CHECK_SIM: {
                  if (!sudahKirim) {
                        kirimAT("AT+CPIN?");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 2000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "READY")) {
                              Serial.println(F("(+) SIM Siap"));
                              logOled(F("(+) SIM SIAP"));
                              retryReg = 0;
                              connState = CON_CHECK_SIGNAL;
                        } else {
                              retryReg++;

                              Serial.print(F("("));
                              Serial.print(retryReg);
                              Serial.println(F(") SIM Tidak Siap"));
                              logOledRetry(F("SIM TIDAK SIAP"), retryReg);
                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_CHECK_SIGNAL: {
                  if (!sudahKirim) {
                        kirimAT("AT+CSQ");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 2000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        char* p = strstr(buf, ": ");
                        if (p) {
                              uint8_t csq = atoi(p + 2);
                              if (csq >= 10 && csq != 99) {
                                    Serial.print(F("(+) Sinyal "));
                                    Serial.println(csq);
                                    logOledInt(F("(+) SINYAL "), csq);

                                    retryReg = 0;
                                    connState = CON_CHECK_REG;
                              } else {
                                    retryReg++;

                                    Serial.print(F("("));
                                    Serial.print(retryReg);
                                    Serial.print(F(") Sinyal "));
                                    Serial.println(csq);

                                    oled.print(F("("));
                                    oled.print(retryReg);
                                    oled.print(F(") SINYAL "));
                                    oled.println(csq);

                                    if (retryReg >= 5) {
                                          retryReg = 0;
                                          connState = CON_RESTART_MODEM;
                                    }
                              }
                        } else {
                              retryReg++;
                              sudahKirim = false;

                              Serial.print(F("("));
                              Serial.print(retryReg);
                              Serial.println(F(") Sinyal Error"));

                              logOledRetry(F("SINYAL ERROR"), retryReg);

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_CHECK_REG: {
                  if (!sudahKirim) {
                        kirimAT("AT+CREG?");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 2000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, ",1") || strstr(buf, ",5")) {
                              Serial.println(F("(+) Jaringan OK"));
                              logOled(F("(+) JARINGAN OK"));
                              retryReg = 0;
                              connState = CON_CHECK_GPRS;
                        } else {
                              retryReg++;

                              Serial.print(F("("));
                              Serial.print(retryReg);
                              if (strstr(buf, ",0")) {
                                    Serial.println(F(") Belum REG"));
                                    logOledRetry(F("BELUM REG"), retryReg);
                              } else if (strstr(buf, ",2")) {
                                    Serial.println(F(") Cari Jaringan"));
                                    logOledRetry(F("CARI JARINGAN"), retryReg);
                              } else if (strstr(buf, ",3")) {
                                    Serial.println(F(") REG Ditolak"));
                                    logOledRetry(F("REG DITOLAK"), retryReg);
                              } else {
                                    Serial.println(F(") REG Unknown"));
                                    logOledRetry(F("REG UNKNOWN"), retryReg);
                              }

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_CHECK_GPRS: {
                  if (!sudahKirim) {
                        kirimAT("AT+CGATT?");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 2000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, ": 1")) {
                              Serial.println(F("(+) Data Aktif"));
                              logOled(F("(+) DATA AKTIF"));
                              retryReg = 0;
                              connState = CON_CHECK_APN;
                        } else {
                              retryReg++;

                              Serial.print(F("("));
                              Serial.print(retryReg);
                              Serial.println(F(") Cek Data"));

                              logOledRetry(F("CEK DATA"), retryReg);
                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_CHECK_APN: {
                  if (!sudahKirim) {
                        kirimAT("AT+CSTT?");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 2000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "internet")) {
                              Serial.println(F("(+) APN Siap"));
                              logOled(F("(+) APN SIAP"));
                              connState = CON_CHECK_IP;
                        } else {
                              connState = CON_SET_APN;
                        }
                  }
            } break;

            case CON_SET_APN: {
                  if (!sudahKirim) {
                        kirimAT("AT+CSTT=\"internet\"");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 2000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "OK")) {
                              Serial.println(F("(+) APN SET"));
                              logOled(F("(+) APN SET"));

                              retryReg = 0;
                              connState = CON_ACTIVATE;
                        } else {
                              retryReg++;

                              Serial.print(F("("));
                              Serial.print(retryReg);
                              Serial.println(F(") SET APN"));

                              logOledRetry(F("SET APN"), retryReg);

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_ACTIVATE: {
                  if (!sudahKirim) {
                        kirimAT("AT+CIICR");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "OK")) {
                              retryReg = 0;
                              connState = CON_CHECK_IP;
                        } else {
                              retryReg++;

                              Serial.print(F("("));
                              Serial.print(retryReg);
                              Serial.println(F(") Aktivasi Data"));

                              logOledRetry(F("AKTIVASI DATA"), retryReg);

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_CHECK_IP: {
                  if (!sudahKirim) {
                        kirimAT("AT+CIFSR");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 3000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (!strstr(buf, "ERROR") && strstr(buf, ".")) {
                              char* p = buf;
                              while (*p && !(*p >= '0' && *p <= '9')) p++;
                              char* end = strpbrk(p, "\r\n");
                              if (end) *end = '\0';
                              Serial.print(F("(+) IP "));
                              Serial.println(p);

                              oled.print(F("(+) IP "));
                              oled.println(p);

                              retryReg = 0;
                              connState = CON_DONE;
                        } else {
                              retryReg++;

                              Serial.print(F(" ("));
                              Serial.print(retryReg);
                              Serial.println(F(") IP Gagal"));

                              logOledRetry(F("IP GAGAL"), retryReg);

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_DONE: {
                  Serial.println(F("(+) Internet Siap"));
                  logOled(F("(+) INTERNET SIAP"));

                  oled.setScrollMode(SCROLL_MODE_OFF);
                  oled.clear();
                  systemState = SYS_IDLE;
            } break;
      }
}

void prosesKirimData() {
      bacaAT();

      switch (sendState) {
            case SEND_TCP_OPEN: {
                  if (!sudahKirim) {
                        while (sim800c.available()) sim800c.read();
                        memset(buf, 0, sizeof(buf));
                        bufIdx = 0;

                        sim800c.print(F("AT+CIPSTART=\"TCP\",\""));
                        sim800c.print(SERVER_IP);
                        sim800c.print(F("\","));
                        sim800c.println(SERVER_PORT);

                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if ((!strstr(buf, "FAIL") && !strstr(buf, "ERROR")) || strstr(buf, "ALREADY CONNECT")) {
                              statusTeks = "TCP OK";
                              progressLevel = 2;
                              retryReg = 0;
                              sendState = SEND_REQUEST;
                        } else {
                              retryReg++;
                              Serial.print(F(" ("));
                              Serial.print(retryReg);
                              Serial.println(F(") TCP Gagal"));

                              statusTeks = "TCP ERR ";
                              statusTeks += retryReg;
                              progressLevel = 1;

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    resetKeConnecting();
                              }
                        }
                  }
            } break;

            case SEND_HEARTBEAT: {
                  if (!sudahKirim) {
                        while (sim800c.available()) sim800c.read();
                        memset(buf, 0, sizeof(buf));
                        bufIdx = 0;

                        sim800c.println(F("AT+CIPSEND=5"));
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, ">")) {
                              sim800c.println(F("PING"));
                              memset(buf, 0, sizeof(buf));
                              bufIdx = 0;

                              statusTeks = "PING RDY";
                              retryReg = 0;
                              sendState = SEND_WAIT_PONG;
                        } else {
                              retryReg++;

                              statusTeks = "PING ERR ";
                              statusTeks += retryReg;

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    resetKeConnecting();
                              }
                        }
                  }
            } break;

            case SEND_WAIT_PONG: {
                  if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, "PONG")) {
                              Serial.println(F("(+) Server Dibuka"));
                              statusTeks = "PING OK";
                              progressLevel = 3;

                              retryReg = 0;
                              sendState = SEND_REQUEST;
                        } else {
                              retryReg++;
                              Serial.println(F("(x) Server Tidak Respon"));

                              statusTeks = "PING FAIL ";
                              statusTeks += retryReg;
                              progressLevel = 2;

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    resetKeConnecting();
                              }
                        }
                  }
            } break;

            case SEND_REQUEST: {
                  if (!sudahKirim) {
                        long totalChar = strlen(DEVICE_TOKEN) + 1;
                        for (uint8_t i = 0; i < PANJANG_BUFFER; i++) {
                              char tmp[12];
                              totalChar += sprintf(tmp, "%u:%u,", wadah.bufferIR[i], wadah.bufferRed[i]);
                        }
                        totalChar += 1;

                        while (sim800c.available()) sim800c.read();
                        memset(buf, 0, sizeof(buf));
                        bufIdx = 0;

                        sim800c.print(F("AT+CIPSEND="));
                        sim800c.println(totalChar);

                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 5000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        if (strstr(buf, ">")) {
                              Serial.println(F("(+) Siap Kirim"));
                              statusTeks = "READY TX";

                              retryReg = 0;
                              sendState = SEND_TRANSMIT;
                        } else {
                              retryReg++;
                              Serial.println(F("(x) Request Gagal"));

                              statusTeks = "TX FAIL ";
                              statusTeks += retryReg;

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    sendState = SEND_CLOSE;
                              }
                        }
                  }
            } break;

            case SEND_TRANSMIT: {
                  if (!sudahKirim) {
                        while (sim800c.available()) sim800c.read();
                        memset(buf, 0, sizeof(buf));
                        bufIdx = 0;

                        sim800c.print(DEVICE_TOKEN);
                        sim800c.print(F("|"));
                        for (uint8_t i = 0; i < PANJANG_BUFFER; i++) {
                              sim800c.print(wadah.bufferIR[i]);
                              sim800c.print(F(":"));
                              sim800c.print(wadah.bufferRed[i]);
                              sim800c.print(F(","));
                        }
                        sim800c.print(F("\n"));

                        Serial.println(F("(+) Data Terkirim"));
                        statusTeks = "SENT OK";
                        progressLevel = 3;

                        sudahKirim = true;
                        waktuKirim = millis();
                        sendState = SEND_WAIT_REPLY;
                  }
            } break;

            case SEND_WAIT_REPLY: {
                  char* star = strchr(buf, '*');
                  if (star) {
                        char* end = strchr(star, '#');
                        if (end) {
                              *end = '\0';
                              if (pecahDataFeedback(star + 1)) {
                                    statusTeks = "FINISHED";
                                    progressLevel = 4;
                                    dataUpdate = true;
                                    butuhRetryCepat = false;
                              } else {
                                    statusTeks = "UNSTEADY";
                                    progressLevel = 3;
                              }
                              sendState = SEND_CLOSE;
                        }
                  } else if (millis() - waktuKirim > 15000) {
                        sudahKirim = false;
                        waktuKirim = millis();

                        Serial.println(F("(x) Timeout Server"));
                        statusTeks = "TIMEOUT";

                        sendState = SEND_CLOSE;
                  }
            } break;

            case SEND_CLOSE: {
                  if (!sudahKirim) {
                        kirimAT("AT+CIPCLOSE");
                        sudahKirim = true;
                        waktuKirim = millis();
                  }
                  if (millis() - waktuKirim > 1000) {
                        sudahKirim = false;
                        waktuMulai = millis();
                        sendState = SEND_DONE;
                  }
            } break;

            case SEND_DONE: {
                  if (millis() - waktuMulai > 1500) {
                        statusTeks = "CLOSE";
                        progressLevel = 5;

                        if (millis() - waktuMulai > 3000) {
                              waktuMulai = millis();
                              systemState = SYS_IDLE;
                              sendState = SEND_TCP_OPEN;
                        }
                  }
            } break;
      }
}

bool pecahDataFeedback(char* buf) {
      bool hasilAnalisa = true;

      char* ptr = strtok(buf, ";");
      if (ptr) dataVitals.hr = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) dataVitals.spo2 = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) dataVitals.sbp = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) dataVitals.dbp = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) dataVitals.hb = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) dataVitals.std = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) jamHH = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) jamMM = atoi(ptr);

      ptr = strtok(NULL, ";");
      if (ptr) jamSS = atoi(ptr);

      if (dataVitals.hr > 0) {
            // Print hasil ke Serial untuk verifikasi
            Serial.println(F("\n======= HASIL ANALISA ======="));
            Serial.print(F("HR\t: "));
            Serial.print(dataVitals.hr);
            Serial.println(F("\tbpm"));
            Serial.print(F("SPO2\t: "));
            Serial.print(dataVitals.spo2);
            Serial.println(F("\t%"));
            Serial.print(F("SBP\t: "));
            Serial.print(dataVitals.sbp);
            Serial.println(F("\tmmHg"));
            Serial.print(F("DBP\t: "));
            Serial.print(dataVitals.dbp);
            Serial.println(F("\tmmHg"));
            Serial.print(F("HB\t: "));
            Serial.print(dataVitals.hb);
            Serial.println(F("\tg/L"));
            Serial.print(F("STD\t: "));
            Serial.print(dataVitals.std);
            Serial.println(F("\tcounts"));
            Serial.println(F("============================="));
      } else {
            Serial.print(F("(!) STD "));
            Serial.print(dataVitals.std);  // Tampilkan tanpa desimal agar bersih
            Serial.println(F(" counts, sinyal tidak stabil !"));

            hasilAnalisa = false;
      }
      return hasilAnalisa;
}

void resetKeConnecting() {
      oled.clear();
      oled.setFont(System5x7);
      oled.setScrollMode(SCROLL_MODE_AUTO);

      sudahKirim = false;
      first = true;
      waktuKirim = millis();

      progressLevel = 0;
      statusTeks = "";
      retryReg = 0;

      connState = CON_WAIT_READY;
      systemState = SYS_CONNECTING;
      sendState = SEND_TCP_OPEN;
}