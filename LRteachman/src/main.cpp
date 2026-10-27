
#include <Arduino.h>
#include <Wire.h>
#include <PCA9685.h>

PCA9685 pwm(0x40);

const int SERVO_MIN = 102;
const int SERVO_MAX = 512;
const int SERVO_COUNT = 4;

// サーボごとのボタンピン
const int BUTTON_PIN = 2;

// 移動速度
const int ANGLE_STEP = 1;
const unsigned long MOVE_INTERVAL = 20;

// サーボごとの角度A・角度B
struct ServoConfig {
    int angleA;
    int angleB;
};

ServoConfig config[SERVO_COUNT] = {
    { 5, 135},  //　↖
    { 172, 42},  //　↗
    { 177, 165},  //　↙
    { 3, 15}   //　↘
};

int currentAngle[SERVO_COUNT] = {0, 45, 30, 60};
int targetAngle[SERVO_COUNT]  = {0, 45, 30, 60};

unsigned long lastStep[SERVO_COUNT] = {0};

bool atAngleB = false;
bool lastButtonState = HIGH;
unsigned long lastButtonTime = 0;

const unsigned long DEBOUNCE_MS = 30;


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

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    for (int ch = 0; ch < SERVO_COUNT; ch++) {
        currentAngle[ch] = config[ch].angleA;
        targetAngle[ch] = config[ch].angleA;
        servoWrite(ch, config[ch].angleA);
        lastStep[ch] = millis();
    }

    delay(1000);
}

void loop() {
    unsigned long now = millis();

    // ボタン入力
    bool buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW &&
        lastButtonState == HIGH &&
        now - lastButtonTime >= DEBOUNCE_MS) {

        atAngleB = !atAngleB;

        for (int ch = 0; ch < SERVO_COUNT; ch++) {
            targetAngle[ch] = atAngleB
                ? config[ch].angleB
                : config[ch].angleA;
        }

        lastButtonTime = now;
    }

    lastButtonState = buttonState;

    // 全サーボを少しずつ目標角度へ動かす
    for (int ch = 0; ch < SERVO_COUNT; ch++) {
        if (now - lastStep[ch] >= MOVE_INTERVAL) {
            if (currentAngle[ch] < targetAngle[ch]) {
                currentAngle[ch] += ANGLE_STEP;

                if (currentAngle[ch] > targetAngle[ch]) {
                    currentAngle[ch] = targetAngle[ch];
                }

                servoWrite(ch, currentAngle[ch]);
            }
            else if (currentAngle[ch] > targetAngle[ch]) {
                currentAngle[ch] -= ANGLE_STEP;

                if (currentAngle[ch] < targetAngle[ch]) {
                    currentAngle[ch] = targetAngle[ch];
                }

                servoWrite(ch, currentAngle[ch]);
            }

            lastStep[ch] = now;
        }
    }
}
