#include "Functions.h"
#include "Global.h"

void initDisp() {
      Wire.begin();
      Wire.setClock(100000L);
      oled.begin(&Adafruit128x64, I2C_ADDRESS);

      oled.setCursor(0, 3);
      oled.setFont(Cooper19);
      oled.print(F("Wellsense"));

      delay(3000);
      oled.clear();

      oled.setFont(System5x7);
      oled.setScrollMode(SCROLL_MODE_AUTO);
}

void updateTampilan() {
      if (first || dataUpdate) {
            tampilVitals();
            first = false;
            dataUpdate = false;
      }

      oled.setCursor(0, 0);
      oled.print(statusTeks);
      for (int i = 0; i < (10 - (int)statusTeks.length()); i++) oled.print(F(" "));

      oled.setCursor(0, 1);
      for (int i = 0; i <= 5; i++) oled.print((i <= progressLevel) ? F("o") : F("."));
}

void tampilVitals() {
      // 1. AMBIL DATA (Pastikan tipe data sesuai struct: uint8_t & uint16_t)
      uint8_t sbp = dataVitals.sbp;
      uint8_t dbp = dataVitals.dbp;
      uint8_t hr = dataVitals.hr;
      uint8_t spo2 = dataVitals.spo2;
      uint16_t hb = dataVitals.hb;  // hb dalam g/L (misal 145)

      // --- SISI KIRI (Tensi: SBP & DBP) ---
      // Tentukan posisi label (anchor) berdasarkan jumlah digit tertinggi
      uint8_t defaultX = (sbp >= 10 || dbp >= 10) ? 26 : 14;
      uint8_t anchorL = (sbp >= 100 || dbp >= 100) ? 38 : defaultX;

      for (uint8_t r = 3; r <= 7; r++) oled.clearField(0, r, 65);
      oled.setFont(lcdnums12x16);

      // Print SBP: Tambah spasi padding agar rata kanan terhadap label
      oled.setCursor(0, 3);
      if (anchorL == 38 && sbp < 10)
            oled.print(F("  "));  // Satuan di mode ratusan
      else if (anchorL == 38 && sbp < 100)
            oled.print(F(" "));  // Puluhan di mode ratusan
      else if (anchorL == 26 && sbp < 10)
            oled.print(F(" "));  // Satuan di mode puluhan
      oled.print(sbp);

      // Print DBP: Logika padding yang sama dengan SBP
      oled.setCursor(0, 6);
      if (anchorL == 38 && dbp < 10)
            oled.print(F("  "));
      else if (anchorL == 38 && dbp < 100)
            oled.print(F(" "));
      else if (anchorL == 26 && dbp < 10)
            oled.print(F(" "));
      oled.print(dbp);

      // Label Sisi Kiri (SBP/DBP)
      oled.setFont(font5x7);
      oled.setCursor(anchorL, 3);
      oled.print(F("SBP"));
      oled.setCursor(anchorL, 4);
      oled.print(F("mmHg"));
      oled.setCursor(anchorL, 6);
      oled.print(F("DBP"));
      oled.setCursor(anchorL, 7);
      oled.print(F("mmHg"));

      // --- SISI KANAN (HR, SPO2, HB) ---
      // Geser anchor ke kiri (67) jika ada data yang butuh 3 digit
      uint8_t anchorR = (hr >= 100 || spo2 >= 100 || hb >= 100) ? 67 : 79;

      oled.setFont(lcdnums12x16);

      // Print HR: Clear area angka (67-104) & cetak rata kanan
      oled.clearField(67, 0, 38);
      oled.clearField(67, 1, 38);
      oled.setCursor(anchorR, 0);
      if (hr < 100 && anchorR == 67) oled.print(F(" "));  // Gap untuk ratusan
      if (hr < 10) oled.print(F(" "));                    // Gap untuk puluhan
      oled.print(hr);

      // Print SPO2
      oled.clearField(67, 3, 38);
      oled.clearField(67, 4, 38);
      oled.setCursor(anchorR, 3);
      if (spo2 < 100 && anchorR == 67) oled.print(F(" "));
      if (spo2 < 10) oled.print(F(" "));
      oled.print(spo2);

      // Print HB (g/L)
      oled.clearField(67, 6, 38);
      oled.clearField(67, 7, 38);
      oled.setCursor(anchorR, 6);
      if (hb < 100 && anchorR == 67) oled.print(F(" "));
      if (hb < 10) oled.print(F(" "));
      oled.print(hb);

      // Label Sisi Kanan (Tetap di posisi 105)
      oled.setFont(font5x7);
      oled.setCursor(105, 0);
      oled.print(F("HR"));
      oled.setCursor(105, 1);
      oled.print(F("bpm"));
      oled.setCursor(105, 3);
      oled.print(F("SPO2"));
      oled.setCursor(105, 4);
      oled.print(F("%"));
      oled.setCursor(105, 6);
      oled.print(F("HB"));
      oled.setCursor(105, 7);
      oled.print(F("g/L"));
}