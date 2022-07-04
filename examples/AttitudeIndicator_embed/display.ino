#if defined(__MK20DX256__)
#define IMAGE_BUFFER_ROWS 40
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define IMAGE_BUFFER_ROWS 120
#else
#define IMAGE_BUFFER_ROWS 240
#endif

DMAMEM uint16_t g_image_scratch_buffer[IMAGE_BUFFER_ROWS*SCREEN_WIDTH];
uint32_t g_image_scratch_buffer_size = IMAGE_BUFFER_ROWS*SCREEN_WIDTH;

void init() {

  while (!Serial && millis() < 5000)
    ;  // wait for Arduino Serial Monitor
  Serial.begin(9600);
  if (CrashReport) Serial.print(CrashReport);
  delay(500);

  #ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWriteFast(TFT_BL, HIGH);
  #endif  
  tft.begin();

  g_jpg_scale_x_above[0] = (tft.width() * 3) / 2;
  g_jpg_scale_x_above[1] = tft.width() * 3;
  g_jpg_scale_x_above[2] = tft.width() * 6;
  g_jpg_scale_x_above[3] = tft.width() * 12;

  g_jpg_scale_y_above[0] = (tft.height() * 3) / 2;
  g_jpg_scale_y_above[1] = tft.height() * 3;
  g_jpg_scale_y_above[2] = tft.height() * 6;
  g_jpg_scale_y_above[3] = tft.height() * 12;

  Serial.println("After TFT Begin");
  tft.fillScreen(RED);
  delay(250);
  tft.fillScreen(GREEN);
  delay(250);
  tft.fillScreen(BLUE);
  delay(250);
  tft.fillScreen(BLACK);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  //tft.println("Waiting for Arduino Serial Monitor...........");
  #if !defined(__MK20DX256__)
  tft.useFrameBuffer(1);
  #endif
#ifdef _ILI9341_t3NH_
  tft.setClipRect(0, 40, 240, 240);
#endif
  g_BMPScale = 1;
  g_center_image = 1;

#if 0
  g_image_scratch_buffer_size = tft.width() * IMAGE_BUFFER_ROWS;
  if (g_image_scratch_buffer_size > (Maquette_Avion_width * Maquette_Avion_height))
    g_image_scratch_buffer_size = (Maquette_Avion_width * Maquette_Avion_height);
  g_image_scratch_buffer = (uint16_t *)malloc(g_image_scratch_buffer_size * sizeof(uint16_t));

  if (!g_image_scratch_buffer) {
    Serial.println("Error could not allocate buffer to display ismages");
    while (1)
      ;
  }
  #endif
}

//=============================================================================
// BMP support
//=============================================================================
// This method takes our image and centers it calls off to our write
// clipped rect.

#ifndef USE_RLE_IMAGE
void bmpDisp(const uint16_t *storage, int image_width, int image_height, bool fErase) {
  g_image_offset_x = (tft.width() - image_width) / 2;
  g_image_offset_y = (tft.height() - image_height) / 2;
  if (g_debug_BMP) Serial.printf("\tImage Offsets (%d, %d)\n", g_image_offset_x, g_image_offset_y);
  writeClippedRect(0, 0, image_width, image_height, storage);
}
#else
// RLE version.
// note so far we are ignoring the RLE_BUFFER_ROWS as the writeClipped works at pixel level...
void bmpDisp(const uint16_t *storage, int image_width, int image_height, bool fErase) {
  int row, col;
  g_image_offset_x = (tft.width() - image_width) / 2;
  g_image_offset_y = (tft.height() - image_height) / 2;
  if (g_debug_BMP) Serial.printf("\tImage Offsets (%d, %d)\n", g_image_offset_x, g_image_offset_y);

  uint16_t rle_len = 0;
  uint16_t rle_color;

  for (row = 0; row < image_height; row++) {  // For each scanline...
    uint16_t *usT = g_image_scratch_buffer;
    for (col = 0; col < image_width; col++) {  // For each pixel...
      if (rle_len == 0) {
        rle_len = *storage++;
        rle_color = *storage++;
      }
      *usT++ = rle_color;
      rle_len--;
    }  // end pixel
    writeClippedRect(0, row, image_width, 1, g_image_scratch_buffer);
  }  // end scanline
}
#endif

//===========================================================================================

inline void UpdateColorSums(uint16_t color, uint16_t &r_sum, uint16_t &g_sum, uint16_t &b_sum, uint8_t &sum_count) {
  uint8_t r, g, b;
  tft.color565toRGB(color, r, g, b);
  r_sum += r;
  g_sum += g;
  b_sum += b;
  sum_count++;
}

