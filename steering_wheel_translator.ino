#include <SPI.h>
#include <math.h>

#define PIONEER
//#define KENWOOD

#define MAZDA

// Variables specific to your controller / schematics
#define BUTTON_SENSOR_PIN 15
#define OPTOCOUPLER_PIN 16
#define CHIP_SELECT_POTENTIOMETER_PIN 10
#define DIVIDER_RESISTOR 270
#define POTMETER_RESISTANCE 100000
#define STEPS 256
#define RESISTANCE_PER_STEPS (POTMETER_RESISTANCE / STEPS)

#ifdef PIONEER
    #include "pioneer.h"
#endif
#ifdef KENWOOD
    #include "kenwood.h"
#endif

#ifdef MAZDA
    #include "mazda.h"
#endif

typedef struct button {
    int steps;
    int min;
    int max;
} button;

#define CALC_STEPS(resistance) round((double)resistance / (double)RESISTANCE_PER_STEPS)
#define CALC_ADC_READ(resistance) 1024 - (((double) DIVIDER_RESISTOR) / ((double) resistance + (double) DIVIDER_RESISTOR) * 1024)
#define BUTTON(resistance, min, max) {CALC_STEPS(resistance), CALC_ADC_READ(min), CALC_ADC_READ(max)}

button buttons[] {
    BUTTON(SOURCE_RESISTANCE, IN_SOURCE_MIN_RESISTANCE, IN_SOURCE_MAX_RESISTANCE),
    BUTTON(MUTE_RESISTANCE, IN_MUTE_MIN_RESISTANCE, IN_MUTE_MAX_RESISTANCE),
    BUTTON(DISPLAY_RESISTANCE, IN_DISPLAY_MIN_RESISTANCE, IN_DISPLAY_MAX_RESISTANCE),
    BUTTON(NEXT_TRACK_RESISTANCE, IN_NEXT_TRACK_MIN_RESISTANCE, IN_NEXT_TRACK_MAX_RESISTANCE),
    BUTTON(PREVIOUS_TRACK_RESISTANCE, IN_PREVIOUS_TRACK_MIN_RESISTANCE, IN_PREVIOUS_TRACK_MAX_RESISTANCE),
    BUTTON(VOLUME_UP_RESISTANCE, IN_VOLUME_UP_MIN_RESISTANCE, IN_VOLUME_UP_MAX_RESISTANCE),
    BUTTON(VOLUME_DOWN_RESISTANCE, IN_VOLUME_DOWN_MIN_RESISTANCE, IN_VOLUME_DOWN_MAX_RESISTANCE),
    BUTTON(ESCAPE_RESISTANCE, IN_ESCAPE_MIN_RESISTANCE, IN_ESCAPE_MAX_RESISTANCE)
};

#define ENTRIES sizeof(buttons) / sizeof(button)

void setup() {
    pinMode(BUTTON_SENSOR_PIN, INPUT);
    pinMode(CHIP_SELECT_POTENTIOMETER_PIN, OUTPUT);
    pinMode(OPTOCOUPLER_PIN, OUTPUT);
    SPI.begin();
    //Serial.begin(9600);
}

void setResistance(int steps) {
    digitalWrite(CHIP_SELECT_POTENTIOMETER_PIN, LOW);

    int address = 0;
    address |= (steps >> 8) & 0x01; // Get the eighth bit of the value, 
    // the potmeter uses 9 bits to decide swiper position and the zeroes is for the address
    int value = steps & 0xFF;
    SPI.transfer(address);
    SPI.transfer(value);

    digitalWrite(CHIP_SELECT_POTENTIOMETER_PIN, HIGH);
}

void loop() {
    int value = analogRead(BUTTON_SENSOR_PIN);
    //Serial.println(value);
    bool found = false;
    for (int i = 0; i < ENTRIES; i++) {
        button btn = buttons[i];
        if (btn.min <= value && value <= btn.max) {
            setResistance(btn.steps);
            found = true;
            
            //Serial.println("Button");
            //Serial.println(btn.steps);
            digitalWrite(OPTOCOUPLER_PIN, HIGH);
            break;
        }
    }

    if (!found) {
        setResistance(STEPS);
        digitalWrite(OPTOCOUPLER_PIN, LOW);
        //Serial.println(value);
    }

    delay(10);
}

