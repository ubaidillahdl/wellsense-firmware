#include "Functions.h"
#include "Global.h"

void initSistem() {
      Serial.begin(115200);
      sim800c.begin(9600);
}