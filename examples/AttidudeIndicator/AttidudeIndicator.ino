#include "config.h"

//Image storage size for
int horizon_ground_sky_width = 200;
int horizon_ground_sky_height = 576;
uint16_t horizon_ground_sky_buffer[200 * 576];
uint16_t *horizon_ground_sky_image;

int maquette_avion_width = 98;
int maquette_avion_height = 49;
uint16_t maquette_avion_buffer[98 * 49];
uint16_t *maquette_avion_image;


int       horizon_background_width;
int       horizon_background_height;
uint16_t *horizon_background_image;

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
  imageFile = SD.open("Horizon_GroundSky.bmp", FILE_READ);
  name = "Horizon_GroundSky.bmp";

  //bmpDraw(imageFile, name, true);
  horizon_ground_sky_image = bmpImport(imageFile, name, horizon_ground_sky_width, horizon_ground_sky_height, horizon_ground_sky_buffer);
  Serial.printf("Horizon_GroundSky.bmp %p %d %d\n", horizon_ground_sky_image, horizon_ground_sky_width, horizon_ground_sky_height);
  Serial.flush();
  bmpDisp(horizon_ground_sky_image, horizon_ground_sky_width, horizon_ground_sky_height, true);

  imageFile = SD.open("Maquette_Avion.bmp", FILE_READ);
  name = "Maquette_Avion.bmp";
  //bmpDraw(imageFile, name, true);
  maquette_avion_image = bmpImport(imageFile, name, maquette_avion_width, maquette_avion_height, maquette_avion_buffer);
  Serial.printf("Maquette_Avion.bmp %p %d %d\n", maquette_avion_image, maquette_avion_width, maquette_avion_height);
  Serial.flush();
  //bmpDisp(image2[0], maquette_avion_width, maquette_avion_height, true);
  Rotate_and_Draw_Bitmap(maquette_avion_image, maquette_avion_width, maquette_avion_height, 45.0f);


  // try horizon load in memory
  imageFile = SD.open("Horizon_Background.bmp", FILE_READ);
  name = "Horizon_Background.bmp";
  horizon_background_image = bmpImport(imageFile, name, horizon_background_width, horizon_background_height, nullptr);
  Serial.printf("Horizon_Background.bmp %p %d %d\n", horizon_background_image, horizon_background_width, horizon_background_height);
  Serial.flush();

  //imageFile = SD.open("Horizon_Background.bmp", FILE_READ);
  //bmpDraw(imageFile, name, true);
  bmpDisp(horizon_background_image, horizon_background_width, horizon_background_height, true);
  Serial.printf("After Horizon_Background.bmp");
  Serial.flush();
  tft.updateScreen();

  Serial.printf("End Setup");
  Serial.flush();
  }

int loop_count = 0;
uint32_t sum_elapsed = 0;

void loop() {
  /* Get a new sensor event */

#if 1
  elapsedMillis em = 0;
  sensors_event_t event;
  bno.getEvent(&event);

  /* The processing sketch expects data as roll, pitch, heading */
  Serial.print(F("Orientation: "));
  Serial.print((float)event.orientation.x);
  Serial.print(F(" "));
  Serial.print((float)event.orientation.y);
  Serial.print(F(" "));
  Serial.print((float)event.orientation.z);
  Serial.println(F(""));

  translateY = (float)event.orientation.y * pixs2deg;  // 5 degrees = 20 pixels, nose up
  //imageFile = SD.open("Horizon_GroundSky.bmp", FILE_READ);
  //name = "Horizon_GroundSky.bmp";
  //bmpDraw(imageFile, name, true);
  bmpDisp(horizon_ground_sky_image, horizon_ground_sky_width, horizon_ground_sky_height, true);

  translateY = 0;

  //imageFile = SD.open("Maquette_Avion.bmp", FILE_READ);
  //name = "Maquette_Avion.bmp";
  //bmpDraw(imageFile, name, true);
  //bmpDisp(image2[0], maquette_avion_width, maquette_avion_height, true);
  Rotate_and_Draw_Bitmap(maquette_avion_image, maquette_avion_width, maquette_avion_height, (float)event.orientation.z);


#if 0
  imageFile = SD.open("Horizon_Background.bmp", FILE_READ);
  name = "Horizon_Background.bmp";
  bmpDraw(imageFile, name, true);
#else
  //bmpDraw(imageFile, name, true);
  bmpDisp(horizon_background_image, horizon_background_width, horizon_background_height, true);
#endif
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
#endif
}