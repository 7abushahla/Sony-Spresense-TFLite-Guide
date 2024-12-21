# SD_CSV_TFLM_Inference

This example demonstrates how to load a CSV file from an SD card, select random samples, and perform inference using TensorFlow Lite Micro.

## Features
- Reads floating-point data from a CSV file on an SD card.
- Selects random samples for inference.
- Measures and reports inference times.

## Prerequisites
- Arduino IDE installed.
- TensorFlow Lite Micro library.
- SD card module connected to the Arduino.
- Properly formatted CSV file uploaded to the SD card.

## Setup Instructions
1. Connect the SD card module to your Arduino board.
2. Upload the `SD_CSV_TFLM_Inference.ino` sketch to your Arduino.
3. Open the Serial Monitor (Tools > Serial Monitor) and set the baud rate to **115200**.
4. Reset the Arduino board to start the inference process.
