
Sony Spresense: Deploying TensorFlow Lite Models
================================================

Welcome to the **Sony Spresense TFLite Deployment Guide**! This repository provides a comprehensive, step-by-step guide to deploying TensorFlow Lite (TFLite) models on the Sony Spresense board. Whether you're setting up your development environment or deploying your first model, this guide will help you navigate the process with ease, referencing official documentation where necessary.

**Credits:** Special thanks to [@YoshinoTaro](https://github.com/YoshinoTaro) for developing the [Spresense Arduino Package for TensorFlow](https://github.com/YoshinoTaro/spresense-arduino-tensorflow).

* * *

Table of Contents
-----------------

1.  [Introduction](#introduction)
2.  [Hardware Overview](#hardware-overview)
3.  [Setting Up the Development Environment](#setting-up-the-development-environment)
    *   [Prerequisites](#prerequisites)
    *   [Step\-by\-Step Setup](#step-by-step-setup)
4.  [Installing and Setting Up Spresense SDK](#installing-and-setting-up-spresense-sdk)
    *   [Download Firmware Binaries](#download-firmware-binaries)
    *   [Clone the Spresense SDK](#clone-the-spresense-sdk)
    *   [Set Up Environment Variables](#set-up-environment-variables)
    *   [Flash the Bootloader](#flash-the-bootloader)
    *   [Verify SDK Setup](#verify-sdk-setup)
    *   [Configure the SDK](#configure-the-sdk)
    *   [Verify Serial Connection and NuttShell](#verify-serial-connection-and-nuttshell)
5.  [Setting Up Arduino IDE for TensorFlow Lite](#setting-up-arduino-ide-for-tensorflow-lite)
6.  [Deploying TensorFlow Lite Models on Spresense](#deploying-tensorflow-lite-models-on-spresense)
    *   [Preparing the Model](#preparing-the-model)
    *   [Verify the Converted Model](#verify-the-converted-model)
    *   [Convert TFLite Model to C Header File](#convert-tflite-model-to-c-header-file)
    *   [Integrate the Model into Arduino Sketch](#integrate-the-model-into-arduino-sketch)
    *   [Example Sketch Code](#example-sketch-code)
7.  [Flashing the Model and Running Inference](#flashing-the-model-and-running-inference)
    *   [Configure Memory Sizes](#1-configure-memory-sizes)
    *   [Compile and Upload the Sketch](#2-compile-and-upload-the-sketch)
    *   [Monitor Serial Output](#3-monitor-serial-output)
8.  [Notes on Optimization and Memory Usage](#notes-on-optimization-and-memory-usage)
9.  [References](#references)

* * *

Introduction
------------

This repository serves as a **Getting Started Guide** for deploying TensorFlow Lite models on the Sony Spresense board. The primary focus is on deployment, but it also covers setting up the user's computer and ensuring proper usage of the board. The guide leverages [TFLite for Microcontrollers (TFLite Micro)](https://ai.google.dev/edge/litert/microcontrollers/get_started) and utilizes C++ to run inference on the device using the TFLite Micro library and process the results.

* * *

Hardware Overview
-----------------

### Sony Spresense Board

*   **Processor:** 6-Core ARM Cortex-M4F
*   **Flash Memory:** 8 MB
*   **SRAM:** 1.5 MB
*   **Expansion:** Supports external microSD cards via the optional extension board

Memory Constraints and Usage
-----------------
### SRAM (1.5 MB)

* Used for runtime data, including variables, stack, heap, and dynamically allocated memory.
* During inference, weights and activations are stored in SRAM temporarily.
* Limited capacity requires careful optimization of model size to prevent memory overflow.

### Flash Memory (8 MB)

* Used for storing the bootloader, firmware, applications, and other persistent data.
* Model weights are stored in flash memory before being loaded into SRAM for inference.
* Effective utilization of this space is crucial for deploying larger models within the board's constraints.

### External Micro SD Card

* Supports storage of auxiliary data, such as images, CSV files, or other non-model-related content.
* Cannot be used to store model weights or binaries for inference.

Understanding the hardware specifications is crucial for optimizing model deployment, especially regarding memory usage. The Spresense board's flash memory and RAM limitations will dictate the size and complexity of the models you can deploy.

**SDK Compatibility:** The Spresense Arduino Package for TensorFlow is based on **SDK v2.5.0**, ensuring compatibility. Later SDK versions are not supported with this package.

* * *

Setting Up the Development Environment
--------------------------------------

### Prerequisites

*   **Operating System:** macOS (steps provided; refer to official documentation for other OS)
*   **Python:** Recommended version 3.10.14
*   **Package Manager:** Conda or similar for virtual environments
*   **Xcode Tools:** Required for macOS

### Step-by-Step Setup

1. **Install USB-to-Serial Drivers**
   - **macOS**:
     - [CP210x USB to Serial Driver for Mac OS X](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - **Installation**:
     - Users must install the drivers **before** connecting the Spresense board to the PC.
       
2.  **Install Python and Create Virtual Environment**
    
    ```bash
    # Install Python (if not already installed)
    # Recommended version: Python 3.10.14
    
    # Create a virtual environment using Conda
    conda create -n spresense_env python=3.10.14
    conda activate spresense_env
    ```
    
3.  **Install Xcode Tools (macOS Only)** Open Terminal and run:
    
    ```bash
    xcode-select --install
    ```
    
4.  **Install Required Tools**
    
    *   **Install Homebrew** (if not already installed):
        
        ```bash
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
        ```
        
    *   **Install wget:**
        
        ```bash
        brew install wget
        ```
        
    *   **Install flock** (necessary for SDK v3.2.0 or later):
        
        ```bash
        brew tap discoteq/discoteq
        brew install flock
        ```
        
5.  **Install Development Tools**
    
    ```bash
    curl -L https://raw.githubusercontent.com/sonydevworld/spresense/master/install-tools.sh > install-tools.sh
    bash install-tools.sh
    ```
    
    **Note for macOS Users:** Ensure you are using `bash` instead of `zsh` for the shell during installation.
    
    *   **Switch to bash:**
        
        ```bash
        exec bash
        ```
        
    *   **Activate Installed Tools:**
        
        ```bash
        source ~/.bash_profile    # For bash
        source ~/spresenseenv/setup
        ```
        

* * *

Installing and Setting Up Spresense SDK
---------------------------------------

### Download Firmware Binaries

Before cloning the SDK, download the required firmware binaries:

*   **Firmware v2.4.0 Download:** [Download Spresense Firmware v2.4.0](https://developer.sony.com/file/download/download-spresense-firmware-v2-4-000)

### Clone the Spresense SDK

1.  **Clone the Repository with Submodules:**
    
    ```bash
    git clone --recursive https://github.com/sonydevworld/spresense.git
    cd spresense
    ```
    
2.  **Checkout Specific SDK Version:**
    
    ```bash
    git checkout v2.5.0
    git submodule init
    git submodule update
    ```
    
    *   **SDK Version:** 2.5.0
    *   **Bootloader Version:** v2.4.0

### Set Up Environment Variables

Ensure the correct paths are set for `SPRESENSE_SDK` and `SPRESENSE_HOME`. Add the following lines to your `~/.bash_profile` (or equivalent for your shell):

```bash
source ~/spresenseenv/setup
export SPRESENSE_SDK=/Users/<your-username>/spresense
export SPRESENSE_HOME=/Users/<your-username>/myapps
```

Replace `<your-username>` with your actual username.

Then activate the changes:

```bash
source ~/.bash_profile
```

### Flash the Bootloader

1.  **Flash the Bootloader (v2.4.0):**
    
    ```bash
    ./tools/flash.sh -e /path/to/spresense-binaries-v2.4.0.zip
    ./tools/flash.sh -l /path/to/spresense/firmware/spresense -c /dev/cu.SLAB_USBtoUART
    ```
    
    **Note:** Replace `/path/to/` with the actual path to your downloaded binaries.

### Verify SDK Setup

1.  **Navigate to SDK Directory and Source Setup Scripts:**
    
    ```bash
    cd spresense/sdk
    source ~/spresenseenv/setup
    source tools/build-env.sh
    ```
    
    You should see output similar to:
    
    ```makefile
    =======================================
    SDK_VERSION = SDK2.5.0
    NUTTX_VERSION = 10.2.0
    SPRESENSE_SDK = /Users/<your-username>/spresense
    SPRESENSE_HOME = /Users/<your-username>/myapps
    GCC_VERSION = arm-none-eabi-gcc (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1 20210824 (release)
    HOST = Darwin x86_64
    =======================================
    ```
    
    Ensure that `SPRESENSE_SDK` and `SPRESENSE_HOME` paths are correctly set.

    *   **Directory Structure:**
    
    *   Your `spresense` directory should contain the `sdk` folder, along with `spresense_env.sh`, `install-tools.sh`, and other files like `examples` and `nuttx`.
    *   Example:
        
        ```javascript
        /Users/<your-username>/spresense/
        ├── sdk/
        ├── spresense_env.sh
        ├── install-tools.sh
        ├── examples/
        └── nuttx/
        ```
        
*   **Application Root:**
    
    *   Refer to the [Spresense SDK Documentation](https://developer.sony.com/spresense/development-guides/sdk_set_up_en.html#_spr_create_approot) for setting the application root, typically at `/home/user/myapps`.
 

### Configure the SDK

1. **Set Initial Configuration:**

    ```bash
    cd spresense/sdk
    tools/config.py examples/hello
    ```

2. **Handling Bootloader Warnings:**  
   If you encounter a message like:

    ```vbnet
    WARNING: New loader vX.Y.Z is required, please download and install.
    ```

   - **Solution:** Flash the new bootloader.
   
    ```bash
    ./tools/flash.sh -e /path/to/spresense-binaries-v2.4.0.zip
    ./tools/flash.sh -l /path/to/spresense/firmware/spresense -c /dev/cu.SLAB_USBtoUART
    ```

3. **Build the Example Image:**

    ```bash
    make
    ```

   After running the `make` command, a `nuttx.spk` file will be created in the `sdk` folder. This file is the final build artifact that can be flashed onto the Spresense board.

4. **Flash the Example to the Board:**

    Use the `tools/flash.sh` script to flash the `nuttx.spk` file onto the board.

    ```bash
    tools/flash.sh -c /dev/cu.SLAB_USBtoUART nuttx.spk
    ```

   *Note:* Replace `/dev/cu.SLAB_USBtoUART` with your board's serial port if it differs.

5. **Reboot and Verify:**

   After flashing, the Spresense board will reboot automatically.

---

### Verify Serial Connection and NuttShell

1. **Identify the Serial Port:**  
   On macOS, the serial port for Spresense is typically `/dev/cu.SLAB_USBtoUART`. To confirm the correct port, run:

    ```bash
    ls /dev/{tty,cu}.*
    ```

   Look for `/dev/cu.SLAB_USBtoUART` in the output.

2. **Install `screen` (if not already installed):**

    ```bash
    brew install screen
    ```

3. **Connect to the Serial Port:**

    Open a serial monitor with a baud rate of 115200:

    ```bash
    screen /dev/cu.SLAB_USBtoUART 115200
    ```

4. **Run the Hello World Example:**

    At the `nsh>` prompt, type:

    ```bash
    nsh> hello
    ```

    You should see:

    ```plaintext
    Hello, World!!
    ```

5. **Exit `screen`:**

   - Press `Ctrl + A`, then `K` to kill the session.
   - Confirm by typing `Y`.

---

Once you see the `Hello, World!!` output, your environment is correctly set up, and the SDK configuration is complete.


* * *

Setting Up Arduino IDE for TensorFlow Lite
------------------------------------------

### 1\. Install Arduino IDE

*   **Download:** Arduino IDE
*   Install the IDE appropriate for your operating system.

### 2\. Install USB-to-Serial Driver

*   Ensure the CP210x USB-to-Serial driver is installed as per the [Setting Up the Development Environment](#setting-up-the-development-environment) section.

### 3\. Install Spresense TensorFlow Board Package

1.  **Open Arduino IDE and Navigate to Preferences:**
    
    *   **Path:** `Arduino IDE > Preferences`
2.  **Add Additional Boards Manager URL:**
    
    ```plaintext
    https://raw.githubusercontent.com/YoshinoTaro/spresense-arduino-tensorflow/main/package_spresense_tensorflow_index.json
    ```
    
3.  **Open Board Manager:**
    
    *   **Path:** `Tools > Board > Boards Manager`
4.  **Search and Install "Spresense Tensorflow Board"**
    

### 4\. Select the Spresense Tensorflow Board

*   **Path:** `Tools > Board > Spresense Tensorflow`

### 5\. Compile and Run the Hello World Example

*   Refer to [YoshinoTaro's GitHub Repository](https://github.com/YoshinoTaro/spresense-arduino-tensorflow/tree/main) for example sketches.
*   Successful installation is confirmed when you see a sine curve on the plotter.

* * *

Deploying TensorFlow Lite Models on Spresense
---------------------------------------------

### Preparing the Model

Convert your trained TensorFlow model to TFLite format. Refer to [Post\-Training Integer Quantization](https://ai.google.dev/edge/litert/models/post_training_integer_quant#convert_to_a_litert_model) for detailed instructions.

#### 1\. Convert to TFLite (No Quantization)

```python
import tensorflow as tf

converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save the model to disk
with open('model.tflite', 'wb') as f:
    f.write(tflite_model)
```

#### 2\. Post-Training Dynamic Range Quantization

```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model_quant = converter.convert()

with open('model_dynamic_quant.tflite', 'wb') as f:
    f.write(tflite_model_quant)
```

#### 3\. Full INT8 Quantization (with Representative Dataset)

**Representative Dataset Example:**

```python
train_generator_qat = train_datagen_qat.flow_from_dataframe(
    dataframe=train_data_qat,
    x_col='file_path',
    y_col='label',
    target_size=(img_width, img_height),
    batch_size=batch_size,
    class_mode='categorical',
    shuffle=True,
    seed=42
)

def representative_data_gen():
    # Number of samples you want to use for calibration
    num_samples = 100
    count = 0

    for input_value, _ in train_generator_qat:
        yield [input_value]
        count += 1
        if count >= num_samples:
            break
```

**Conversion Script:**

```python
converter = tf.lite.TFLiteConverter.from_keras_model(qat_model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
converter.representative_dataset = representative_data_gen

# Convert and save the model
tflite_model = converter.convert()
with open("model_int8.tflite", "wb") as f:
    f.write(tflite_model)
```

### Verify the Converted Model

```python
import tensorflow as tf

# Load the TensorFlow Lite model
interpreter = tf.lite.Interpreter(model_path="model_int8.tflite")
interpreter.allocate_tensors()

# Get input and output details
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Display Input Details
print("Input Details with Quantization Parameters:")
for input_detail in input_details:
    print(f"  Name: {input_detail['name']}")
    print(f"  Shape: {input_detail['shape']}")
    print(f"  Data Type: {input_detail['dtype']}")
    print(f"  Quantization Parameters: {input_detail['quantization']}")
    print(f"  Quantization Scale: {input_detail['quantization_parameters']['scales']}")
    print(f"  Quantization Zero Points: {input_detail['quantization_parameters']['zero_points']}\n")

# Display Output Details
print("\nOutput Details with Quantization Parameters:")
for output_detail in output_details:
    print(f"  Name: {output_detail['name']}")
    print(f"  Shape: {output_detail['shape']}")
    print(f"  Data Type: {output_detail['dtype']}")
    print(f"  Quantization Parameters: {output_detail['quantization']}")
    print(f"  Quantization Scale: {output_detail['quantization_parameters']['scales']}")
    print(f"  Quantization Zero Points: {output_detail['quantization_parameters']['zero_points']}\n")
```

**Example Output:**

```yaml
Input Details with Quantization Parameters:
  Name: serving_default_input_7:0
  Shape: [1, 128, 128, 3]
  Data Type: <class 'numpy.int8'>
  Quantization Parameters: (0.007843137718737125, 0)
  Quantization Scale: [0.00784314]
  Quantization Zero Points: [0]

Output Details with Quantization Parameters:
  Name: StatefulPartitionedCall:0
  Shape: [1, 3]
  Data Type: <class 'numpy.int8'>
  Quantization Parameters: (0.00390625, -128)
  Quantization Scale: [0.00390625]
  Quantization Zero Points: [-128]
```

**Model Optimization Notes:**

*   The Sony Spresense board features an ARM Cortex-M4F, which accelerates INT8 models using the CMSIS\-NN library.
*   **Recommendation:** Use INT8 models for optimal speed.
*   **Constraints:** The board does not support mixed precision or mixed data types. Use either full FLOAT32 or full INT8 models.
    *   **Alternative:** You can set `converter.inference_output_type = tf.float32` to retain full precision at the output while keeping activations and weights as 8-bit.

### Convert TFLite Model to C Header File

**Conversion Script:**

```python
import binascii

def convert_to_c_array(bytes_data) -> str:
    hexstr = binascii.hexlify(bytes_data).decode("UTF-8").upper()
    array = ["0x" + hexstr[i:i + 2] for i in range(0, len(hexstr), 2)]
    array = [array[i:i+10] for i in range(0, len(array), 10)]
    return ",\n  ".join([", ".join(e) for e in array])

# Read the TFLite model
with open('model_int8.tflite', 'rb') as f:
    tflite_binary = f.read()

ascii_bytes = convert_to_c_array(tflite_binary)
header_file_content = (
    "const unsigned char model_tflite[] = {\n  " 
    + ascii_bytes 
    + "\n};\nunsigned int model_tflite_len = " 
    + str(len(tflite_binary)) + ";"
)

# Write to .h file
with open("model.h", "w") as f:
    f.write(header_file_content)
```

**Ensure:** The `.h` file size does not exceed **2.1 MB**.

### Integrate the Model into Arduino Sketch

1.  **Create a New Arduino Sketch:**
    
    *   Save the sketch in a dedicated folder.
2.  **Add the `.h` File:**
    
    *   Place `model.h` in the same folder as the `.ino` file.
3.  **Include the Model in Your Sketch:**
    
    ```cpp
    #include "model.h"
    ```
    

### Example Sketch Code
This example sketch is based on the [TensorFlow Lite for Microcontrollers Get Started](https://ai.google.dev/edge/litert/microcontrollers/get_started) guide.

```cpp
// TensorFlow Lite Micro includes
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"  // Your model file after conversion

// Globals for TensorFlow Lite Micro
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  int inference_count = 0;
  
  // Define the size of the tensor arena (adjust as needed)
  constexpr int kTensorArenaSize = 300 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
} 

// Function to set up the TensorFlow Lite Micro model
void setupModel() {
  // Initialize TensorFlow Lite Micro target
  tflite::InitializeTarget();

  // Initialize the tensor arena to zero
  memset(tensor_arena, 0, sizeof(tensor_arena));

  // Set up the error reporter
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Load the TensorFlow Lite model
  model = tflite::GetModel(model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version mismatch.");
    while (1); // Halt if version mismatch
  }

  // Create an operator resolver
  static tflite::AllOpsResolver resolver;

  // Create the interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory for the tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed.");
    while (1); // Halt if allocation fails
  }

  // Obtain pointers to the model's input and output tensors
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Get information about the memory area to use for the model's input
  Serial.println("Model input:");
  Serial.println("Number of dimensions: " + String(input->dims->size));
  for (int n = 0; n < input->dims->size; ++n)
  {
    Serial.println("dims->data[" + String(n) + "]: " + String(input->dims->data[n]));
  }
  Serial.print("Input type: ");
  Serial.println(input->type);

  Serial.println("\nModel output:");
  Serial.println("dims->size: " + String(output->dims->size));
  for (int n = 0; n < output->dims->size; ++n)
  {
    Serial.println("dims->data[" + String(n) + "]: " + String(output->dims->data[n]));
  }
  Serial.print("Output type: ");
  Serial.println(output->type);

  Serial.println("Completed TensorFlow setup");
  Serial.println();
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) { /* Wait for Serial to initialize */ }

  // Set up the TensorFlow Lite Micro model
  setupModel();
}

void loop() {
  // Placeholder for your main code
  // You can add inference calls or other logic here

  // For example, a simple delay to prevent the loop from running too fast
  delay(1000);
}
```

### Notes

*   **Memory Allocation:** Adjust `kTensorArenaSize` based on your model's memory requirements. Monitor memory usage with `interpreter->arena_used_bytes()`.

* * *

Flashing the Model and Running Inference
----------------------------------------

### 1\. Configure Memory Sizes

Refer to the [Spresense Arduino Developer Guide](https://developer.sony.com/spresense/development-guides/arduino_developer_guide_en.html#_tutorial_memory_config) to select appropriate memory configurations.

*   **Default Memory Configuration:**
    
    *   **MainCore:** 768 KB (first six tiles)
    *   **Shared Memory:** 768 KB (remaining six tiles) for SubCore, Audio DSP, and other libraries
*   **Adjusting Memory:**
    
    *   **Increase MainCore Memory:** Up to 1.5 MB (1536 KB) if not using SubCore or Audio DSP.
    *   **Reduce MainCore Memory:** Allocate more memory to SubCore and Audio DSP based on your use case.

**Memory Configuration in Arduino IDE:**

*   **Path:** `Arduino IDE > Tools > Memory Configuration`
*   Adjust the memory sizes according to your application's needs.

### 2\. Compile and Upload the Sketch

1.  **Open Arduino IDE.**
2.  **Select the Spresense Tensorflow Board:**
    *   `Tools > Board > Spresense Tensorflow`
3.  **Compile the Sketch:**
    *   Click the **Verify** button.
4.  **Upload the Sketch:**
    *   Click the **Upload** button.

### 3\. Monitor Serial Output

*   **Open Serial Monitor:**
    
    *   `Tools > Serial Monitor`
    *   Set baud rate to **115200**
*   **Verify Inference Results:**
    
    *   You should see inference outputs similar to:
        
        ```csharp
         AllocateTensors() Success
         Arena used bytes: 15000
         Model input:
         Number of dimensions: 4
         dims->data[0]: 1
         dims->data[1]: 28
         dims->data[2]: 28
         dims->data[3]: 1
         Input type: 1
         
         Model output:
         Number of dimensions: 2
         dims->data[0]: 1
         dims->data[1]: 10
         Output type: 1
         
         Completed TensorFlow setup
        ...
        ```

       *  The `input->type` and `output->type` fields represent the data types of the tensors used by the TensorFlow Lite model. Below is a list of the possible data types and their corresponding integer values:
          
             *   `KTfLiteNoType` (0): Undefined or unspecified type.
             *   `kTfLiteFloat32` (1): 32-bit floating-point numbers.
             *   `kTfLiteInt32` (2): 32-bit signed integers.
             *   `kTfLiteUInt8` (3): 8-bit unsigned integers.
             *   `KTfLiteInt64` (4): 64-bit signed integers.
             *   `KTfLiteString` (5): String data type.
             *   `KTfLiteBool` (6): Boolean values (`true` or `false`).
             *   `KTfLiteInt16` (7): 16-bit signed integers.
             *   `KTfLiteComplex64` (8): Complex numbers with 64-bit precision.
             *   `KTfLiteInt8` (9): 8-bit signed integers.
             *   `kTfLiteFloat16` (10): 16-bit floating-point numbers.

*   **Performance Metrics:**
    
    *   Refer to repository files for scripts that run multiple inferences (e.g., 1000 inferences) to calculate average inference time and frames per second (FPS).

**Exiting Serial Monitor:**

*   To stop the serial monitor, simply close the Serial Monitor window in Arduino IDE.

* * *

Notes on Optimization and Memory Usage
--------------------------------------

*   **Use INT8 Models:**
    
    *   The [CMSIS\-NN](https://github.com/ARM-software/CMSIS-NN) library accelerates INT8 models for the ARM Cortex-M4F processor.
    *   **Recommendation:** Use INT8 models for optimal speed.
    *   **Constraints:** The board does not support mixed precision or mixed data types. Use either full FLOAT32 or full INT8 models.
        *   **Alternative:** You can set `converter.inference_output_type = tf.float32` to retain full precision at the output while keeping activations and weights as 8-bit.
*   **Adjust Memory Configuration:**
    
    *   **MainCore Default Memory:** 768 KB
    *   **Maximum Memory for MainCore:** 1.5 MB (if SubCore and Audio DSP are disabled).
*   **Model Size:**
    
    *   Ensure the `.h` file size does not exceed **2.1 MB**.

* * *

References
----------

*   [Sony Spresense Official Documentation](https://developer.sony.com/spresense/)
*   [Spresense SDK Getting Started Guide (CLI)](https://developer.sony.com/spresense/development-guides/sdk_set_up_en.html)
*   [Spresense Arduino Developer Guide](https://developer.sony.com/spresense/development-guides/arduino_developer_guide_en.html)
*   [YoshinoTaro's Spresense Arduino TensorFlow Repository](https://github.com/YoshinoTaro/spresense-arduino-tensorflow)
*   [TensorFlow Lite for Microcontrollers](https://ai.google.dev/edge/litert/microcontrollers/overview)
*   [CMSIS\-NN Library](https://github.com/ARM-software/CMSIS-NN)
*   [Post\-Training Integer Quantization](https://ai.google.dev/edge/litert/models/post_training_integer_quant#convert_to_a_litert_model)
*   [Download Spresense Firmware v2.4.0](https://developer.sony.com/file/download/download-spresense-firmware-v2-4-000)

* * *

Thank you for using this guide! I hope it helps you successfully deploy your TensorFlow Lite models on the Sony Spresense board.
