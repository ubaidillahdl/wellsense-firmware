#include "Functions.h"
#include "Global.h"

void kirimAT(const char* perintah) {
      while (sim800c.available()) sim800c.read();
      memset(buf, 0, sizeof(buf));
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
                        Serial.println(F(">>> Restart Modem..."));
                        logOled(F("Restart Modem..."));

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
                        Serial.println(F(">>> Menunggu SIM800C..."));
                        logOled(F("Menunggu SIM800C..."));
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
                        if (strstr(buf, "OK")) {
                              Serial.println(F(">>> SIM800C Terhubung"));
                              logOled(F("SIM800C Terhubung"));

                              sudahKirim = false;
                              connState = CON_CHECK_STATUS;
                        } else {
                              sudahKirim = false;
                              waktuKirim = millis();
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
                        if (strstr(buf, "IP GPRSACT")) {
                              logOled(F("IP Gprsact"));
                              connState = CON_CHECK_IP;
                        } else if (strstr(buf, "IP STATUS")) {
                              logOled(F("IP Status"));
                              connState = CON_CHECK_APN;
                        } else if (strstr(buf, "PDP DEACT")) {
                              logOled(F("Pdp Deact"));
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
                  }
                  if (millis() - waktuKirim > 3000) {
                        sudahKirim = false;
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
                        if (strstr(buf, "READY")) {
                              Serial.println(F(">>> SIM Sudah Siap"));
                              logOled(F("SIM Sudah Siap"));
                              retryReg = 0;
                              sudahKirim = false;
                              connState = CON_CHECK_SIGNAL;
                        } else {
                              retryReg++;
                              sudahKirim = false;

                              Serial.print(F(">>> ("));
                              Serial.print(retryReg);
                              Serial.println(F("SIM Belum Siap..."));
                              logOledRetry(F("SIM Belum Siap..."), retryReg);
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
                        char* p = strstr(buf, ": ");
                        if (p) {
                              uint8_t csq = atoi(p + 2);
                              if (csq >= 10 && csq != 99) {
                                    Serial.print(F(">>> Kekuatan Sinyal "));
                                    Serial.println(csq);

                                    logOledInt(F("Kekuatan Sinyal "), csq);

                                    sudahKirim = false;
                                    connState = CON_CHECK_REG;
                              } else {
                                    sudahKirim = false;
                                    Serial.print(F(">>> Sinyal Lemah "));
                                    Serial.println(csq);

                                    logOledInt(F("Sinyal Lemah"), csq);
                              }
                        } else {
                              Serial.println(F(">>> CSQ Gagal Dibaca"));
                              logOled(F("CSQ Gagal Dibaca"));
                              sudahKirim = false;
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
                        if (strstr(buf, ",1") || strstr(buf, ",5")) {
                              Serial.println(F(">>> Terdaftar di Jaringan"));
                              logOled(F("Terdaftar di Jaringan"));
                              retryReg = 0;
                              sudahKirim = false;
                              connState = CON_CHECK_GPRS;
                        } else {
                              retryReg++;
                              sudahKirim = false;

                              Serial.print(F(">>> ("));
                              Serial.print(retryReg);
                              if (strstr(buf, ",0")) {
                                    Serial.println(F(") Tidak Terdaftar..."));
                                    logOledRetry(F("Tidak Terdaftar..."), retryReg);
                              } else if (strstr(buf, ",2")) {
                                    Serial.println(F(") Mencari Jaringan..."));
                                    logOledRetry(F("Mencari Jaringan..."), retryReg);
                              } else if (strstr(buf, ",3")) {
                                    Serial.println(F(") Registrasi Ditolak..."));
                                    logOledRetry(F("Registrasi Ditolak..."), retryReg);
                              } else {
                                    Serial.println(F(") Jaringan Tidak Dikenal..."));
                                    logOledRetry(F("Jaringan Tidak Dikenal..."), retryReg);
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
                        if (strstr(buf, ": 1")) {
                              Serial.println(F(">>> Terhubung ke Jaringan"));
                              logOled(F("Terhubung ke Jaringan"));
                              retryReg = 0;
                              sudahKirim = false;
                              connState = CON_CHECK_APN;
                        } else {
                              retryReg++;
                              sudahKirim = false;

                              Serial.print(F(">>> ("));
                              Serial.print(retryReg);
                              Serial.println(F(") Menunggu Jaringan..."));

                              logOledRetry(F("Menunggu Jaringan..."), retryReg);
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
                        if (strstr(buf, "internet")) {
                              Serial.println(F(">>> APN Sudah Diset"));
                              logOled(F("APN Sudah Diset"));
                              sudahKirim = false;
                              connState = CON_CHECK_IP;
                        } else {
                              sudahKirim = false;
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
                        if (strstr(buf, "OK")) {
                              Serial.println(F(">>> SUKSES Set APN"));
                              logOled(F("SUKSES Set APN"));
                              sudahKirim = false;
                              connState = CON_ACTIVATE;
                        } else {
                              sudahKirim = false;
                              Serial.println(F(">>> Mencoba Set APN..."));
                              logOled(F("Mencoba Set APN..."));
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
                        if (strstr(buf, "OK")) {
                              retryReg = 0;
                              sudahKirim = false;
                              connState = CON_CHECK_IP;
                        } else {
                              retryReg++;
                              sudahKirim = false;
                              Serial.print(F(">>> ("));
                              Serial.print(retryReg);
                              Serial.println(F(") Menghubungkan ke Internet..."));

                              logOledRetry(F("Menghubungkan ke Internet..."), retryReg);

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
                        if (!strstr(buf, "ERROR") && strstr(buf, ".")) {
                              char* p = buf;
                              while (*p && !(*p >= '0' && *p <= '9')) p++;
                              char* end = strpbrk(p, "\r\n");
                              if (end) *end = '\0';
                              Serial.print(F(">>> IP: "));
                              Serial.println(p);

                              oled.print(F("IP: "));
                              oled.println(p);

                              retryReg = 0;
                              sudahKirim = false;
                              connState = CON_DONE;
                        } else {
                              retryReg++;
                              sudahKirim = false;

                              Serial.print(F(">>> ("));
                              Serial.print(retryReg);
                              Serial.println(F(") Gagal Mendapatkan IP..."));

                              logOledRetry(F("Gagal Mendapatkan IP..."), retryReg);

                              if (retryReg >= 5) {
                                    retryReg = 0;
                                    connState = CON_RESTART_MODEM;
                              }
                        }
                  }
            } break;

            case CON_DONE: {
                  Serial.println(F(">>> Terhubung ke Internet"));
                  logOled(F("Terhubung ke Internet"));

                  systemState = SYS_IDLE;
            } break;
      }
}