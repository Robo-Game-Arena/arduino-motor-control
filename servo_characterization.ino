#include <ESP32Servo.h>

// ——— Servo objects —————————————————————————————————————————————————————
Servo leftServo;
Servo rightServo;

// ——— Configuration ————————————————————————————————————————————————————
const float WHEELBASE = 90.0;         // distance between wheels in millimeters
const float WHEEL_RADIUS = 25.0;      //
const float WHEEL_VEL_MAX = 500.0;    // max wheel velocity in millimeters/sec
const float WHEEL_OMEGA_MAX = WHEEL_VEL_MAX / WHEEL_RADIUS;  // max wheel angular velocity in radians/sec

const int LEFT_SERVO_PIN  = 5;    // ESP32 Feather pin for left continuous‐rotation servo
const int RIGHT_SERVO_PIN = 4;    // ESP32 Feather pin for right continuous‐rotation servo

const float OFFSETS[5] = {0.000, 0.250, 0.500, 0.750, 1.000};
const float SCALE_L[5] = {0.000, 0.159, 0.324, 0.495, 1.000};
const float SCALE_R[5] = {0.000, 0.116, 0.231, 0.410, 0.686};


const int PWM_MIN    = 500;        // full reverse
const int PWM_STOP   = 1500;       // stop
const int PWM_MAX    = 2500;       // full forward

float speed = 0.0; // percent

int omegaToOffset(int raw, bool isLeft) {
  float percent_offset = float(map(raw, -WHEEL_OMEGA_MAX, WHEEL_OMEGA_MAX, -1000, 1000)) / 1000;
  float scale = 0;
  for (int i = 0; i < 4; i++) {
    if ((OFFSETS[i] <= abs(percent_offset)) & (abs(percent_offset) <= OFFSETS[i+1])) { 
      float x = (percent_offset - OFFSETS[i]) / 0.25; 
      if (isLeft) {
        scale = SCALE_L[i] + x * (SCALE_L[i+1] - SCALE_L[i]);
      }
      else {
        scale = SCALE_R[i] + x * (SCALE_R[i+1] - SCALE_R[i]);
      }
    }
  }
  Serial.println(scale);
  if (raw > 0) { //positive
    return int((PWM_MAX - PWM_STOP) * scale);
  }
  else {
    return int((PWM_MIN - PWM_STOP) * scale);
  }
}

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
  leftServo.writeMicroseconds(PWM_STOP + omegaToOffset(WHEEL_OMEGA_MAX*speed, true));
  rightServo.writeMicroseconds(PWM_STOP - omegaToOffset(WHEEL_OMEGA_MAX*speed, false));     
  delay(2000);
  leftServo.writeMicroseconds(PWM_STOP);
  rightServo.writeMicroseconds(PWM_STOP);
  delay(1000);
}
