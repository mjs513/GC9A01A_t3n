#include "config.h"
#include "Horizon_Background.h"
#include "Horizon_GroundSky.h"
#include "Maquette_Avion.h"
#include "HeadingIndicator_Aircraft.h"
#include "HeadingWeel.h"

void setup() {
  init();

  /* Initialise the sensor */
  if (!bno.begin()) {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    //while(1);
  }

  delay(1000);

  /* Use external crystal for better accuracy */
  bno.setExtCrystalUse(true);

  /* Display some basic information on this sensor */
  displaySensorDetails();

  translateY = 0;
  //translateY = 20;  // 5 degrees = 20 pixels, nose up
//  Pause();
  //bmpDisp(Horizon_GroundSky_image, Horizon_GroundSky_width, Horizon_GroundSky_height, true);
//  Pause();
  //Rotate_and_Draw_Bitmap(Maquette_Avion_image, Maquette_Avion_width, Maquette_Avion_height, 45.0f, 48, 25, 0, 0);
 // Pause();
  //bmpDisp(Horizon_Background_image, Horizon_Background_width, Horizon_Background_height, true);
//  Pause();
 // tft.updateScreen();

  //Pause();
  //bmpDisp(HeadingWeel_image, HeadingWeel_width, HeadingWeel_height, true);
  tft.fillScreen(BLACK);
  Rotate_and_Draw_Bitmap(HeadingWeel_image, HeadingWeel_width, HeadingWeel_height, 45.0f, 119, 119, 0, 10);
  bmpDisp(HeadingIndicator_Aircraft_image, HeadingIndicator_Aircraft_width, HeadingIndicator_Aircraft_height, true);

  tft.updateScreen();
  Pause();

  Serial.printf("End Setup");

  }

int loop_count = 0;
uint32_t sum_elapsed = 0;

void loop() {
  tft.fillScreen(LIGHTGREY);
  elapsedMillis em = 0;
  sensors_event_t event;
  bno.getEvent(&event);

  /* The processing sketch expects data as roll, pitch, heading */
  #if 0
  Serial.print(F("Orientation: "));
  Serial.print((float)event.orientation.x);
  Serial.print(F(" "));
  Serial.print((float)event.orientation.y);
  Serial.print(F(" "));
  Serial.print((float)event.orientation.z);
  Serial.println(F(""));
#endif

  float roll = (float)event.orientation.z;
  float pitch = (float)event.orientation.y;

  translateY = pitch * pixs2deg;  // 5 degrees = 20 pixels, nose up
  //imageFile = SD.open("Horizon_GroundSky.bmp", FILE_READ);
  //name = "Horizon_GroundSky.bmp";
  //bmpDraw(imageFile, name, true);
  //bmpDisp(Horizon_GroundSky_image, Horizon_GroundSky_width, Horizon_GroundSky_height, true);
  elapsedMicros emDraw;
  Rotate_and_Draw_Bitmap(Horizon_GroundSky_image, Horizon_GroundSky_width, Horizon_GroundSky_height,  roll, 100, 288, 0, translateY);
  Serial.printf("Rotate_and_Draw_Bitmap(Horizon_GroundSky_image... ): %u\n", (uint32_t)emDraw);
//  Pause();

  translateY = 0;

  //imageFile = SD.open("Maquette_Avion.bmp", FILE_READ);
  //name = "Maquette_Avion.bmp";
  //bmpDraw(imageFile, name, true);
  emDraw = 0;
  bmpDisp(Maquette_Avion_image, Maquette_Avion_width, Maquette_Avion_height, true);
  Serial.printf("bmpDisp(Maquette_Avion_image, ... ): %u\n", (uint32_t)emDraw);
//  Pause();
  //Rotate_and_Draw_Bitmap(Maquette_Avion_image, Maquette_Avion_width, Maquette_Avion_height, (float)event.orientation.z, 0, 0);


  //bmpDraw(imageFile, name, true);
  emDraw = 0;
  bmpDisp(Horizon_Background_image, Horizon_Background_width, Horizon_Background_height, true);
  Serial.printf("bmpDisp(Horizon_Background_image, Horizon_Background_width... ): %u\n", (uint32_t)emDraw);
//  Pause();
  tft.updateScreen();

  uint32_t em_time = em;
  sum_elapsed += em_time;
  if (++loop_count == 10) {
    Serial.printf("Loop time: %u\n", sum_elapsed);
    loop_count = 0;
    sum_elapsed = 0;
  }  

  //MTP.loop();

  if (em_time < BNO055_SAMPLERATE_DELAY_MS)
    delay(BNO055_SAMPLERATE_DELAY_MS - em_time);
}

void Pause() {
  while (Serial.read() != -1);
  Serial.println("Press any Key to continue");
  while (Serial.read() == -1);
  while (Serial.read() != -1);
}