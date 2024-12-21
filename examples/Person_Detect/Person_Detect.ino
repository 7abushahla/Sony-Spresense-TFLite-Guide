#include <Camera.h>
#include <malloc.h> // Include malloc.h for mallinfo()

// TensorFlow Lite Micro includes
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "person_detect_model.h"

// Constants
#define BAUDRATE (115200)

// Camera Configuration Constants (Adjust as needed)
const int OFFSET_X = 32;
const int OFFSET_Y = 12;
const int IMAGE_WIDTH = 160;
const int IMAGE_HEIGHT = 120;
const int TARGET_WIDTH = 96;
const int TARGET_HEIGHT = 96;
const int PIXFMT = CAM_IMAGE_PIX_FMT_YUV422;

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

  // Function to report memory usage
  void ReportMemoryUsage(const char* context) {
    struct mallinfo mi = mallinfo(); // Get memory allocation info
    Serial.print(context);
    Serial.print(" Free memory: ");
    Serial.println(mi.fordblks, DEC); // Free space in the heap
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
  model = tflite::GetModel(model_tflite); // Ensure model_tflite is defined in person_detect_model.h
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version mismatch.");
    while (1); // Halt if version mismatch
  } else {
    Serial.println("Model schema version matches.");
  }

  // Create an operator resolver
  static tflite::AllOpsResolver resolver;
  // For optimization, include only the operations your model requires

  // Create the interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory for the tensors
  ReportMemoryUsage("Before AllocateTensors(): ");
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed.");
    while (1); // Halt if allocation fails
  } else {
    Serial.println("AllocateTensors() Success.");
  }
  ReportMemoryUsage("After AllocateTensors(): ");

  size_t used_size = interpreter->arena_used_bytes();
  Serial.println("Arena used bytes: " + String(used_size));

  // Assign model input and output buffers (tensors) to pointers
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Get information about the model's input tensor
  Serial.println("Model Input:");
  Serial.println("Number of dimensions: " + String(input->dims->size));
  for (int n = 0; n < input->dims->size; ++n) {
    Serial.println("dims->data[" + String(n) + "]: " + String(input->dims->data[n]));
  }
  Serial.print("Input type: ");
  Serial.println(input->type);

  // Get information about the model's output tensor
  Serial.println("\nModel Output:");
  Serial.println("Number of dimensions: " + String(output->dims->size));
  for (int n = 0; n < output->dims->size; ++n) {
    Serial.println("dims->data[" + String(n) + "]: " + String(output->dims->data[n]));
  }
  Serial.print("Output type: ");
  Serial.println(output->type);

  Serial.println("Completed TensorFlow setup.");
  Serial.println();
}

// Function to print camera errors
void printError(enum CamErr err) {
  Serial.print("Error: ");
  switch (err) {
    case CAM_ERR_NO_DEVICE:
      Serial.println("No Device");
      break;
    case CAM_ERR_ILLEGAL_DEVERR:
      Serial.println("Illegal device error");
      break;
    case CAM_ERR_ALREADY_INITIALIZED:
      Serial.println("Already initialized");
      break;
    case CAM_ERR_NOT_INITIALIZED:
      Serial.println("Not initialized");
      break;
    case CAM_ERR_NOT_STILL_INITIALIZED:
      Serial.println("Still picture not initialized");
      break;
    case CAM_ERR_CANT_CREATE_THREAD:
      Serial.println("Failed to create thread");
      break;
    case CAM_ERR_INVALID_PARAM:
      Serial.println("Invalid parameter");
      break;
    case CAM_ERR_NO_MEMORY:
      Serial.println("No memory");
      break;
    case CAM_ERR_USR_INUSED:
      Serial.println("Buffer already in use");
      break;
    case CAM_ERR_NOT_PERMITTED:
      Serial.println("Operation not permitted");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
}

// Callback function for camera image capture
void CamCB(CamImage img) {
  static uint32_t last_millis = 0;

  if (!img.isAvailable()) {
    Serial.println("Image is not available");
    return;
  }

  uint16_t* buf = (uint16_t*)img.getImgBuff();   
  int n = 0;
   
  for (int y = OFFSET_Y; y < OFFSET_Y + TARGET_HEIGHT; ++y) {
    for (int x = OFFSET_X; x < OFFSET_X + TARGET_WIDTH; ++x) {
      uint16_t value = buf[y * IMAGE_WIDTH + x];
      uint16_t y_h = (value & 0xF000) >> 8;
      uint16_t y_l = (value & 0x00F0) >> 4;
      value = (y_h | y_l);      
      input->data.f[n++] = (float)(value) / 255.0;
    }
  }

  Serial.println("Running inference...");

  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Inference failed");
    return;
  }

  // Assuming output tensor has at least two elements
  int8_t person_score = output->data.int8[1];
  int8_t no_person_score = output->data.int8[0];
  
  Serial.print("Person Score: " + String(person_score) + ", ");
  Serial.println("No Person Score: " + String(no_person_score));
  
  if ((person_score > no_person_score) && (person_score > 40)) {
    Serial.println("Person detected! \n");
  } else {
    Serial.println("No person detected. \n");
  }
  
  uint32_t current_millis = millis();
  uint32_t duration = current_millis - last_millis;
  Serial.println("Inference duration: " + String(duration) + " ms");
  last_millis = current_millis; 
}

void setup() {
  CamErr err;

  // Initialize serial communication
  Serial.begin(BAUDRATE);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  // Initialize TensorFlow Lite model
  setupModel();

  // Initialize Camera
  Serial.println("Prepare camera");
  err = theCamera.begin(1, CAM_VIDEO_FPS_15, IMAGE_WIDTH, IMAGE_HEIGHT, PIXFMT);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
    while (1); // Halt if camera initialization fails
  }

  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
    while (1); // Halt if streaming fails
  }

  /* Auto white balance configuration */
  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
    // Continue even if setting white balance fails
  }
}

void loop() {
  // Main loop can remain empty or include other functionalities
  // Since inference is handled via the camera callback, no action is needed here
  delay(1000); // Optional: Add a delay to reduce CPU usage
}
