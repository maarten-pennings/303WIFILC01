// led.cpp - control the LED on the 303WIFILC01 board

#include <Arduino.h>
#include "led.h"

#define LED_PIN 2

void led_init() {
  led_off(); 
  pinMode(LED_PIN, OUTPUT);
  Serial.printf("led : init\n");
}

void led_set(int on) {
  digitalWrite(LED_PIN, !on ); // low active
}

int led_get() {
  return !digitalRead(LED_PIN); // low active
}

void led_on() {
  led_set(1);
}

void led_off() {
  led_set(0);
}
