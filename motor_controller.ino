#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <ESP32Servo.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

#define SERVICE_UUID        "0000S001-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000C001-0000-1000-8000-00805f9b34fb"

// ——— Servo objects —————————————————————————————————————————————————————
Servo leftServo;
Servo rightServo;

// ——— Configuration ————————————————————————————————————————————————————
const float WHEELBASE = 130.0;         // distance between wheels in millimeters
const float WHEEL_RADIUS = 30.0;      //
const float WHEEL_VEL_MAX = 348.0;    // max wheel velocity in millimeters/sec
const float WHEEL_OMEGA_MAX = WHEEL_VEL_MAX / WHEEL_RADIUS;  // max wheel angular velocity in radians/sec
const float BODY_VEL_MAX = WHEEL_VEL_MAX; // given 0 turning
const float BODY_OMEGA_MAX = WHEEL_VEL_MAX / (WHEELBASE / 2) ; // given 0 displacement

// motor 
const float OFFSETS[5] = {0.000, 0.250, 0.500, 0.750, 1.000};
const float SCALE_L[5] = {0.000, 0.159, 0.324, 0.495, 1.000};
const float SCALE_R[5] = {0.000, 0.101, 0.201, 0.380, 0.624};

const int LEFT_SERVO_PIN  = 5;    // ESP32 Feather pin for left continuous‐rotation servo
const int RIGHT_SERVO_PIN = 4;    // ESP32 Feather pin for right continuous‐rotation servo

const int PWM_MIN    = 500;        // full reverse
const int PWM_STOP   = 1500;       // stop
const int PWM_MAX    = 2500;       // full forward

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
  //Serial.println("v");
  //Serial.println(v);
  //Serial.println("w");
  //Serial.println(new_w);
  float W_left  = (2*v - WHEELBASE * new_w) / (2 * WHEEL_RADIUS);
  float W_right = (2*v + WHEELBASE * new_w) / (2 * WHEEL_RADIUS);
  //Serial.println(W_left);
  //Serial.println(W_right);

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

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      setMotorPWM(PWM_STOP, PWM_STOP);

      Serial.println("BLE Disconnected! Restarting advertising...");
      // Restart advertising immediately after disconnect
      BLEDevice::startAdvertising(); 
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    String value = pCharacteristic->getValue(); 
    size_t len = pCharacteristic->getLength();

    if (len == 8) {
      int32_t v_mmps, w_degps;

      memcpy(&v_mmps, &value[0], 4);
      memcpy(&w_degps, &value[4], 4);
          
      Serial.printf("Received BLE V: %d mm/s, W: %d deg/s\n", v_mmps, w_degps);
      PWM result = velocitiesToPWM(v_mmps, w_degps);  

      Serial.printf("Sending PWM left: %d,  right: %d\n", result.left, result.right);
      setMotorPWM(result.left, result.right);
    } else {
        Serial.print("Received invalid BLE command length: ");
        Serial.println(value.length());
    }
  }
};

void setup() {
  Serial.begin(115200);

  ////Servo
  // allocate two hardware PWM timers (required by ESP32Servo)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  // 50 Hz refresh rate, attach servos with custom pulse bounds
  leftServo.setPeriodHertz(50);
  rightServo.setPeriodHertz(50);
  leftServo.attach(LEFT_SERVO_PIN,  PWM_MIN, PWM_MAX);
  rightServo.attach(RIGHT_SERVO_PIN, PWM_MIN, PWM_MAX);


  ////Bluetooth
  BLEDevice::init("ESP32_Feather_BLE_01");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);

  // Set the advertising interval
  // The values are in units of 0.625 ms.
  // Common short intervals:
  // 0x20 (32 * 0.625ms = 20ms) - Very fast, high power
  // 0x30 (48 * 0.625ms = 30ms)
  // 0xA0 (160 * 0.625ms = 100ms) - Good balance for responsiveness
  // 0x200 (512 * 0.625ms = 320ms) - Standard, moderate power

  pAdvertising->setMinPreferred(0x30); // 30 ms (48 * 0.625ms)
  pAdvertising->setMaxPreferred(0x50); // 50 ms (80 * 0.625ms) - Keeps a tight range

  BLEDevice::startAdvertising();
  Serial.println("BLE Server Started");
}

void loop() {
  delay(10);
}
