#include <Adafruit_GFX.h>

#include <SPI.h>
#include <GC9A01A_t3n.h>

#include "font_Arial.h"
#include "font_ArialBold.h"
#include "font_ComicSansMS.h"
#include "font_OpenSans.h"
#include "font_DroidSans.h"
#include "font_Michroma.h"
#include "font_Crystal.h"
#include "font_ChanceryItalic.h"

#define CS 10
#define DC 9
#define RST 8
GC9A01A_t3n tft = GC9A01A_t3n(CS, DC, RST);
uint8_t test_screen_rotation = 0;


void setup() {
  Serial.begin(38400);
  long unsigned debug_start = millis ();
  while (!Serial && ((millis () - debug_start) <= 5000)) ;
  Serial.println("Setup");
  tft.begin();

  tft.setRotation(4);
  tft.fillWindow(BLACK);

  tft.setTextColor(WHITE);
  tft.setFont(Arial_14);
  tft.println("Arial_14");
  displayStuff();

  tft.setTextColor(YELLOW);
  tft.setFont(Arial_14_Bold);
  tft.println("ArialBold 14");
  displayStuff();

  tft.setTextColor(GREEN);
  tft.setFont(ComicSansMS_14);
  tft.println("ComicSansMS 14");
  displayStuff(); 

  nextPage();
  
  tft.setTextColor(WHITE);
  tft.setFont(DroidSans_14);
  tft.println("DroidSans_14");
  displayStuff();

  tft.setTextColor(YELLOW);
  tft.setFont(Michroma_14);
  tft.println("Michroma_14");
  displayStuff();

  tft.setTextColor(BLACK, YELLOW);
  tft.setFont(Crystal_24_Italic);
  tft.println("CRYSTAL_24");
  displayStuff();

  nextPage();

  tft.setTextColor(GREEN);
  tft.setFont(Chancery_24_Italic);
  tft.println("Chancery_24_Italic");
  displayStuff();

  //anti-alias font OpenSans
  tft.setTextColor(RED, YELLOW);
  tft.setFont(OpenSans24);
  tft.println("OpenSans 18");
  displayStuff(); 
  
  Serial.println("Basic Font Display Complete");
  Serial.println("Loop test for alt colors + font");
}

void loop()
{
  Serial.printf("\nRotation: %d\n", test_screen_rotation);
  tft.setRotation(test_screen_rotation);
  test_screen_rotation = (test_screen_rotation + 1) & 0x3;
  
  nextPage();

  tft.setTextColor(WHITE);
  tft.setFont(Arial_14);
  tft.println("Arial_14");
  displayStuff1();

  tft.setTextColor(YELLOW);
  tft.setFont(Arial_14_Bold);
  tft.println("ArialBold 14");
  displayStuff1();

  nextPage();

  tft.setTextColor(GREEN);
  tft.setFont(ComicSansMS_14);
  tft.println("ComicSansMS 14");
  displayStuff1(); 

  tft.setTextColor(WHITE);
  tft.setFont(DroidSans_14);
  tft.println("DroidSans_14");
  displayStuff1();

  nextPage();

  tft.setTextColor(YELLOW);
  tft.setFont(Michroma_14);
  tft.println("Michroma_14");
  displayStuff1();

  nextPage();
  
  tft.setTextColor(BLACK, YELLOW);
  tft.setFont(Crystal_24_Italic);
  tft.println("CRYSTAL_24");
  displayStuff1();

  tft.setTextColor(GREEN);
  tft.setFont(Chancery_24_Italic);
  tft.println("Chancery_24_Italic");
  displayStuff1();
  
  nextPage();

  //anti-alias font OpenSans
  tft.setTextColor(RED, YELLOW);
  tft.setFont(OpenSans24);
  tft.println("OpenSans 18");
  displayStuff1(); 

  nextPage();
}

uint32_t displayStuff()
{
  elapsedMillis elapsed_time = 0;
  tft.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  tft.println("abcdefghijklmnopqrstuvwxyz");
  tft.println("0123456789");
  tft.println("!@#$%^ &*()-");
  tft.println(); tft.println();
  return (uint32_t) elapsed_time;
}

uint32_t displayStuff1()
{
  elapsedMillis elapsed_time = 0;
  tft.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  tft.println("abcdefghijklmnopqrstuvwxyz");
  tft.println("0123456789");
  tft.println("!@#$%^ &*()-");
  static const char alternating_text[] = "AbCdEfGhIjKlMnOpQrStUvWxYz\raBcDeFgHiJkLmNoPqRsTuVwXyZ";
 
  for (uint8_t i = 0; i < sizeof(alternating_text); i++) {
    if (i & 1) tft.setTextColor(WHITE, RED);
    else tft.setTextColor(YELLOW, BLUE);
    tft.write(alternating_text[i]);
  }
  tft.println(); tft.println();
  return (uint32_t) elapsed_time;
}

void nextPage()
{
  Serial.println("Press anykey to continue");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;

  tft.fillWindow(BLACK);
  tft.setCursor(0, 0);
}
