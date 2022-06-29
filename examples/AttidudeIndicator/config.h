#include "SPI.h"

#include "GC9A01A_t3n.h"
//#include <ILI9341_t3n.h>

// *************** Change to your Pin numbers ***************
//#define USE_KURTE_MMOD2

#ifdef USE_KURTE_MMOD2
#define TFT_DC 9
#define TFT_CS 32
#define TFT_RST 31

#else
#define TFT_DC  9
#define TFT_CS 10
#define TFT_RST 8
#endif

#ifdef _ILI9341_t3NH_
#define BLACK ILI9341_BLACK
#define RED ILI9341_RED
#define YELLOW ILI9341_YELLOW
#define WHITE ILI9341_WHITE
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);

#else
GC9A01A_t3n tft = GC9A01A_t3n(TFT_CS, TFT_DC, TFT_RST);
#endif

//=====================================================
#include <SD.h>
//#include <MTP_Teensy.h>

#define BMP_TRANSPARENT  0xFFE0 //yellow

int g_BMPScale = 1;
int g_center_image = 1;
int g_background_color = BLACK;

// scale boundaries {2, 4, 8, 16<maybe>}
enum {SCL_HALF=0, SCL_QUARTER, SCL_EIGHTH, SCL_16TH};

int g_jpg_scale_x_above[4];
int g_jpg_scale_y_above[4];

// variables used in some of the display output functions
int g_image_offset_x = 0;
int g_image_offset_y = 0;
uint8_t g_image_scale = 1;
uint8_t g_debug_output = 0;
uint8_t g_debug_BMP = 0;

//============================================
#define SD_CS BUILTIN_SDCARD
File imageFile;
const char *name = nullptr;

int pixs2deg = (int) (20/5);  //pixels per degree

//=============================================

int translateY = 0;

//==============================================


inline uint16_t Color565(uint8_t r,uint8_t g,uint8_t b) {return tft.color565(r, g, b);}
inline void   Color565ToRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {tft.color565toRGB(color, r, g, b);}

//==============================================

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.

   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3-5V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);