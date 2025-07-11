#include <Ps3Controller.h>
#include <ESP32Servo.h>

// ——— Servo objects —————————————————————————————————————————————————————
Servo leftServo;
Servo rightServo;
Servo grabServo;

// ——— Configuration ————————————————————————————————————————————————————
const int LEFT_SERVO_PIN  = 5;    // ESP32 Feather pin for left continuous‐rotation servo
const int RIGHT_SERVO_PIN = 4;    // ESP32 Feather pin for right continuous‐rotation servo
const int GRAB_SERVO_PIN = 13;

const float WHEELBASE = 130.0;         // distance between wheels in millimeters
const float WHEEL_RADIUS = 30.0;      //
const float WHEEL_VEL_MAX = 348.0;    // max wheel velocity in millimeters/sec
const float WHEEL_OMEGA_MAX = WHEEL_VEL_MAX / WHEEL_RADIUS;  // max wheel angular velocity in radians/sec
const float BODY_VEL_MAX = WHEEL_VEL_MAX; // given 0 turning
const float BODY_OMEGA_MAX = WHEEL_VEL_MAX / (WHEELBASE / 2) ; // given 0 displacement

const float OFFSETS[5] = {0.000, 0.250, 0.500, 0.750, 1.000};
const float SCALE_L[5] = {0.000, 0.159, 0.324, 0.495, 1.000};
const float SCALE_R[5] = {0.000, 0.101, 0.201, 0.380, 0.624};

const int PWM_MIN    = 500;        // full reverse
const int PWM_STOP   = 1500;       // stop
const int PWM_MAX    = 2500;       // full forward

const int DEADZONE   = 5;         // stick deadzone (0–255)

bool adjustSpeed = false;
float speedAdjustment = 0.0;
float leftScale = 1.0;
float rightScale = 1.0;
const float ADJUSTMENT_STEP = 0.05; // Adjustment step size for controller buttons
unsigned long lastButtonPress = 0;
const unsigned long BUTTON_DEBOUNCE = 200; // ms

int sign(int x) {
  return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

// Function to set motor PWM values
void setMotorPWM(int leftPwm, int rightPwm) {
  // Clamp to valid range
  leftPwm  = constrain(leftPwm,  PWM_MIN, PWM_MAX);
  rightPwm = constrain(rightPwm, PWM_MIN, PWM_MAX);

  leftServo.writeMicroseconds(leftPwm);
  rightServo.writeMicroseconds(rightPwm);
}

struct PWM {
  int left;
  int right;
};

struct PWM velocitiesToPWM(int v, int w) {
  float new_w = w / 180.0 * PI;
  Serial.println("v");
  Serial.println(v);
  Serial.println("w");
  Serial.println(new_w);
  float W_left  = (2*v - WHEELBASE * new_w) / (2 * WHEEL_RADIUS);
  float W_right = (2*v + WHEELBASE * new_w) / (2 * WHEEL_RADIUS);
  Serial.println(W_left);
  Serial.println(W_right);

  if (abs(W_left) > WHEEL_OMEGA_MAX) {
    Serial.println("left wheel max exceeded");
    float reduction = WHEEL_OMEGA_MAX / abs(W_left); // to maintain ratio between left:right and v:w
    W_left = sign(W_left) * WHEEL_OMEGA_MAX;
    W_right = W_right * reduction;
  } if (abs(W_right) > WHEEL_OMEGA_MAX) {
    Serial.println("right wheel max exceeded");
    float reduction = WHEEL_OMEGA_MAX  / abs(W_right);
    W_right = sign(W_left) * WHEEL_OMEGA_MAX;
    W_left = W_left * reduction;
  }

  int PWM_left  = int(PWM_STOP + adjustOffset(int(W_left), true));
  int PWM_right = int(PWM_STOP - adjustOffset(int(W_right), false));

  PWM result;
  result.left = PWM_left;
  result.right = PWM_right;
  return result;
}

int stickToV(int raw) {
  if (abs(raw) < DEADZONE) {
    return 0;
  }
  return map(raw, -128, 127, -BODY_VEL_MAX, BODY_VEL_MAX);
}

int stickToW(int raw) {
  if (abs(raw) < DEADZONE) {
    return 0;
  }
  return map(raw, -128, 127, BODY_OMEGA_MAX * 180 / PI, -BODY_OMEGA_MAX * 180 / PI);
}

int adjustOffset(int raw, bool isLeft) {
  float percent_offset = float(map(raw, -WHEEL_OMEGA_MAX, WHEEL_OMEGA_MAX, -1000, 1000)) / 1000;
  //if (isLeft){
  //  Serial.println("left");
  //} else {
  //  Serial.println("right");
  //}
  //Serial.println(percent_offset);
  float scale = 0;
  for (int i = 0; i < 4; i++) {
    if ((OFFSETS[i] <= abs(percent_offset)) & (abs(percent_offset) <= OFFSETS[i+1])) { 
      float x = (abs(percent_offset) - OFFSETS[i]) / 0.25; 
      if (isLeft) {
        scale = SCALE_L[i] + x * (SCALE_L[i+1] - SCALE_L[i]);
        //Serial.println("left");
        //Serial.println(SCALE_L[i+1]);
        //Serial.println(x);
        //Serial.println(SCALE_L[i]);
      }
      else {
        scale = SCALE_R[i] + x * (SCALE_R[i+1] - SCALE_R[i]);
        //Serial.println("right");
        //Serial.println(SCALE_R[i+1]);
        //Serial.println(x);
        //Serial.println(SCALE_R[i]);
      }
    }
  }
  if (raw > 0) { //positive
    return int((PWM_MAX - PWM_STOP) * scale);
  }
  else {
    return int((PWM_MIN - PWM_STOP) * scale);
  }
}

int stickToAngle(int raw) {
  return map(raw, -128, 127, 0, 180);
}

void setup() {
  Serial.begin(115200);

  // allocate three hardware PWM timers (required by ESP32Servo)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);

  // 50 Hz refresh rate, attach servos with custom pulse bounds
  leftServo.setPeriodHertz(50);
  rightServo.setPeriodHertz(50);
  grabServo.setPeriodHertz(50);
  leftServo.attach(LEFT_SERVO_PIN,  PWM_MIN, PWM_MAX);
  rightServo.attach(RIGHT_SERVO_PIN, PWM_MIN, PWM_MAX);
  grabServo.attach(GRAB_SERVO_PIN);

  // start PS3 interface (already paired via SixAxisPairTool)
  Ps3.begin("40:f5:20:45:22:8a");
}

void loop() {
  // if no PS3 pad is connected, just stop and return
  if (!Ps3.isConnected()) {
    leftServo.writeMicroseconds(PWM_STOP);
    rightServo.writeMicroseconds(PWM_STOP);
    delay(20);
    return;
  }

  // read left-stick Y (throttle) and right-stick X (steering)
  int ly = -Ps3.data.analog.stick.ly;  
  int lx = Ps3.data.analog.stick.lx;  

  int ry = Ps3.data.analog.stick.ry;
  int rx = Ps3.data.analog.stick.rx;
  //Serial.println(ly);
  //Serial.println(rx);

  // convert to –1000…+1000, apply deadzone
  int vel = stickToV(ly);
  int omega  = stickToW(rx);

  // mix forward/back + turn for tank drive

  struct PWM result = velocitiesToPWM(vel, omega);


  // command servos
  setMotorPWM(result.left, result.right);
  grabServo.write(stickToAngle(ry));

  delay(20);
}
