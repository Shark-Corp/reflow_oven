#include "general.hpp"
#include "lcd_helper.hpp"

void setup() {
  pinMode(DEBUG_LED_PIN, OUTPUT);
  Serial.begin(115200);


  lcd_init();

}

void loop() {
    digitalWrite(DEBUG_LED_PIN, HIGH);  //LED PIN SET HIGH
    delay(1000);            // 1 SEC DELAY
    digitalWrite(DEBUG_LED_PIN, LOW);   //LED PIN SET LOW
    delay(1000);
}

