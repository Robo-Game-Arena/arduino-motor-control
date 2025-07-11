
# Arduino Motor Control

This repository contains three Arduino sketches and one Excel file, designed to work with a differential drive mobile robot. The robots have an ESP32 Feather microcontroller, 2 MG90S Micro Servos (continuous), and a battery:

1. **get_MAC_address.ino**
2. **servo_characterization.ino**
3. **servo_normalization.xlsx**
4. **motor_controller.ino**

When starting from scratch, these files should be used in order.

## File Descriptions

### <ins> get_MAC_address.ino </ins>

  Reads the MAC address of the ESP32 Feather and prints it to the serial monitor.

  **Usage:** Copy MAC address to main.py in the __ repository

### <ins> servo_characterization.ino </ins>

  Sweeps from 0-100% PWM in 25% intervals to produce PWM-velocity mapping.

  **Usage:**  Measure corresponding servo angular velocities and copy into servo_normalization.xlsx

### <ins> servo_normalization.xlsx </ins>

  Given non-linear PWM-velocity mapping, linearizes velocity curve and produces corresponding PWM values.

  **Usage:** After updating angular velocity values, copy output into motor_controller.ino

### <ins> motor_controller.ino </ins>

  Processes velocity commands recieved via BLE, converting them into PWM signals for the servos.

  **Configuration:**
  - WHEELBASE: distance between wheels in millimeters
  - WHEEL_RADIUS: radius of wheels in millimeters  
  - WHEEL_VEL_MAX: max velocity of wheels in millimeters/second (choose lower max)
  - SCALE_L[5], SCALE_R[5]: PWM-velocity mapping from servo_normalization.xlsx
  - SERVICE_UUID: service UUID for the ESP32, can be found with nRF Connect app
  - CHARACTERISTIC_UUID: characteristic UUID for the ESP32, can be found with nRF Connect app

  **Usage:**
  - Receives linear and angular velocity values via BLE
  - Controls servos given adjusted PWM-velocity mapping to achieve given linear and angular velocity

## Requirements

- ESP32 Feather microcontroller
- 2x MG90S Micro Servo (continuous)
- 5V battery 

## License

This code is released as-is under the MIT License.  
Feel free to modify and use it for any purpose. Attribution appreciated.
