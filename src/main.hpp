#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <espnow.h>

#define BOARD2//slave
// #define BOARD1//programmer

#ifdef BOARD2 //programmer bc:ff:4d:f8:04:f8 COM3 white
  #define BOARD {0xb4, 0xe6, 0x2d, 0x37, 0x16, 0xbf}
#endif
#ifdef BOARD1 //slave bc:ff:4d:f9:d0:23 COM5 black
  #define BOARD {0xBC, 0xFF, 0x4d, 0xf9, 0xee, 0x54}
#endif



void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len);

void boardInitialisation();

#include <GyverOLED.h>

#ifdef BOARD2

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

void screenInitialize();
#endif

#include <GyverButton.h>

#define CLOUD_PIN 14

GButton cloudButton(CLOUD_PIN);
void buttonInitialize();

#endif