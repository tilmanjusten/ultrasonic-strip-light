#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <hcsr04.h>

#define TRIG_PIN_1 3
#define ECHO_PIN_1 4
#define PIXEL_PIN_1 2
#define NUMPIXELS 12

#define DISTANCE_MIN 30
#define OFFSET_MAX 50
#define STEPS 64
#define EXPONENT 2
#define BRIGHTNESS_MAX_REL pow(STEPS, EXPONENT)
#define BRIGHTNESS_MAX 255
#define DURATION 1000

Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN_1, NEO_GRB + NEO_KHZ800);
HCSR04 hcsr04_1(TRIG_PIN_1, ECHO_PIN_1, 0, 1000);

int distanceMax1 = 200;
int currentBrightness1 = 0;

int updateStrip(Adafruit_NeoPixel &pixels, int realDistance, int distanceMax, int currentBrightness)
{
    int x = max(0, min(STEPS, map(realDistance, DISTANCE_MIN, distanceMax, 0, STEPS)));
    unsigned int rel_brightness = pow(x, EXPONENT);
    int brightness = max(0, min(BRIGHTNESS_MAX, map(rel_brightness, 0, BRIGHTNESS_MAX_REL, 0, BRIGHTNESS_MAX)));
    int diff = brightness - currentBrightness;
    int step = diff > 0 ? 1 : -1;
    int progress = 0;
    int changeDelay = ceil(DURATION / abs(diff));

    Serial.print("Update brightness from ");
    Serial.print(currentBrightness);
    Serial.print(" to ");
    Serial.print(brightness);
    Serial.println();

    while (progress != diff)
    {
        progress += step;

        int brightness_val = currentBrightness + progress;

        for (int i = 0; i < NUMPIXELS; i++)
        {
            // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
            pixels.setPixelColor(i, pixels.Color(brightness_val, brightness_val, (int)brightness_val / 2));
        }

        // Serial.print("Update strip step -> ");
        // Serial.println(brightness);

        pixels.show(); // This sends the updated pixel color to the hardware.

        delay(changeDelay);
    }

    delay(500);

    return brightness;
}

void setup()
{
    Serial.begin(9600);
    pixels1.begin();

    delay(20);

    for (int i = 0; i < NUMPIXELS; i++)
    {
        // pixels1.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels1.setPixelColor(i, pixels1.Color(0, 0, 0));
    }

    pixels1.show(); // This sends the updated pixel color to the hardware.

    delay(20);

    distanceMax1 = hcsr04_1.distanceInMillimeters() - OFFSET_MAX;

    Serial.print("Max Distance is ");
    Serial.print(distanceMax1);
    Serial.println("mm");

    delay(100);
}

void loop()
{
    int realDistance1 = hcsr04_1.distanceInMillimeters();

    if (realDistance1 <= distanceMax1 + (OFFSET_MAX / 2))
    {
        currentBrightness1 = updateStrip(pixels1, realDistance1, distanceMax1, currentBrightness1);
    }

    delay(500);
}