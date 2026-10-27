
#include <Arduino.h>
#include <Wire.h>
#include <PCA9685.h>

PCA9685 pwm(0x40);

const int SERVO_MIN = 102;
const int SERVO_MAX = 512;
const int SERVO_COUNT = 4;

// 各サーボの往復設定
struct ServoConfig {
    int angleA;                  // 往復する角度A
    int angleB;                  // 往復する角度B
    unsigned long interval;      // 角度を切り替える間隔(ms)
};

ServoConfig config[SERVO_COUNT] = {
    { 5, 135, 5000 },  // サーボ0：0度 ⇔ 90度、1秒ごと
    { 172, 42, 5000 },  // サーボ1：45度 ⇔ 135度、1.5秒ごと
    { 177, 165, 5000 },  // サーボ2：30度 ⇔ 150度、2秒ごと
    { 3, 15, 5000 }   // サーボ3：60度 ⇔ 120度、0.5秒ごと
};

bool atAngleB[SERVO_COUNT] = {false, false, false, false};
unsigned long lastMove[SERVO_COUNT] = {0};

void servoWrite(int channel, int angle) {
    angle = constrain(angle, 0, 180);
    int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(channel, 0, pulse);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    pwm.begin();
    pwm.setPWMFreq(50);

    delay(500);

    // 起動時は全サーボを角度Aにする
    for (int ch = 0; ch < SERVO_COUNT; ch++) {
        servoWrite(ch, config[ch].angleA);
        lastMove[ch] = millis();
    }
}

void loop() {
    unsigned long now = millis();

    for (int ch = 0; ch < SERVO_COUNT; ch++) {
        if (now - lastMove[ch] >= config[ch].interval) {
            atAngleB[ch] = !atAngleB[ch];

            int angle = atAngleB[ch]
                ? config[ch].angleB
                : config[ch].angleA;

            servoWrite(ch, angle);
            lastMove[ch] = now;
        }
    }
}
