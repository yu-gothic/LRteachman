
#include <Arduino.h>
#include <Wire.h>
#include <PCA9685.h>

PCA9685 pwm(0x40);

// サーボ設定
const int SERVO_MIN = 102;
const int SERVO_MAX = 512;
const int SERVO_COUNT = 5;

// ボタン
const int BUTTON_A = 2;
const int BUTTON_B = 3;

// 移動速度
const int ANGLE_STEP = 1;
const unsigned long MOVE_INTERVAL = 20;

// 角度B到達後、角度Aに戻るまでの時間
const unsigned long RETURN_DELAY = 6000;

// サーボごとの角度
struct ServoConfig {
    int angleA;
    int angleB;
};

ServoConfig config[SERVO_COUNT] = {
    {135,   5},  // ↖
    { 42, 172},  // ↗
    {165, 177},  // ↙
    { 15,   3},  // ↘
    {  0, 180}   // 5つ目
};

int currentAngle[SERVO_COUNT];
int targetAngle[SERVO_COUNT];
unsigned long lastStep[SERVO_COUNT] = {0};

// ボタンのチャタリング対策
const unsigned long DEBOUNCE_MS = 30;
const int buttonPins[2] = {BUTTON_A, BUTTON_B};

bool lastReading[2] = {HIGH, HIGH};
bool stableState[2] = {HIGH, HIGH};
unsigned long lastChange[2] = {0, 0};


// 動作状態
enum MotionState {
    IDLE,
    MOVING_TO_B,
    WAITING,
    RETURNING
};

MotionState motionState = IDLE;

unsigned long reachedBAt = 0;
int pendingServo4Angle = 90;

// サーボに角度を指令
void servoWrite(int channel, int angle) {
    angle = constrain(angle, 0, 180);

    int pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(channel, 0, pulse);
}

// サーボ0〜3の移動完了判定
bool firstFourReachedTarget() {
    for (int ch = 0; ch < 4; ch++) {
        if (currentAngle[ch] != targetAngle[ch]) {
            return false;
        }
    }
    return true;
}

// ボタン操作
void activateButton(int buttonIndex) {
    // 初期位置に戻るまでボタン操作を無視
    if (motionState != IDLE) {
        return;
    }

    // サーボ0〜3を角度Bへ
    for (int ch = 0; ch < 4; ch++) {
        targetAngle[ch] = config[ch].angleB;
    }

    // サーボ4の移動先を予約
    pendingServo4Angle = (buttonIndex == 0) ? 0 : 180;

    motionState = MOVING_TO_B;
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);

    pwm.begin();
    pwm.setPWMFreq(50);
    delay(500);

    // 初期位置
    for (int ch = 0; ch < SERVO_COUNT; ch++) {
        int initialAngle = (ch == 4)
            ? 90
            : config[ch].angleA;

        currentAngle[ch] = initialAngle;
        targetAngle[ch] = initialAngle;

        servoWrite(ch, initialAngle);
        lastStep[ch] = millis();
    }

    delay(1000);

    for (int i = 0; i < 2; i++) {
        lastReading[i] = digitalRead(buttonPins[i]);
        stableState[i] = lastReading[i];
    }
}

void loop() {
    unsigned long now = millis();

    // ボタン入力
    for (int i = 0; i < 2; i++) {
        bool reading = digitalRead(buttonPins[i]);

        if (reading != lastReading[i]) {
            lastChange[i] = now;
        }

        if (now - lastChange[i] >= DEBOUNCE_MS &&
            reading != stableState[i]) {

            stableState[i] = reading;

            if (stableState[i] == LOW) {
                activateButton(i);
            }
        }

        lastReading[i] = reading;
    }

    // 全サーボを少しずつ動かす
    for (int ch = 0; ch < SERVO_COUNT; ch++) {
        if (now - lastStep[ch] >= MOVE_INTERVAL) {

            if (currentAngle[ch] < targetAngle[ch]) {
                currentAngle[ch] += ANGLE_STEP;

                if (currentAngle[ch] > targetAngle[ch]) {
                    currentAngle[ch] = targetAngle[ch];
                }
            }
            else if (currentAngle[ch] > targetAngle[ch]) {
                currentAngle[ch] -= ANGLE_STEP;

                if (currentAngle[ch] < targetAngle[ch]) {
                    currentAngle[ch] = targetAngle[ch];
                }
            }

            if (currentAngle[ch] != targetAngle[ch] ||
                motionState != IDLE) {
                servoWrite(ch, currentAngle[ch]);
            }

            lastStep[ch] = now;
        }
    }

    // 4個のサーボが角度Bに到達
    if (motionState == MOVING_TO_B &&
        firstFourReachedTarget()) {

        // ここで初めて5つ目のサーボを動かす
        targetAngle[4] = pendingServo4Angle;

        reachedBAt = now;
        motionState = WAITING;
    }

    // 一定時間後にサーボ0〜3を戻す
    if (motionState == WAITING &&
        now - reachedBAt >= RETURN_DELAY) {

        for (int ch = 0; ch < 4; ch++) {
            targetAngle[ch] = config[ch].angleA;
        }
        targetAngle[4] = 90; // サーボ4は中央に戻す

        motionState = RETURNING;
    }

    // 5個すべてが初期位置に戻ったら操作可能
    if (motionState == RETURNING) {
        bool allReturned = true;

        for (int ch = 0; ch < 4; ch++) {
            if (currentAngle[ch] != config[ch].angleA) {
                allReturned = false;
                break;
            }
        }

        if (currentAngle[4] != 90) {
            allReturned = false;
        }

        if (allReturned) {
            motionState = IDLE;
        }
    }
}
