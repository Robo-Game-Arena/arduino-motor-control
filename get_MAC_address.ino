#include <BLEDevice.h>

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32_BLE_Demo");  // Name your device
  Serial.print("BLE MAC Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());

  // unplug and plug esp32 board
}

void loop() {}
