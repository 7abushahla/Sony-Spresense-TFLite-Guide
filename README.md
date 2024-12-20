
# Sony Spresense: Deploying TensorFlow Lite Models

This GitHub repository provides a detailed guide for deploying TensorFlow Lite (TFLite) models on the Sony Spresense board. It includes information about setting up the environment, using the Spresense Arduino Package for TensorFlow, and optimizing for resource-constrained deployment. This guide gives credit to **[@YoshinoTaro](https://github.com/YoshinoTaro)** for their Spresense Arduino Package for TensorFlow.

---

## General Outline

### 1. Understanding Sony Spresense Memory and Storage
- **Flash Memory and RAM**:
  - Usable Flash Memory: ~6 MB
  - Usable RAM: ~1 MB (subject to SDK and system configurations).
- **External microSD Card**:
  - Usage requires the optional extension board, expanding storage capabilities for applications.

Note: The Spresense Arduino Package for TensorFlow is based on **SDK v2.5.0**, and we specifically use this version as it ensures compatibility with the package. Later SDK versions are not supported with this package.

---

### 2. Setting Up the Development Environment

#### Prerequisites
1. **Install Required Tools**:
   - **Arduino IDE**: Download and install for your operating system.
   - **USB-to-Serial Drivers**:
     - [CP210x USB to serial driver for Windows 10/11](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
     - [CP210x USB to serial driver for Mac OS X](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).

2. **Install Spresense SDK (v2.5.0)**:
   Follow the official "[Spresense SDK Getting Started Guide (CLI)](https://developer.sony.com/spresense/development-guides/sdk_set_up_en.html)" with the following steps:

#### Installation for macOS
1. **Install Xcode Tools**:
   ```bash
   xcode-select --install
   ```

2. **Install Required Tools**:
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
   brew install wget
   brew tap discoteq/discoteq
   brew install flock
   curl -L https://raw.githubusercontent.com/sonydevworld/spresense/master/install-tools.sh > install-tools.sh
   bash install-tools.sh
   ```
   Use `bash` as the shell:
   ```bash
   exec bash
   ```

3. **Set Up the Environment**:
   Add the following lines to your `~/.bash_profile` (or equivalent for your shell):
   ```bash
   source ~/spresenseenv/setup
   export SPRESENSE_SDK=/Users/<your-username>/spresense
   export SPRESENSE_HOME=/Users/<your-username>/myapps
   ```
   Then activate the changes:
   ```bash
   source ~/.bash_profile
   ```

4. **Clone Spresense SDK (v2.5.0)**:
   ```bash
   git clone --recursive https://github.com/sonydevworld/spresense.git
   cd spresense
   git checkout v2.5.0
   git submodule init
   git submodule update
   ```

5. **Flash the Bootloader (v2.4.0)**:
   ```bash
   ./tools/flash.sh -e /path/to/spresense-binaries-v2.4.0.zip
   ./tools/flash.sh -l /path/to/firmware/spresense -c /dev/cu.SLAB_USBtoUART
   ```

6. **Verify SDK Setup**:
   ```bash
   cd spresense/sdk
   source ~/spresenseenv/setup
   source tools/build-env.sh
   ```
   You should see output confirming the SDK version:
   ```
   =======================================
      SDK_VERSION = SDK2.5.0
    NUTTX_VERSION = 10.2.0
   SPRESENSE_SDK = /Users/<your-username>/spresense
   SPRESENSE_HOME = /Users/<your-username>/myapps
      GCC_VERSION = arm-none-eabi-gcc (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1 20210824 (release)
             HOST = Darwin x86_64
   =======================================
   ```

To test the environment setup, build and run the Hello World example as described in the [official documentation](https://developer.sony.com/spresense/development-guides/sdk_set_up_en.html#_example_application_of_hello_world).

---

### 3. Installing the Spresense Arduino Package for TensorFlow

1. **Install the Package**:
   1. Open "Preferences" in Arduino IDE and add the URL:
      ```
      https://raw.githubusercontent.com/YoshinoTaro/spresense-arduino-tensorflow/main/package_spresense_tensorflow_index.json
      ```
   2. Open "Boards Manager" and install "Spresense Tensorflow Board."
   3. Select "Spresense Tensorflow" in the board selection.

2. **Verify Installation**:
   Compile and run the Hello World example provided in the Spresense Arduino Package for TensorFlow repository to ensure the setup is complete.

---

### 4. Deploying TensorFlow Lite Models

#### Preparing the Model
1. Convert the trained TensorFlow model to TFLite format:
   - **No Quantization**:
     ```python
     converter = tf.lite.TFLiteConverter.from_keras_model(model)
     tflite_model = converter.convert()
     with open("model.tflite", "wb") as f:
         f.write(tflite_model)
     ```
   - **Dynamic Range Quantization**:
     ```python
     converter.optimizations = [tf.lite.Optimize.DEFAULT]
     tflite_model_quant = converter.convert()
     ```
   - **Full INT8 Quantization** (with representative dataset):
     ```python
     converter.optimizations = [tf.lite.Optimize.DEFAULT]
     converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
     converter.inference_input_type = tf.int8
     converter.inference_output_type = tf.int8
     converter.representative_dataset = representative_data_gen
     tflite_model = converter.convert()
     with open("quantized_model.tflite", "wb") as f:
         f.write(tflite_model)
     ```

#### Verifying Quantization Details
To inspect the quantization details of the input and output tensors:
```python
# Load the TensorFlow Lite model
interpreter = tf.lite.Interpreter(model_path="quantized_model.tflite")
interpreter.allocate_tensors()

# Get input and output details
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Display input details including quantization parameters
print("Input Details with Quantization Parameters:")
for input_detail in input_details:
    print(f"  Name: {input_detail['name']}")
    print(f"  Shape: {input_detail['shape']}")
    print(f"  Data Type: {input_detail['dtype']}")
    print(f"  Quantization Parameters: {input_detail['quantization']}")
    print()

# Display output details including quantization parameters
print("
Output Details with Quantization Parameters:")
for output_detail in output_details:
    print(f"  Name: {output_detail['name']}")
    print(f"  Shape: {output_detail['shape']}")
    print(f"  Data Type: {output_detail['dtype']}")
    print(f"  Quantization Parameters: {output_detail['quantization']}")
    print()
```

#### Converting Model to C Header File
Convert the TFLite model to a C-style header file for deployment:
```python
import binascii
def convert_to_c_array(bytes) -> str:
    hexstr = binascii.hexlify(bytes).decode("UTF-8").upper()
    array = ["0x" + hexstr[i:i + 2] for i in range(0, len(hexstr), 2)]
    array = [array[i:i+10] for i in range(0, len(array), 10)]
    return ",
  ".join([", ".join(e) for e in array])

tflite_binary = open('quantized_model.tflite', 'rb').read()
ascii_bytes = convert_to_c_array(tflite_binary)
header_file = f"const unsigned char model_tflite[] = {{
  {ascii_bytes}
}};
unsigned int model_tflite_len = {len(tflite_binary)};"

with open("model.h", "w") as f:
    f.write(header_file)
```

#### Running the Model on Spresense
1. Include the generated `.h` file in your Arduino sketch.
2. Set an appropriate `kTensorArenaSize` value.
3. Compile and upload the sketch.
4. Verify inference results via the serial monitor.

---

### 5. Notes on Optimization and Memory Usage

- **Use INT8 Models**:
  - The CMSIS-NN library accelerates int8 models for the ARM Cortex-M4F processor.
  - Ensure full precision output if required: `converter.inference_output_type = tf.float32`.

- **Adjust Memory Configuration**:
  - MainCore default memory: 768 KB
  - Maximum memory for MainCore: 1.5 MB (if SubCore and Audio DSP are disabled).

Refer to:
- [Sony Spresense SDK Documentation](https://developer.sony.com/spresense/development-guides)
- [Arduino Developer Guide](https://developer.sony.com/spresense/development-guides/arduino_developer_guide_en.html)

---

Further updates and examples will be added iteratively. Contributions and feedback are welcome!
