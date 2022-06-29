void init() {
  //MTP.begin();

  while (!Serial && millis() < 5000)
    ;  // wait for Arduino Serial Monitor
  Serial.begin(9600);
  if (CrashReport) Serial.print(CrashReport);
  delay(500);

  tft.begin();

  g_jpg_scale_x_above[0] = (tft.width() * 3) / 2;
  g_jpg_scale_x_above[1] = tft.width() * 3;
  g_jpg_scale_x_above[2] = tft.width() * 6;
  g_jpg_scale_x_above[3] = tft.width() * 12;

  g_jpg_scale_y_above[0] = (tft.height() * 3) / 2;
  g_jpg_scale_y_above[1] = tft.height() * 3;
  g_jpg_scale_y_above[2] = tft.height() * 6;
  g_jpg_scale_y_above[3] = tft.height() * 12;

  Serial.print(F("Initializing SD card..."));
  tft.println(F("Init SD card..."));
  if (!SD.begin(SD_CS)) {
    tft.setTextSize(2);
    tft.fillScreen(RED);
    while (!SD.begin(SD_CS)) {
      //Serial.println(F("failed to access SD card!"));
      tft.printf("failed to access SD card on cs:%u!\n", SD_CS);
      delay(2000);
    }
  }
  //MTP.addFilesystem(SD, "SD Card");

  Serial.println("After TFT Begin");
  tft.fillScreen(BLACK);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  //tft.println("Waiting for Arduino Serial Monitor...........");
  tft.useFrameBuffer(1);
#ifdef _ILI9341_t3NH_
  tft.setClipRect(0, 40, 240, 240);
#endif
  g_BMPScale = 1;
  g_center_image = 1;
}

//=============================================================================
// BMP support
//=============================================================================
// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance for tiny AVR chips.

