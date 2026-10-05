#include <Arduino.h>

const int DIN_PIN = 7;
const int LED_PIN = 13;

void setup(){
    pinMode( DIN_PIN, INPUT_PULLUP );
    pinMode( LED_PIN, OUTPUT );
}

void loop(){
    int value;
    
    value = digitalRead( DIN_PIN );

    if ( value == HIGH ){
        digitalWrite( LED_PIN, LOW );
    }else{
        digitalWrite( LED_PIN, HIGH );
    }

    delay( 100 );
}