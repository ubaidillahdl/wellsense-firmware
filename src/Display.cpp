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