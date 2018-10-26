#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <hcsr04.h>
#include <LowPower.h>

#define PIXEL_PIN_1 2
#define TRIG_PIN_1 3
#define ECHO_PIN_1 4

#define PIXEL_PIN_2 6
#define TRIG_PIN_2 7
#define ECHO_PIN_2 8

#define NUMPIXELS 12

#define DISTANCE_MIN 30
#define OFFSET_MAX 25
#define OFFSET_MIN 15
#define STEPS 64
#define EXPONENT 2
#define BRIGHTNESS_MAX_REL pow(STEPS, EXPONENT)
#define BRIGHTNESS_MAX 255
#define DURATION 1000

Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN_1, NEO_GRB + NEO_KHZ800);
HCSR04 hcsr04_1(TRIG_PIN_1, ECHO_PIN_1, OFFSET_MIN, 500 + OFFSET_MAX);

int distanceMax1 = 200;
int currentBrightness1 = 0;

Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN_2, NEO_GRB + NEO_KHZ800);
HCSR04 hcsr04_2(TRIG_PIN_2, ECHO_PIN_2, OFFSET_MIN, 500 + OFFSET_MAX);

int distanceMax2 = 200;
int currentBrightness2 = 0;

void sleep(period_t sleepTime)
{
    LowPower.idle(
        sleepTime,
        ADC_OFF,
        TIMER2_OFF,
        TIMER1_OFF,
        TIMER0_OFF,
        SPI_OFF,
        USART0_OFF,
        TWI_OFF);
}

int updateStrip(int no, Adafruit_NeoPixel &pixels, int realDistance, int distanceMax, int currentBrightness)
{
    int x = max(0, min(STEPS, map(realDistance, DISTANCE_MIN, distanceMax, 0, STEPS)));
    unsigned int rel_brightness = pow(x, EXPONENT);
    int brightness = max(0, min(BRIGHTNESS_MAX, map(rel_brightness, 0, BRIGHTNESS_MAX_REL, 0, BRIGHTNESS_MAX)));
    int diff = brightness - currentBrightness;
    int step = diff > 0 ? 1 : -1;
    int progress = 0;
    // ~ 60fps minimum
    int changeDelay = min(16, ceil(DURATION / abs(diff)));

    Serial.print("Update brightness of ");
    Serial.print(no);
    Serial.print(" from ");
    Serial.print(currentBrightness);
    Serial.print(" to ");
    Serial.print(brightness);
    Serial.println();

    while (progress != diff)
    {
        progress += step;

        int brightness_val = currentBrightness + progress;
        int brightness_val_R = brightness_val * 1;
        int brightness_val_G = brightness_val_R > 0 ? max(1, brightness_val * 0.5) : 0;
        int brightness_val_B = brightness_val_R > 9 ? max(1, brightness_val * 0.1) : 0;

        for (int i = 0; i < NUMPIXELS; i++)
        {
            // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
            pixels.setPixelColor(i, pixels.Color(brightness_val_R, brightness_val_G, brightness_val_B));
        }

        // Serial.print("Update strip step -> ");
        // Serial.println(brightness);

        pixels.show(); // This sends the updated pixel color to the hardware.

        delay(changeDelay);
    }

    sleep(SLEEP_1S);

    return brightness;
}

void setup()
{
    Serial.begin(9600);

    pixels1.begin();
    delay(10);

    pixels2.begin();
    delay(10);

    for (int i = 0; i < NUMPIXELS; i++)
    {
        // pixels1.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels1.setPixelColor(i, pixels1.Color(0, 0, 0));
        pixels2.setPixelColor(i, pixels2.Color(0, 0, 0));
    }

    pixels1.show(); // This sends the updated pixel color to the hardware.
    pixels2.show();

    delay(10);

    Serial.print("Min distance is ");
    Serial.println(OFFSET_MIN * 2);
    Serial.println("mm");

    distanceMax1 = hcsr04_1.distanceInMillimeters() - (OFFSET_MAX * 2);

    Serial.print("#1 Max distance is ");
    Serial.print(distanceMax1);
    Serial.println("mm");

    delay(10);

    distanceMax2 = hcsr04_2.distanceInMillimeters() - (OFFSET_MAX * 2);

    Serial.print("#2 Max distance is ");
    Serial.print(distanceMax2);
    Serial.println("mm");

    delay(100);
}

void loop()
{
    sleep((currentBrightness1 < 1 && currentBrightness2 < 1) ? SLEEP_2S : SLEEP_250MS);

    int realDistance1 = hcsr04_1.distanceInMillimeters();

    if (realDistance1 > 0 && realDistance1 <= distanceMax1 + OFFSET_MAX)
    {
        currentBrightness1 = updateStrip(1, pixels1, realDistance1, distanceMax1, currentBrightness1);
    }

    sleep(SLEEP_120MS);

    int realDistance2 = hcsr04_2.distanceInMillimeters();

    if (realDistance2 > 0 && realDistance2 <= distanceMax2 + OFFSET_MAX)
    {
        currentBrightness2 = updateStrip(2, pixels2, realDistance2, distanceMax2, currentBrightness2);
    }
}