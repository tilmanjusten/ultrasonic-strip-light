#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <hcsr04.h>

#define TRIG_PIN 5
#define ECHO_PIN 6
#define PIXEL_PIN 3
#define NUMPIXELS 16

#define DISTANCE_MIN 30
#define OFFSET_MAX 50
#define STEPS 64
#define EXPONENT 2
#define BRIGHTNESS_MAX_REL pow(STEPS, EXPONENT)
#define BRIGHTNESS_MAX 255

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
HCSR04 hcsr04(TRIG_PIN, ECHO_PIN, 0, 1000);

int current = 0;
int distanceMax = 200;
int currentBrightness = 0;

void setup()
{
    Serial.begin(9600);
    pixels.begin();

    delay(20);

    for (int i = 0; i < NUMPIXELS; i++)
    {
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }

    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(20);

    distanceMax = hcsr04.distanceInMillimeters() - OFFSET_MAX;

    Serial.print("Max Distance is ");
    Serial.print(distanceMax);
    Serial.println("mm");

    delay(100);
}

void loop()
{
    int realDistance = hcsr04.distanceInMillimeters();

    if (realDistance <= distanceMax + (OFFSET_MAX / 2))
    {
        int x = max(0, min(STEPS, map(realDistance, DISTANCE_MIN, distanceMax, 0, STEPS)));
        unsigned int rel_brightness = pow(x, EXPONENT);
        int brightness = max(0, min(BRIGHTNESS_MAX, map(rel_brightness, 0, BRIGHTNESS_MAX_REL, 0, BRIGHTNESS_MAX)));
        int diff = brightness - currentBrightness;
        int step = diff > 0 ? 1 : -1;
        int start = 0;

        Serial.print("distance: ");
        Serial.print(realDistance);
        Serial.print(", x: ");
        Serial.print(x);
        Serial.print(", rel. brightness: ");
        Serial.print(rel_brightness);
        Serial.print(", current brightness: ");
        Serial.print(currentBrightness);
        Serial.print(", brightness: ");
        Serial.print(brightness);
        Serial.print(", diff: ");
        Serial.print(diff);
        Serial.println();

        while (start != diff)
        {
            start += step;
            Serial.print(start);
            Serial.print(" c: ");
            Serial.print(currentBrightness);
            Serial.print(" -> ");
            Serial.print(currentBrightness + start);
            Serial.println();

            int brightness_val = currentBrightness + start;

            for (int i = 0; i < NUMPIXELS; i++)
            {
                // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
                pixels.setPixelColor(i, pixels.Color(brightness_val, brightness_val, (int)brightness_val / 2));
            }

            // Serial.print("Update strip step -> ");
            // Serial.println(brightness);

            pixels.show(); // This sends the updated pixel color to the hardware.

            delay(10);
        }

        currentBrightness = brightness;

        delay(100);
    }

    delay(100);
}