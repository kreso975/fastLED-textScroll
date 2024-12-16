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

const char* message = "Historia est magistra VITAE";
int scrollPosition = 0;

void setup()
{    
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


void displayMessage(CRGB color, const char* message, int numSpaces)
{
    static bool bufferInitialized = false;
    static CRGB* displayBuffer = nullptr; // Pointer to store the buffer
    static int bufferSize = 0;
    static const char* previousMessage = nullptr;
    static int previousNumSpaces = 0;

    // Check if the message or number of spaces has changed
    if (previousMessage != message || previousNumSpaces != numSpaces)
    {
        // Free the old buffer if it exists
        if (displayBuffer != nullptr)
            delete[] displayBuffer;

        // Calculate the new buffer size
        bufferSize = (strlen(message) + numSpaces) * 6 * kMatrixHeight;
        displayBuffer = new CRGB[bufferSize]; // Allocate memory for the new buffer

        // Initialize the buffer with the new message and spaces
        for (int i = 0; i < bufferSize; i++)
            displayBuffer[i] = CRGB::Black;

        for (int i = 0; i < strlen(message) + numSpaces; i++)
        {
            char charToDisplay = (i < numSpaces) ? ' ' : message[i - numSpaces];
            int charPosition = i * 6;
            for (int x = 0; x < FontWidth; x++)
            {
                for (int y = 0; y < FontHeight; y++)
                {
                    if (bitRead(pgm_read_byte(&(Font[charToDisplay][x])), y) == 1)
                    {
                        int bufferIndex = (charPosition + x) + (y * (strlen(message) + numSpaces) * 6);
                        if (bufferIndex < bufferSize)
                            displayBuffer[bufferIndex] = color;
                    }
                }
            }
        }

        previousMessage = message; // Update the previous message
        previousNumSpaces = numSpaces; // Update the previous number of spaces
        scrollPosition = 0; // Reset the scroll position
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the display

    // Copy the relevant part of the buffer to the LED matrix
    for (int x = 0; x < kMatrixWidth; x++)
    {
        for (int y = 0; y < kMatrixHeight; y++)
        {
            int bufferIndex = (scrollPosition + x) % ((strlen(message) + numSpaces) * 6) + (y * (strlen(message) + numSpaces) * 6);
            if (bufferIndex < bufferSize)
                leds[XYsafe(kMatrixWidth - 1 - x, y)] = displayBuffer[bufferIndex];
        }
    }

    scrollPosition++;
    if (scrollPosition >= (strlen(message) + numSpaces) * 6)
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