void Rotate_and_Draw_Bitmap(const uint16_t *storage, int image_width, int image_height, float angle, int pivotX, int pivotY, int translateX, int translateY) {
  static int first_calls = 3;

  g_image_offset_x = (tft.width() - image_width) / 2;

  // this function will do it's own offsetting of the y as to know what bands we need. 
  //g_image_offset_y = (tft.height() - image_height) / 2;
  g_image_offset_y = 0;

  if (g_debug_BMP) Serial.printf("\tImage Offsets (%d, %d)\n", g_image_offset_x, g_image_offset_y);

  //int xc = (int)((float)image_width / 2.0 + 0.5);   // Calculate the (rotation) center of the image (x fraction)
  //int yc = (int)((float)image_height / 2.0 + 0.5);  // Calculate the (rotation) center of the image (y fraction)

  //Serial.printf("xc: %d, yc: %d\n", xc, yc);

  angle = angle * 0.01745329;  //convert angle to radians

  float sin_angle = sin(angle);  // Pre-calculate the time consuming sinus
  float cos_angle = cos(angle);  // Pre-calculate the time consuming cosinus

  int rows, cols;  //x = rows, y = cols

  int image_offset_y = (tft.height() - image_height) / 2;
  int band_starting_row;
  int row_limit;
  if (image_height >= tft.height()) {
    band_starting_row = 0;
    row_limit = tft.height();
  } else {
    band_starting_row = image_offset_y;
    row_limit = image_offset_y + image_height;
  }
  // compute how many rows we can do per pass, by how big our buffer is.
  int rows_per_band = g_image_scratch_buffer_size / image_width;
  int count_buffer_used = rows_per_band * image_width;


  while (band_starting_row < row_limit) {
    int band_ending_row = band_starting_row + rows_per_band;
    if (band_ending_row > row_limit) {
      band_ending_row = row_limit;
      rows_per_band = row_limit - band_starting_row;
    }

    // clear the array note on last band clear more than we may need...
    // Note we will mark this item as not set... 
    for (int i = 0; i < count_buffer_used; i++) g_image_scratch_buffer[i] = BMP_PIXEL_NOT_SET;

    const uint16_t *image_in = storage;
    uint16_t image_in_color;
    if (first_calls) Serial.printf("%d - %d (%d), %p\n", band_starting_row, band_ending_row, rows_per_band);
#ifdef USE_RLE_IMAGE
    uint16_t rle_len = 0;
#endif


    for (rows = 0; rows < image_height; rows++) {  // i goes through all the Bytes of the image
      for (cols = 0; cols < image_width; cols++) {

#ifdef USE_RLE_IMAGE
        if (rle_len == 0) {
          rle_len = *image_in++;
          image_in_color = *image_in++;
        }
        rle_len--;
#else
        image_in_color = *image_in++;
#endif
        // no use doing the math if the write clipped, will ignore it anyway...
        // Experiment to map these also as to know which are supposed to be
        // transparant and which ones were not mapped to...
        //if (image_in_color != BMP_TRANSPARENT) {
          int col_new = ((cols - pivotX + translateX) * cos_angle - (rows - pivotY + translateY) * sin_angle) + pivotX;
          int row_new = ((cols - pivotX + translateX) * sin_angle + (rows - pivotY + translateY) * cos_angle) + pivotY;

          // Check if the rotated pixel is within the image and now in our band range
          if ((col_new >= 0) && (col_new < image_width) && (row_new >= 0) && (row_new < image_height)) {
            // now make sure in band. need to offset
            row_new += image_offset_y;

            if ((row_new >= band_starting_row) && (row_new < band_ending_row)) {
              uint32_t new_index = ((row_new - band_starting_row) * image_width) + col_new;
              g_image_scratch_buffer[new_index] = image_in_color;
            }
          }
        //}
      }
    }
    // Experiment now look for those which are marked as BMP_PIXEL_NOT_SET, let us try to fill them in.
    uint16_t * ppixel = g_image_scratch_buffer;
    for (rows = 0; rows < rows_per_band; rows++) {  // i goes through all the Bytes of the image
      for (cols = 0; cols < image_width; cols++) {
        if (*ppixel == BMP_PIXEL_NOT_SET) {
          // lazy if on edge of band kiss
          if ((rows == 0) || (rows == (rows_per_band - 1)) || (cols == 0) || (cols == (image_width - 1))) *ppixel = BMP_TRANSPARENT;
          else {
            uint16_t r_sum=0;
            uint16_t g_sum = 0;
            uint16_t b_sum = 0;
            uint8_t rgb_count = 0;
            uint8_t trans_count = 0;
            uint16_t color = *(ppixel -1);  // one to left;
            if (color == BMP_TRANSPARENT) trans_count++;
            else UpdateColorSums(color, r_sum, g_sum, b_sum, rgb_count);
            
            color = *(ppixel - image_width);  // one to above;
            if (color == BMP_TRANSPARENT) trans_count++;
            else UpdateColorSums(color, r_sum, g_sum, b_sum, rgb_count);

            color = *(ppixel +1);  // one to right;
            if (color == BMP_TRANSPARENT) trans_count++;
            else if (color != BMP_PIXEL_NOT_SET) UpdateColorSums(color, r_sum, g_sum, b_sum, rgb_count);

            color = *(ppixel + image_width);  // one to below;
            if (color == BMP_TRANSPARENT) trans_count++;
            else if (color != BMP_PIXEL_NOT_SET) UpdateColorSums(color, r_sum, g_sum, b_sum, rgb_count);

            if (rgb_count > trans_count) *ppixel = Color565(r_sum/rgb_count, g_sum/ rgb_count, b_sum / rgb_count);
            else *ppixel = BMP_TRANSPARENT;
          }
        }
        ppixel++;
      }
    }

    

    writeClippedRect(0, band_starting_row, image_width, rows_per_band, g_image_scratch_buffer);  // Draw the rotated image
    band_starting_row = band_ending_row;                                                         // setup for the next band
  }
  if (first_calls) first_calls--;
}

