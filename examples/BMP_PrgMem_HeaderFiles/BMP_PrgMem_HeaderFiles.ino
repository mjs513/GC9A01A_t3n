#include <SD.h>
//============================================
#define SD_CS BUILTIN_SDCARD
File imageFile;
const char *name = nullptr;
uint8_t g_debug_BMP = 0;

//============================================
//Image storage size for

int HeadingIndicator_Aircraft_width = 138;
int HeadingIndicator_Aircraft_height = 172;
uint16_t HeadingIndicator_Aircraft_buffer[138 * 172];
uint16_t *HeadingIndicator_Aircraft_image;

int HeadingIndicator_Background_width = 260;
int HeadingIndicator_Background_height = 260;
uint16_t HeadingIndicator_Background_buffer[260 * 260];
uint16_t *HeadingIndicator_Background_image;

int HeadingWeel_width = 237;
int HeadingWeel_height = 237;
uint16_t HeadingWeel_buffer[237 * 237];
uint16_t *HeadingWeel_image;

void setup() {

  while (!Serial && millis() < 5000)
    ;  // wait for Arduino Serial Monitor
  Serial.begin(9600);
  if (CrashReport) Serial.print(CrashReport);
  delay(500);

  Serial.print(F("Initializing SD card...\n"));
  if (!SD.begin(SD_CS)) {
    while (!SD.begin(SD_CS)) {
      //Serial.println(F("failed to access SD card!"));
      Serial.printf("failed to access SD card on cs:%u!\n", SD_CS);
      delay(2000);
    }
  }
  
  Generate_embed_files();
}

void loop() {}

void Generate_embed_files()
{
  //
  imageFile = SD.open("HeadingIndicator_Aircraft.bmp", FILE_READ);
  name = "HeadingIndicator_Aircraft.bmp";
  HeadingIndicator_Aircraft_image = bmpImport(imageFile, name, HeadingIndicator_Aircraft_width, HeadingIndicator_Aircraft_height, HeadingIndicator_Aircraft_buffer);
  Generate_embed_file("HeadingIndicator_Aircraft", HeadingIndicator_Aircraft_width, HeadingIndicator_Aircraft_height, HeadingIndicator_Aircraft_image);
  Serial.printf("Generated %s.h on SD Card\n", name);
  //
  imageFile = SD.open("HeadingIndicator_Background.bmp", FILE_READ);
  name = "HeadingIndicator_Background.bmp";
  HeadingIndicator_Background_image = bmpImport(imageFile, name, HeadingIndicator_Background_width, HeadingIndicator_Background_height, HeadingIndicator_Background_buffer);
  Generate_embed_file("HeadingIndicator_Background", HeadingIndicator_Background_width, HeadingIndicator_Background_height, HeadingIndicator_Background_image);
  Serial.printf("Generated %s.h on SD Card\n", name);
  //
  imageFile = SD.open("HeadingWeel.bmp", FILE_READ);
  name = "HeadingWeel.bmp";
  HeadingWeel_image = bmpImport(imageFile, name, HeadingWeel_width, HeadingWeel_height, HeadingWeel_buffer);
  Generate_embed_file("HeadingWeel", HeadingWeel_width, HeadingWeel_height, HeadingWeel_image);
  Serial.printf("Generated %s.h on SD Card\n", name);

  Serial.println("FINI !!");
}


void Generate_embed_file(const char *name, uint32_t width, uint32_t height, uint16_t *image)
{
  char file_name[256];
  uint32_t image_size = (uint32_t)width * height;
  uint32_t index;

  sprintf(file_name, "%s.h", (char *)name);
  imageFile = SD.open(file_name, FILE_WRITE_BEGIN);
  imageFile.truncate(0);
  imageFile.printf("const int %s_width = %u;\n", name, width);
  imageFile.printf("const int %s_height = %u;\n", name, height);

  imageFile.println("#ifndef USE_RLE_IMAGE");
  imageFile.printf("const uint16_t %s_image[%u] PROGMEM = {\n", name, image_size);
  for (index = 0; index < image_size -1; index++) {
    imageFile.printf("0x%04X,", image[index]);
    if ((index & 0xf) == 0xf) {
      imageFile.printf("    // 0x%04X(%u)\n", index+1, index+1);
    }
  }
  // Need to output the last pixel
  imageFile.printf("0x%04X     // 0x%04X(%u)\n", image[index], index+1, index+1);
  imageFile.printf("};\n");
  imageFile.println("#else // USE_RLE_IMAGE");

  // Experiment a quick and dirty for some form of RLE encoding...
  imageFile.printf("const uint16_t %s_image[] PROGMEM = {\n", name);
  // bugbug experiment allow lengths of 2 bytes... maybe 1 later..
  uint16_t rle_color = image[0];
  int row_count_output = 0;

  uint16_t rle_len = 1;
  uint32_t rle_size = 0;
  for (index = 1; index < image_size; index++) {
    if (image[index] == rle_color) rle_len++;
    else {
      imageFile.printf("%d,0x%04X, ", rle_len, rle_color);
      rle_size += 2;
      row_count_output++;
      if (row_count_output == 8) {
        imageFile.print("\n");
        row_count_output = 0;
      }
      rle_color = image[index];
      rle_len = 1;
    }
  }
  // Need to output the last pixel
  imageFile.printf("%5d,0x%04X\n", rle_len, rle_color);
  rle_size += 2;
  imageFile.printf("};    // %04X(%u)\n", rle_size, rle_size);
  imageFile.println("#endif // USE_RLE_IMAGE");

  imageFile.close();
  
}
  static uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

#define BUFFPIXEL 80
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
