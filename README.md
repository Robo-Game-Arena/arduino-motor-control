
# Arduino Motor Control

This repository contains three Arduino sketches, designed to work with a differential drive mobile robot. The robots have an ESP32 Feather microcontroller, 2 MG90S Micro Servos (continuous), and a battery:

- **motor_controller.ino**
- **get_MAC_address.ino**
- **servo_characterization.ino**

## File Descriptions

### motor_controller.ino

Processes velocity commands recieved via BLE, converting them into PWM signals for diff

**Features:**

- Uses `analogWrite()` for speed control  
- Direction toggled via digital pin  
- Simple and adaptable for many motor driver circuits

**Pin assignments:**

- `DIR_PIN` (e.g. D4): motor direction  
- `PWM_PIN` (e.g. D5): speed control

### get_MAC_address.ino

Reads the MAC address of an ESP32 Feather and prints it to the serial monitor.

### servo_characterization.ino

Sweeps a servo from 0° to 180° and logs pulse widths for characterization.

**Features:**

- Uses `Servo.h` library  
- Serial output of angle and microsecond pulse  
- Helpful for mapping angles to PWM signals

**Use case:** Servo calibration and mechanical limit testing.

## How to Use

1. Open the `.ino` file you want in the Arduino IDE  
2. Connect the appropriate hardware:  
   - Ethernet module (W5500)  
   - Motor with H-bridge or driver  
   - Servo motor  
3. Upload the code to your board  
4. Open the Serial Monitor (9600 or 115200 baud) to observe output

## Requirements

- Arduino-compatible board (Uno, Nano, etc.)  
- Libraries:  
  - `Ethernet.h` for MAC address retrieval  
  - `Servo.h` for servo testing  
- Hardware:  
  - W5500 Ethernet module  
  - Brushed DC motor with driver  
  - Servo motor (standard PWM)

## License

This code is released as-is under the MIT License.  
Feel free to modify and use it for any purpose. Attribution appreciated.