#define BUFFPIXEL 80
void bmpDraw(File &bmpFile, const char *filename, bool fErase) {

  //  File     bmpFile;
  int image_width, image_height;        // W+H in pixels
  uint8_t bmpDepth;                     // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;              // Start of image data in file
  uint32_t rowSize;                     // Not always = image_width; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];      // pixel buffer (R+G+B per pixel)
  uint16_t buffidx = sizeof(sdbuffer);  // Current position in sdbuffer
  boolean goodBmp = false;              // Set to true on valid header parse
  boolean flip = true;                  // BMP is stored bottom-to-top
  int row, col;
  uint8_t r, g, b;
  uint32_t pos = 0, startTime = millis();

  if (g_debug_BMP) {
    Serial.println();
    Serial.print(F("Loading image '"));
    Serial.print(filename);
    Serial.println('\'');
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) {  // BMP signature
    uint32_t bmp_size = read32(bmpFile);
    if (g_debug_BMP) {
      Serial.print(F("File size: "));
      Serial.println(bmp_size);
    }
    (void)read32(bmpFile);             // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);  // Start of image data
    if (g_debug_BMP) {
      Serial.print(F("Image Offset: "));
      Serial.println(bmpImageoffset, DEC);
    }
    // Read DIB header
    uint32_t dibHeader = read32(bmpFile);
    if (g_debug_BMP) {
      Serial.print(F("Header size: "));
      Serial.println(dibHeader);
    }
    image_width = read32(bmpFile);
    image_height = read32(bmpFile);
    if (read16(bmpFile) == 1) {    // # planes -- must be '1'
      bmpDepth = read16(bmpFile);  // bits per pixel
      if (g_debug_BMP) {
        Serial.print(F("Bit Depth: "));
        Serial.println(bmpDepth);
      }
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) {  // 0 = uncompressed
        goodBmp = true;                                  // Supported BMP format -- proceed!
        if (g_debug_BMP) {
          Serial.print(F("Image size: "));
          Serial.print(image_width);
          Serial.print('x');
          Serial.println(image_height);
        }

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (image_width * 3 + 3) & ~3;

        // If image_height is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (image_height < 0) {
          image_height = -image_height;
          flip = false;
        }

        g_image_scale = 1;
        if (g_BMPScale > 0) {
          g_image_scale = g_BMPScale;  // use what they passed in
        } else if (g_BMPScale < 0) {
          if (image_width > tft.width()) g_image_scale = (image_width + tft.width() - 1) / tft.width();
          if (image_height > tft.height()) {
            int yscale = (image_height + tft.height() - 1) / tft.height();
            if (yscale > g_image_scale) g_image_scale = yscale;
          }
        } else {
          if ((image_width > g_jpg_scale_x_above[SCL_16TH]) || (image_height > g_jpg_scale_y_above[SCL_16TH])) {
            g_image_scale = 16;
          } else if ((image_width > g_jpg_scale_x_above[SCL_EIGHTH]) || (image_height > g_jpg_scale_y_above[SCL_EIGHTH])) {
            g_image_scale = 8;
          } else if ((image_width > g_jpg_scale_x_above[SCL_QUARTER]) || (image_height > g_jpg_scale_y_above[SCL_QUARTER])) {
            g_image_scale = 4;
          } else if ((image_width > g_jpg_scale_x_above[SCL_HALF]) || (image_height > g_jpg_scale_y_above[SCL_HALF])) {
            g_image_scale = 2;
          }
        }
        if (g_debug_BMP) Serial.printf("Scale: 1/%d\n", g_image_scale);
        if (g_center_image) {
          g_image_offset_x = (tft.width() - (image_width / g_image_scale)) / 2;
          g_image_offset_y = (tft.height() - (image_height / g_image_scale)) / 2;
          Serial.printf("\tImage Offsets (%d, %d)\n", g_image_offset_x, g_image_offset_y);
        } else {
          g_image_offset_x = 0;
          g_image_offset_y = 0;
        }

        if (fErase && (((image_width / g_image_scale) < tft.width()) || ((image_height / g_image_scale) < image_height))) {
#if !defined(BMP_TRANSPARENT)
          tft.fillScreen((uint16_t)g_background_color);
#endif
        }

        // now we will allocate large buffer for SCALE*width
        uint16_t *usPixels = (uint16_t *)malloc(image_width * g_image_scale * sizeof(uint16_t));
        if (usPixels) {
          for (row = 0; row < image_height; row++) {  // For each scanline...

            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if (flip)  // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (image_height - 1 - row) * rowSize;
            else  // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) {  // Need seek?
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer);  // Force buffer reload
            }

            uint16_t *pusRow = usPixels + image_width * (row % g_image_scale);
            for (col = 0; col < image_width; col++) {  // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) {  // Indeed
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0;  // Set index to beginning
              }

              // Convert pixel from BMP to TFT format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              pusRow[col] = Color565(r, g, b);
            }  // end pixel
            if (g_image_scale == 1) {
              writeClippedRect(0, row, image_width, 1, pusRow);
              //for(uint16_t ix = 0; ix<image_width; ix++) tft.drawPixel(ix, row, pusRow[ix]);
            } else {
              ScaleWriteClippedRect(row, image_width, usPixels);
            }
          }                // end scanline
          free(usPixels);  // free it after we are done
          usPixels = nullptr;
        }  // malloc succeeded
        if (g_debug_BMP) {
          Serial.print(F("Loaded in "));
          Serial.print(millis() - startTime);
          Serial.println(" ms");
        }
      }  // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));
}

