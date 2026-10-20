#include <Arduino.h>
#include <Servo.h>

Servo myServo;

const int servoPin = 9;
const int button0 = 3;
const int button180 = 2;

const unsigned long randomDuration = 0; // ランダム動作時間

bool moving = false;
int targetAngle = 90;

unsigned long startTime = 0;
unsigned long lastMoveTime = 0;

void setup() {
  myServo.attach(servoPin,500, 2500); //sf90には500～2500の範囲で書き込む必要がある
  //myServo.attach(servoPin);

  pinMode(button0, INPUT_PULLUP);
  pinMode(button180, INPUT_PULLUP);

  myServo.write(90);

  randomSeed(analogRead(A0));
}

void loop() {

  // ランダム動作中
  if (moving) {

    // 一定間隔でランダムな角度へ移動
    if (millis() - lastMoveTime >= 100) {
      int randomAngle = random(0, 181);
      myServo.write(randomAngle);
      lastMoveTime = millis();
    }

    // 3秒経過したら設定角度へ
    if (millis() - startTime >= randomDuration) {
      myServo.write(targetAngle);
      moving = false;
    }

    return;
  }

  // D2 → 最終的に0°
  if (digitalRead(button0) == LOW) {
    targetAngle = 0;
    moving = true;
    startTime = millis();
    lastMoveTime = millis();

    delay(200); // ボタンのチャタリング対策
  }

  // D3 → 最終的に180°
  else if (digitalRead(button180) == LOW) {
    targetAngle = 180;
    moving = true;
    startTime = millis();
    lastMoveTime = millis();

    delay(200);
  }
}