inline void writeClippedLine(int x, int y, int cx, const uint16_t *pixels) {
  uint16_t ixcolor;
  const uint16_t *ppcolor = nullptr;
  for (uint16_t ix = 0; ix < cx; ix++) {
    if (*pixels != BMP_TRANSPARENT) {
      if (!ppcolor) {
        ppcolor = pixels;
        ixcolor = ix;
      }
    } else if (ppcolor) {
      tft.writeRect(x+ ixcolor, y , ix - ixcolor, 1, ppcolor);
      ppcolor = nullptr;
    }
    pixels++;
    //uint16_t pixel_color = *pixels++;
    //if (pixel_color != BMP_TRANSPARENT) tft.drawPixel(ix + x, iy + y, pixel_color);
  }
  if (ppcolor) {
    tft.writeRect(x + ixcolor, y , cx - ixcolor, 1, ppcolor);
  }

}

void writeClippedRect(int x, int y, int cx, int cy, const uint16_t *pixels) {
  x += g_image_offset_x;
  y += g_image_offset_y;
  ;
  int end_x = x + cx;  //cx = witdth
  int end_y = y + cy;  //cy = 1

  if ((x >= 0) && (y >= 0) && (end_x <= tft.width()) && (end_y <= tft.height())) {
    //tft.writeRect(x, y, cx, cy, pixels);
    // the whole image is drawn.
    for (uint16_t iy = 0; iy < cy; iy++) {
      writeClippedLine(x, y + iy, cx, pixels);
      pixels += cx; // update to next line
    }
    if (g_debug_output) Serial.printf("\t1:(%d, %d, %d, %d) %p\n", x, y, cx, cy, pixels);
    //WaitforWRComplete();
  } else {
    int width = cx;
    if (end_x > tft.width()) width -= (end_x - tft.width());
    if (x < 0) width += x;
    const uint16_t *ppixLine = pixels;
    for (int yt = y; yt < end_y; yt++) {
      if (yt >= tft.height()) break;
      if (yt >= 0) {
        if (x >= 0) {
          //tft.writeRect(x, yt, width, 1, ppixLine);
          writeClippedLine(x, yt, width, ppixLine);
          if (g_debug_output) Serial.printf("\t2:(%d, %d, %d, %d) %p\n", x, yt, width, 1, ppixLine);
        } else {
          //tft.writeRect(0, yt, width, 1, ppixLine - x);
          writeClippedLine(0, yt, width, ppixLine - x);
          if (g_debug_output) Serial.printf("\t3:(%d, %d, %d, %d ++) %p\n", 0, yt, width, 1, ppixLine - x);
        }
        //WaitforWRComplete();
      }
      ppixLine += cx;
    }
  }
}