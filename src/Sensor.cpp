#include "Functions.h"
#include "Global.h"

void initSensorMAX() {
      if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
            Serial.println(F("Sensor tidak ditemukan"));
            while (1)
                  ;  // Berhenti jika sensor gagal init
      }

      // Config: LED 255 (Full), 400Hz Sample, Pulse Width 411, Range 16384
      particleSensor.setup(255, 1, 2, 400, 411, 16384);
      particleSensor.enableDATARDY();
      particleSensor.getINT1();  // Clear interrupt flag
      attachInterrupt(digitalPinToInterrupt(INTERRUPT), handleInterrupt, FALLING);

      particleSensor.setPulseAmplitudeRed(0);   // Red mati saat standby
      particleSensor.setPulseAmplitudeIR(255);  // IR tetap nyala untuk deteksi tangan
}

void handleInterrupt() {
      dataReady = true;
}

bool adaTangan() {
      long currentIR = particleSensor.getIR();  // Baca IR instan
      static unsigned long lastUpdate = 0;

      // Threshold deteksi tangan (berdasarkan hasil tuning)
      if (currentIR > 150000 && currentIR < 240000) {
            return true;
      } else {
            // Notifikasi ke Serial tiap 500ms
            if (millis() - lastUpdate > 500) {
                  Serial.println(F(">>> Pasang Sensor ke Pergelangan !"));
                  lastUpdate = millis();
            }
            return false;
      }
}

void bangunSesi() {
      // Tampilkan status di Serial (VIP atau Normal)
      Serial.println(butuhRetryCepat ? F("\n>>> Pengukuran ulang...") : F("\n>>> Pengukuran baru..."));

      particleSensor.setPulseAmplitudeRed(255);
      particleSensor.clearFIFO();  // --- PROSES FLUSH (Pembersihan Sisa Data) ---
      particleSensor.getINT1();    // Clear flag interrupt pada sensor agar tidak ada trigger palsu di awal

      // Pasang kembali interupsi untuk mulai mendeteksi data baru
      attachInterrupt(digitalPinToInterrupt(INTERRUPT), handleInterrupt, FALLING);
}

void prosesStandby() {
      static unsigned long lastFlash = 0;  // Simpan waktu terakhir kedip
      unsigned long currentMillis = millis();

      // Atur interval kedip LED setiap 500ms (0.5 detik)
      if (currentMillis - lastFlash > 500) {
            lastFlash = currentMillis;

            static bool ledNyala = false;
            ledNyala = !ledNyala;  // Tukar status ON/OFF

            if (ledNyala) {
                  particleSensor.setPulseAmplitudeRed(20);  // Nyalakan LED Red redup (power 20) sebagai indikator standby
            } else {
                  particleSensor.setPulseAmplitudeRed(0);  // Matikan LED Red
            }
      }
}

void prosesSampling() {
      dataReady = false;
      particleSensor.check();

      // while (particleSensor.available()) {
      if (particleSensor.available()) {
            rawRed = particleSensor.getFIFORed();
            rawIR = particleSensor.getFIFOIR();

            // LPF Digital (Integer math): Meredam noise frekuensi tinggi
            filteredRed = (8815 * filterRed.y1 + 592 * rawRed + 592 * filterRed.x1) / 10000;
            filteredIR = (8815 * filterIR.y1 + 592 * rawIR + 592 * filterIR.x1) / 10000;

            // Simpan history filter
            filterRed.x1 = rawRed;
            filterRed.y1 = filteredRed;
            filterIR.x1 = rawIR;
            filterIR.y1 = filteredIR;

            // Turunkan rate data ke 50Hz
            int16_t outRed, outIR;
            updateDesimasi(desimIR, filteredIR, outIR);

            if (updateDesimasi(desimRed, filteredRed, outRed)) {
                  if (bufferIdx < PANJANG_BUFFER) {
                        wadah.bufferRed[bufferIdx] = outRed;
                        wadah.bufferIR[bufferIdx] = outIR;
                        bufferIdx++;
                  }
            }
            particleSensor.nextSample();
      }
      particleSensor.getINT1();  // Clear interrupt
}

bool updateDesimasi(DesimasiState &d, int32_t inputVal, int16_t &outputVal) {
      d.s1Sum += inputVal;
      d.s1Count++;

      if (d.s1Count >= 2) {  // Stage 1: 400Hz -> 200Hz
            d.s2Sum += d.s1Sum / 2;
            d.s2Count++;
            d.s1Sum = 0;
            d.s1Count = 0;

            if (d.s2Count >= 2) {  // Stage 2: 200Hz -> 100Hz
                  d.s3Sum += d.s2Sum / 2;
                  d.s3Count++;
                  d.s2Sum = 0;
                  d.s2Count = 0;

                  if (d.s3Count >= 2) {  // Stage 3: 100Hz -> 50Hz
                        outputVal = (int16_t)(d.s3Sum / 2);
                        d.s3Sum = 0;
                        d.s3Count = 0;
                        return true;  // Data 50Hz siap
                  }
            }
      }
      return false;
}

void normalisasiBuffer() {
      long totalRed = 0;
      long totalIR = 0;

      // 1. Hitung Rata-rata (DC Offset)
      for (int i = 0; i < PANJANG_BUFFER; i++) {
            totalRed += wadah.bufferRed[i];
            totalIR += wadah.bufferIR[i];
      }

      int32_t avgRed = totalRed / PANJANG_BUFFER;
      int32_t avgIR = totalIR / PANJANG_BUFFER;

      // 2. Terapkan Normalisasi (AC + Offset 500)
      for (int i = 0; i < PANJANG_BUFFER; i++) {
            // Kurangi dengan rata-rata agar nilai berpusat di 0, lalu geser ke 500
            wadah.bufferRed[i] = (wadah.bufferRed[i] - avgRed) + 500;
            wadah.bufferIR[i] = (wadah.bufferIR[i] - avgIR) + 500;

            // Safety Guard: Pastikan tidak negatif dan hemat karakter (max 3-4 digit)
            if (wadah.bufferRed[i] < 0) wadah.bufferRed[i] = 0;
            if (wadah.bufferIR[i] < 0) wadah.bufferIR[i] = 0;
      }
}