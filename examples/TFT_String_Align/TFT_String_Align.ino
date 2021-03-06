/*
Tests string alignment

Normally strings are printed relative to the top left corner but this can be
changed with the setTextDatum() function. The library has #defines for:

TL_DATUM = Top left
TC_DATUM = Top centre
TR_DATUM = Top right
ML_DATUM = Middle left
MC_DATUM = Middle centre
MR_DATUM = Middle right
BL_DATUM = Bottom left
BC_DATUM = Bottom centre
BR_DATUM = Bottom right
*/

#include <GC9A01A_t3n.h> // Hardware-specific library
#include <SPI.h>
#include <GC9A01A_t3n_font_Arial.h>

#define RST 23
#define DC 9
#define CS 10
GC9A01A_t3n tft = GC9A01A_t3n(CS, DC, RST);

unsigned long drawTime = 0;

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);
  tft.setFont(Arial_18);
  //tft.setTextSize(4);
}

void loop() {

  tft.fillScreen(BLACK);
  
  for(byte datum = 0; datum < 9; datum++) {
    tft.setTextColor(WHITE, BLACK);
    
    tft.setTextDatum(datum);
    
    tft.drawNumber(88,160,60);
    tft.fillCircle(160,120,5,RED);
    
    tft.setTextDatum(MC_DATUM);
    
    tft.setTextColor(YELLOW);
    tft.drawString("TEENSY 4",160,120);
    delay(1000);
    tft.fillScreen(BLACK);
  }

  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(BLACK);
  tft.drawString("X",160,120);
  delay(1000);
  tft.fillScreen(BLACK);
  
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(BLACK);
  tft.drawString("X",160,120);
  delay(1000);
  tft.fillScreen(BLACK);

  tft.setTextColor(WHITE, BLUE);

  tft.setTextDatum(MC_DATUM);

  //Test floating point drawing function
  float test = 67.125;
  tft.drawFloat(test, 4, 160, 180);
  delay(1000);
  tft.fillScreen(BLACK);
  test = -0.555555;
  tft.drawFloat(test, 3, 160, 180);
  delay(1000);
  tft.fillScreen(BLACK);
  test = 0.1;
  tft.drawFloat(test, 4, 160, 180);
  delay(1000);
  tft.fillScreen(BLACK);
  test = 9999999;
  tft.drawFloat(test, 1, 160, 180);
  delay(1000);
  
  tft.fillCircle(160,180,5,YELLOW);
  
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(BLACK);
  tft.drawString("X",160,180);

  delay(4000);
}
