
# Arduino Motor Control

This repository contains three Arduino sketches, designed to work with a differential drive mobile robot. The robots have an ESP32 Feather microcontroller, 2 MG90S Micro Servos (continuous), and a battery:

- **get_MAC_address.ino**
- **servo_characterization.ino**
- **motor_controller.ino**

## File Descriptions

### get_MAC_address.ino

Reads the MAC address of the ESP32 Feather and prints it to the serial monitor.

**Usage:** Copy MAC address to main.py in the __ repository

### servo_characterization.ino

Sweeps from 0-100% PWM in 25% intervals for user to measure corresponding servo angular velocities.

**Usage:** Produce PWM-velocity mapping for usage in /servo_normalization.xlsx

### motor_controller.ino

Processes velocity commands recieved via BLE, converting them into PWM signals for the servos.

**Configuration:**
- WHEELBASE: distance between wheels in millimeters
- WHEEL_RADIUS: radius of wheels in millimeters  
- WHEEL_VEL_MAX: max velocity of wheels in millimeters/second (choose lower max)  

**Features:**

- Uses `analogWrite()` for speed control  
- Direction toggled via digital pin  
- Simple and adaptable for many motor driver circuits

**Pin assignments:**

- `DIR_PIN` (e.g. D4): motor direction  
- `PWM_PIN` (e.g. D5): speed control

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
