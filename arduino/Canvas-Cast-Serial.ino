/**
 * Canvas Cast Serial
 * https://github.com/owenmcateer/canvas-cast-serial
 *
 * How to use:
 * 1) Edit the config settings below.
 * 2) Upload to your Arduino device.
 *    The following libraries are required:
 *     - FastLED ()>= 3.1.8)
 * 3) Open the HTML page using your Arduino's serial port.
 *
 * Report any issues to: https://github.com/owenmcateer/canvas-cast-serial/issues
 */

// Matrix size
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 8;

// This can be used as a safety feature (0-255)
const int maxBrightness = 128;

// Matrix settings
// See FastLED for more info:
// https://github.com/FastLED/FastLED/wiki/Overview
#define LED_PIN D4
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define BRIGHTNESS 127

//
// End config
//

// Matrix setup
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_DMA
#include <FastLED.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001008)
#warning "Requires FastLED 3.1.8 or later; check github for latest code."
#endif

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[NUM_LEDS + 1];
CRGB* const leds(leds_plus_safety_pixel + 1);

// Header codes
const uint8_t frameHeader[3] = { 0xDE, 0xC0, 0xDE };
const uint8_t commandHeader[3] = { 0xC0, 0xFF, 0xEE };

// Setup
void setup() {
  Serial.begin(1000000);
  delay(10);

  // Start LEDs
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(min(BRIGHTNESS, maxBrightness));

  // Set status LED
  FastLED.clear();
  leds[0] = CRGB::Green;
  FastLED.show();
}


void loop() {
  // Read the serial data directly, then we'll check for headers.
  while(true) {
    while(Serial.available() == 0){ } // Wait for new serial data
    uint8_t b = Serial.read();

    // Is this a frame incoming?
    bool isFrameHeader = false;
    if(b == frameHeader[0]) {
      isFrameHeader = true;
      for(int i = 1; isFrameHeader && (i < sizeof(frameHeader)); i++) {
        while(Serial.available() == 0){} // Wait for new serial data
        b = Serial.read();
        if(b != frameHeader[i]) {
          // This doesn't look like a frame header, break.
          isFrameHeader = false;
        }
      }
    }

    // Check for a command header
    bool isCommandHeader = false;
    if(b == commandHeader[0]) {
      isCommandHeader = true;
      for(int i = 1; isCommandHeader && (i < sizeof(commandHeader) + 1); i++) {
        while(Serial.available() == 0){} // Wait for new serial data
        b = Serial.read();
         if (i == 3) {
          // Once found run commnad
          int mappedBrightness = map(int(b), 0, 255, 0, maxBrightness);
          FastLED.setBrightness(mappedBrightness);
          break;
        }

        if(b != commandHeader[i]) {
          // This doesn't look like a command header, break.
          isCommandHeader = false;
        }
      }
    }

    // Frame incoming
    if(isFrameHeader) {
      // This is a frame of data, we can now read all the LED data.
      int bytesRead = 0;
      while(bytesRead < (NUM_LEDS *3)) {
        bytesRead += Serial.readBytes(((uint8_t*)leds) + bytesRead, (NUM_LEDS*3)-bytesRead);
      }
      // Keep reading data until we have the entire frame then breal.
      break;
    }
  }

  // Send frame data to the LEDs
  FastLED.show();

  // Finally, flush out any data in the serial buffer
  // Ready to start fresh again.
  // This may be needed when we have been interrupted oddly by writing out led data.
  while(Serial.available() > 0) { Serial.read(); }
}
