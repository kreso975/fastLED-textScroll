#include <FastLED.h> // version 3.7.8
#include "Font6x8.h"

#define LED_PIN 0
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define BRIGHTNESS 20

// Params for width and height
const uint8_t kMatrixWidth = 32;
const uint8_t kMatrixHeight = 8;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;
const bool    kMatrixVertical = true;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

const char* message = "Historia est Magistra VITAE";
int scrollPosition = 0;

void setup()
{   
    //Serial.begin(115200); // OFF for better performance
    //delay(3000);
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 700); // cap on 700mA
    FastLED.setBrightness(BRIGHTNESS);    
}

void loop()
{
    displayMessage(CRGB::Blue, message, 8); // Scroll message
    for (int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8_video(BRIGHTNESS); } // adjust brightness
    //FastLED.setBrightness(BRIGHTNESS);
    // Show the LEDDs
    FastLED.show();
    FastLED.delay(10);
    
}

String convertToSingleByte(String input) {
    String output = "";
    for (int i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (c == 0xC4 || c == 0xC5) {
            char nextChar = input.charAt(i + 1);
            switch (nextChar) {
                case 0x8D: output += (char)0xE8; i++; break; // č (C4)
                case 0x8C: output += (char)0xC8; i++; break; // Č (C4)
                case 0x87: output += (char)0xE6; i++; break; // ć (C4)
                case 0x86: output += (char)0xC6; i++; break; // Ć (C4)
                case 0xBE: output += (char)0x9E; i++; break; // ž (C5)
                case 0xBD: output += (char)0x8E; i++; break; // Ž (C5)
                case 0xA1: output += (char)0x9A; i++; break; // š (C5)
                case 0xA0: output += (char)0x8A; i++; break; // Š (C5)
                case 0x91: output += (char)0xF0; i++; break; // đ (C5)
                case 0x90: output += (char)0xD0; i++; break; // Đ (C5)
                default: output += c; break;
            }
        } else {
            output += c;
        }
    }
    return output;
}

void displayMessage(CRGB color, const char* message, int numSpaces) {
    static bool bufferInitialized = false;
    static CRGB* displayBuffer = nullptr; // Pointer to store the buffer
    static int bufferSize = 0;
    static String previousMessage = "";
    static int previousNumSpaces = 0;

    String convertedMessage = convertToSingleByte(message);
    
    // Check if the message or number of spaces has changed
    if (previousMessage != convertedMessage || previousNumSpaces != numSpaces)
    {
        // Free the old buffer if it exists
        if (displayBuffer != nullptr) {
            delete[] displayBuffer;
        }

        // Calculate the new buffer size
        bufferSize = (convertedMessage.length() + numSpaces) * 6 * kMatrixHeight;
        displayBuffer = new CRGB[bufferSize]; // Allocate memory for the new buffer

        // Initialize the buffer with the new message and spaces
        for (int i = 0; i < bufferSize; i++)
            displayBuffer[i] = CRGB::Black;

        // Add spaces in front of the message
        for (int i = 0; i < numSpaces; i++) {
            int charPosition = i * 6;
            for (int x = 0; x < 6; x++) {
                for (int y = 0; y < 8; y++) {
                    int bufferIndex = (charPosition + x) + (y * (convertedMessage.length() + numSpaces) * 6);
                    if (bufferIndex < bufferSize)
                        displayBuffer[bufferIndex] = CRGB::Black;
                }
            }
        }

        for (int i = 0; i < convertedMessage.length(); i++) {
            char charToDisplay = convertedMessage.charAt(i);
            int charPosition = (i + numSpaces) * 6;
            for (int x = 0; x < 6; x++) {
                for (int y = 0; y < 8; y++) {
                    if (bitRead(pgm_read_byte(&(Font[charToDisplay][x])), y) == 1) {
                        int bufferIndex = (charPosition + x) + (y * (convertedMessage.length() + numSpaces) * 6);
                        if (bufferIndex < bufferSize)
                            displayBuffer[bufferIndex] = color;
                    }
                }
            }
        }

        previousMessage = convertedMessage; // Update the previous message
        previousNumSpaces = numSpaces; // Update the previous number of spaces
        scrollPosition = 0; // Reset the scroll position
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the display

    // Copy the relevant part of the buffer to the LED matrix
    for (int x = 0; x < kMatrixWidth; x++) {
        for (int y = 0; y < kMatrixHeight; y++) {
            int bufferIndex = (scrollPosition + x) % ((convertedMessage.length() + numSpaces) * 6) + (y * (convertedMessage.length() + numSpaces) * 6);
            if (bufferIndex < bufferSize)
                leds[XYsafe(kMatrixWidth - 1 - x, y)] = displayBuffer[bufferIndex];
        }
    }

    scrollPosition++;
    if (scrollPosition >= (convertedMessage.length() + numSpaces) * 6)
        scrollPosition = 0;

}


uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false)
  {
    if (kMatrixVertical == false)
      i = (y * kMatrixWidth) + x;
    else
      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
  }

  if( kMatrixSerpentineLayout == true)
  {
    if (kMatrixVertical == false)
    {
      if( y & 0x01)
      {
        // Odd rows run backwards
        uint8_t reverseX = (kMatrixWidth - 1) - x;
        i = (y * kMatrixWidth) + reverseX;
      } 
      else
      {
        // Even rows run forwards
        i = (y * kMatrixWidth) + x;
      }
    }
    else
    { // vertical positioning
      if ( x & 0x01)
        i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
      else
        i = kMatrixHeight * (kMatrixWidth - x) - (y+1);
    }
  }
  
  return i;
}

uint16_t XYsafe( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}








