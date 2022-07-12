// Simple example to embed pictures in your sketch
// and draw on the ILI9341 display with writeRect()
//
// By Frank Bösing
//
// https://forum.pjrc.com/threads/32601-SPI-Library-Issue-w-ILI9341-TFT-amp-PN532-NFC-Module-on-Teensy-3-2?p=94534&viewfull=1#post94534

#include "SPI.h"
#include <GC9A01A_t3n.h>

// Converted to code with:
// http://www.rinkydinkelectronics.com/t_imageconverter565.php
//
#include "picture.c" //the picture
// *************** Change to your Pin numbers ***************
#define TFT_DC  9
#define TFT_CS 10
#define TFT_RST 8
#define TFT_SCK 13
//#define TFT_MISO 12
#define TFT_MOSI 11

GC9A01A_t3n tft = GC9A01A_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK);

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  tft.writeRect(32, 33, 256, 174, (uint16_t*)picture);
}

void loop(void) {
}