uint16_t *bmpImport(File &bmpFile, const char *filename, int &image_width, int &image_height, uint16_t *buffer) {
  //  File     bmpFile;
  uint16_t *image_buffer = nullptr;

  uint8_t bmpDepth;                     // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;              // Start of image data in file
  uint32_t rowSize;                     // Not always = image_width; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];      // pixel buffer (R+G+B per pixel)
  uint16_t buffidx = sizeof(sdbuffer);  // Current position in sdbuffer
  boolean goodBmp = false;              // Set to true on valid header parse
  boolean flip = true;                  // BMP is stored bottom-to-top
  int row, col;
  uint8_t r, g, b;
  uint32_t pos = 0, startTime = millis();

  if (g_debug_BMP) {
    Serial.println();
    Serial.print(F("Loading image '"));
    Serial.print(filename);
    Serial.println('\'');
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) {  // BMP signature
    uint32_t bmp_size = read32(bmpFile);
    if (g_debug_BMP) {
      Serial.print(F("File size: "));
      Serial.println(bmp_size);
    }
    (void)read32(bmpFile);             // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);  // Start of image data
    if (g_debug_BMP) {
      Serial.print(F("Image Offset: "));
      Serial.println(bmpImageoffset, DEC);
    }
    // Read DIB header
    uint32_t dibHeader = read32(bmpFile);
    if (g_debug_BMP) {
      Serial.print(F("Header size: "));
      Serial.println(dibHeader);
    }
    image_width = read32(bmpFile);
    image_height = read32(bmpFile);
    if (read16(bmpFile) == 1) {    // # planes -- must be '1'
      bmpDepth = read16(bmpFile);  // bits per pixel
      if (g_debug_BMP) {
        Serial.print(F("Bit Depth: "));
        Serial.println(bmpDepth);
      }
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) {  // 0 = uncompressed
        goodBmp = true;                                  // Supported BMP format -- proceed!
        if (g_debug_BMP) {
          Serial.print(F("Image size: "));
          Serial.print(image_width);
          Serial.print('x');
          Serial.println(image_height);
        }

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (image_width * 3 + 3) & ~3;

        // If image_height is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (image_height < 0) {
          image_height = -image_height;
          flip = false;
        }

        // now we will allocate large buffer for SCALE*width
        // quick and dirty... 
        if (buffer) image_buffer = buffer;
        else image_buffer = (uint16_t *)malloc(image_width * image_height * sizeof(uint16_t));
        
        if (image_buffer) {
          for (row = 0; row < image_height; row++) {  // For each scanline...

            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if (flip)  // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (image_height - 1 - row) * rowSize;
            else  // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) {  // Need seek?
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer);  // Force buffer reload
            }

            for (col = 0; col < image_width; col++) {  // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) {  // Indeed
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0;  // Set index to beginning
              }

              // Convert pixel from BMP to TFT format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              *(image_buffer + col + row * image_width) = Color565(r, g, b);
            }              // end pixel
          }                // end scanline
        }  // malloc succeeded
        if (g_debug_BMP) {
          Serial.print(F("Loaded in "));
          Serial.print(millis() - startTime);
          Serial.println(" ms");
        }
      }  // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));

  // return the image.
  return image_buffer;
}


