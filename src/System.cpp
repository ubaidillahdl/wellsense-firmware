#include "Functions.h"
#include "Global.h"

void initSistem() {
      Serial.begin(115200);
      sim800c.begin(9600);

      pinMode(VIBRATOR, OUTPUT);
      pinMode(BUTTON, INPUT);
      vibrate();
}

void vibrate() {
      digitalWrite(VIBRATOR, HIGH);
      delay(80);
      digitalWrite(VIBRATOR, LOW);
      delay(100);
      digitalWrite(VIBRATOR, HIGH);
      delay(150);
      digitalWrite(VIBRATOR, LOW);
}

void vibrateHal() {
      digitalWrite(VIBRATOR, HIGH);
      delay(30);
      digitalWrite(VIBRATOR, LOW);
}

void bacaTombol() {
      static bool statSebelum = HIGH;
      static uint32_t waktuDebounce = 0;
      static bool sudahProses = false;

      bool statSekarang = digitalRead(BUTTON);

      if (statSekarang != statSebelum) {
            waktuDebounce = millis();
            statSebelum = statSekarang;
            sudahProses = false;

            if (statSekarang == LOW) vibrateHal();
      }

      if (!sudahProses && millis() - waktuDebounce > 50 && statSekarang == LOW) {
            switch (halamanState) {
                  case HAL_VITALS:
                        halamanState = HAL_JAM;
                        break;
                  case HAL_JAM:
                        first = true;
                        halamanState = HAL_VITALS;
                        break;
            }
            sudahProses = true;
            sudahRender = false;
            oled.clear();
      }
}