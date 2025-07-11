#include <ESP32Servo.h>

// ——— Servo objects —————————————————————————————————————————————————————
Servo leftServo;
Servo rightServo;

// ——— Configuration ————————————————————————————————————————————————————
const int LEFT_SERVO_PIN  = 5;    // ESP32 Feather pin for left continuous‐rotation servo
const int RIGHT_SERVO_PIN = 4;    // ESP32 Feather pin for right continuous‐rotation servo

const int PWM_MIN    = 500;        // full reverse
const int PWM_STOP   = 1500;       // stop
const int PWM_MAX    = 2500;       // full forward

float speed = 0.0; // percent

void setup() {
    Serial.begin(115200);
    Serial.println(WHEEL_OMEGA_MAX);

  ////Servo
  // allocate two hardware PWM timers (required by ESP32Servo)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  // 50 Hz refresh rate, attach servos with custom pulse bounds
  leftServo.setPeriodHertz(50);
  rightServo.setPeriodHertz(50);
  leftServo.attach(LEFT_SERVO_PIN,  PWM_MIN, PWM_MAX);
  rightServo.attach(RIGHT_SERVO_PIN, PWM_MIN, PWM_MAX);
}

void loop() {
  // increments speed by 25% every loop, resetting to 0% after 100%.
  if (speed >= 1.0) {
    speed = 0.0;
    delay(1000);
  }
  speed = (speed + 0.25);
  leftServo.writeMicroseconds(PWM_STOP + (PWM_MAX - PWM_STOP) * speed);
  rightServo.writeMicroseconds(PWM_STOP + (PWM_MAX - PWM_STOP) * speed);   
  delay(2000);
  leftServo.writeMicroseconds(PWM_STOP);
  rightServo.writeMicroseconds(PWM_STOP);
  delay(1000);
}