void bmpDisp(uint16_t *storage, int image_width, int image_height, bool fErase) {
  int row, col;

  g_image_scale = 1;
  if (g_BMPScale > 0) {
    g_image_scale = g_BMPScale;  // use what they passed in
  } else if (g_BMPScale < 0) {
    if (image_width > tft.width()) g_image_scale = (image_width + tft.width() - 1) / tft.width();
    if (image_height > tft.height()) {
      int yscale = (image_height + tft.height() - 1) / tft.height();
      if (yscale > g_image_scale) g_image_scale = yscale;
    }
  } else {
    if ((image_width > g_jpg_scale_x_above[SCL_16TH]) || (image_height > g_jpg_scale_y_above[SCL_16TH])) {
      g_image_scale = 16;
    } else if ((image_width > g_jpg_scale_x_above[SCL_EIGHTH]) || (image_height > g_jpg_scale_y_above[SCL_EIGHTH])) {
      g_image_scale = 8;
    } else if ((image_width > g_jpg_scale_x_above[SCL_QUARTER]) || (image_height > g_jpg_scale_y_above[SCL_QUARTER])) {
      g_image_scale = 4;
    } else if ((image_width > g_jpg_scale_x_above[SCL_HALF]) || (image_height > g_jpg_scale_y_above[SCL_HALF])) {
      g_image_scale = 2;
    }
  }
  if (g_debug_BMP) Serial.printf("Scale: 1/%d\n", g_image_scale);
  if (g_center_image) {
    g_image_offset_x = (tft.width() - (image_width / g_image_scale)) / 2;
    g_image_offset_y = (tft.height() - (image_height / g_image_scale)) / 2;
    if (g_debug_BMP) Serial.printf("\tImage Offsets (%d, %d)\n", g_image_offset_x, g_image_offset_y);
  } else {
    g_image_offset_x = 0;
    g_image_offset_y = 0;
  }

  if (fErase && (((image_width / g_image_scale) < tft.width()) || ((image_height / g_image_scale) < image_height))) {
#if !defined(BMP_TRANSPARENT)
    tft.fillScreen((uint16_t)g_background_color);
#endif
  }

  // now we will allocate large buffer for SCALE*width
  uint16_t *usPixels = (uint16_t *)malloc(image_width * g_image_scale * sizeof(uint16_t));
  if (usPixels) {
    for (row = 0; row < image_height; row++) {  // For each scanline...
      uint16_t *pusRow = usPixels + image_width * (row % g_image_scale);
      for (col = 0; col < image_width; col++) {  // For each pixel...
        // Time to read more pixel data?
        //Serial.printf("row: %d, col: %d, addr: %d\n", row, col, col+row*image_width);
        pusRow[col] = *(storage + col + row * image_width);
      }  // end pixel
      if (g_image_scale == 1) {
        writeClippedRect(0, row, image_width, 1, pusRow);
        //for(uint16_t ix = 0; ix<image_width; ix++) tft.drawPixel(ix, row, pusRow[ix]);
      } else {
        ScaleWriteClippedRect(row, image_width, usPixels);
      }
    }                // end scanline
    free(usPixels);  // free it after we are done
    usPixels = nullptr;
  }  // malloc succeeded
}

void Rotate_and_Draw_Bitmap(uint16_t *storage, int width, int height, float angle) {

  int image_width = width;
  int image_height = height;

  int xc = (int)((float)image_width / 2.0 + 0.5);   // Calculate the (rotation) center of the image (x fraction)
  int yc = (int)((float)image_height / 2.0 + 0.5);  // Calculate the (rotation) center of the image (y fraction)

  //Serial.printf("xc: %d, yc: %d\n", xc, yc);

  angle = angle * 0.01745329;  //convert angle to radians

  float sin_angle = sin(angle);                       // Pre-calculate the time consuming sinus
  float cos_angle = cos(angle);                       // Pre-calculate the time consuming cosinus
  uint16_t rotated_image[image_height][image_width];  // Image array in RAM (will contain the rotated image)

  int rows, cols;  //x = rows, y = cols

  for (rows = 0; rows < image_height; rows++) {
    for (cols = 0; cols < image_width; cols++) {
      //Serial.printf("row: %d, col: %d, addr: %d\n", rows, cols, cols+rows*image_width);
      //rotated_image[rows][cols] = *(storage+cols+rows*image_width);
      rotated_image[rows][cols] = BMP_TRANSPARENT;
    }
  }  // Clear the array

#if 1
  for (rows = 0; rows < image_height; rows++) {  // i goes through all the Bytes of the image
    for (cols = 0; cols < image_width; cols++) {
      int col_new = ((cols - xc) * cos_angle - (rows - yc) * sin_angle) + xc;
      int row_new = ((cols - xc) * sin_angle + (rows - yc) * cos_angle) + yc;

      // Check if the rotated pixel is within the image
      if ((col_new >= 0 && col_new < image_width) && (row_new >= 0 && row_new < image_height)) {
        //Serial.printf("row: %d, col: %d, col_new: %d, row_new: %d\n", rows, cols, col_new, row_new);
        rotated_image[row_new][col_new] = *(storage + cols + rows * width);
      }
    }
  }
#endif
  bmpDisp(rotated_image[0], image_width, image_height, true);  // Draw the rotated image
}



// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read();  // LSB
  ((uint8_t *)&result)[1] = f.read();  // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read();  // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read();  // MSB
  return result;
}

void writeClippedRect(int x, int y, int cx, int cy, uint16_t *pixels) {
  x += g_image_offset_x;
  y += (g_image_offset_y + translateY);
  ;
  int end_x = x + cx;  //cx = witdth
  int end_y = y + cy;  //cy = 1

  if ((x >= 0) && (y >= 0) && (end_x <= tft.width()) && (end_y <= tft.height())) {
    //tft.writeRect(x, y, cx, cy, pixels);
    for (uint16_t ix = 0; ix < cx; ix++) {
#if defined(BMP_TRANSPARENT)
      if (pixels[ix] != BMP_TRANSPARENT) tft.drawPixel(ix + x, y, pixels[ix]);
#else
      tft.drawPixel(ix + x, y, pixels[ix]);
#endif
    }
    if (g_debug_output) Serial.printf("\t1:(%d, %d, %d, %d) %p\n", x, y, cx, cy, pixels);
    //WaitforWRComplete();
  } else {
    int width = cx;
    if (end_x > tft.width()) width -= (end_x - tft.width());
    if (x < 0) width += x;
    uint16_t *ppixLine = pixels;
    for (int yt = y; yt < end_y; yt++) {
      if (yt >= tft.height()) break;
      if (yt >= 0) {
        if (x >= 0) {
          //tft.writeRect(x, yt, width, 1, ppixLine);
          for (uint16_t ix = 0; ix < width; ix++) {
#if defined(BMP_TRANSPARENT)
            if (ppixLine[ix] != BMP_TRANSPARENT) tft.drawPixel(ix + x, yt, ppixLine[ix]);  //??not sure about this yet
#else
            tft.drawPixel(ix + x, yt, ppixLine[ix]);
#endif
          }
          if (g_debug_output) Serial.printf("\t2:(%d, %d, %d, %d) %p\n", x, yt, width, 1, ppixLine);
        } else {
          //tft.writeRect(0, yt, width, 1, ppixLine - x);
          for (uint16_t ix = 0; ix < width; ix++) {
#if defined(BMP_TRANSPARENT)
            if (ppixLine[ix - x] != BMP_TRANSPARENT) tft.drawPixel(ix, yt, ppixLine[ix - x]);
#else
            tft.drawPixel(ix, yt, ppixLine[ix - x]);
#endif
          }
          if (g_debug_output) Serial.printf("\t3:(%d, %d, %d, %d ++) %p\n", 0, yt, width, 1, ppixLine - x);
        }
        //WaitforWRComplete();
      }
      ppixLine += cx;
    }
  }
}

// Function to draw pixels to the display
void ScaleWriteClippedRect(int row, int image_width, uint16_t *usPixels) {
  // this methos assumes you are writing the data into the proper spots in Image_width*CLIP_LINES rectangle
  if ((row % g_image_scale) == (g_image_scale - 1)) {
    uint16_t newx = 0;
    for (uint16_t pix_cnt = 0; pix_cnt < image_width; pix_cnt += g_image_scale) {
      uint8_t red = 0;
      uint8_t green = 0;
      uint8_t blue = 0;
      float r = 0;
      float g = 0;
      float b = 0;
      for (uint8_t i = 0; i < g_image_scale; i++) {
        for (uint8_t j = 0; j < g_image_scale; j++) {
          Color565ToRGB(usPixels[pix_cnt + i + (j * image_width)], red, green, blue);
          // Sum the squares of components instead
          r += red * red;
          g += green * green;
          b += blue * blue;
        }
      }
      // overwrite the start of our buffer with
      usPixels[newx++] = Color565((uint8_t)sqrt(r / (g_image_scale * g_image_scale)), (uint8_t)sqrt(g / (g_image_scale * g_image_scale)), (uint8_t)sqrt(b / (g_image_scale * g_image_scale)));
    }
    writeClippedRect(0, row / g_image_scale, image_width / g_image_scale, 1, usPixels);
  }
}