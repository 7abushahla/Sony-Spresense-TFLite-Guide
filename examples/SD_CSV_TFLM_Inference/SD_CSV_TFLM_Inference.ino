#include <SDHCI.h>
#include <File.h>
#include <malloc.h> // Include malloc.h for mallinfo()

#define BAUDRATE (115200)

// Initialize SD and File objects
SDClass SD;
File dataFile;

// TensorFlow Lite Micro includes
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "your_model.h"  // Replace with your model header file

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

// Configuration Constants
const unsigned long RANDOM_SEED = 42;          // Random seed for reproducibility
const char* CSV_FILENAME = "data.csv";         // Replace with your CSV file name
const int TOTAL_SAMPLES = 10000;               // Total number of samples in the dataset
const int NUM_FOLDS = 1;                       // Number of folds for inference

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
  model = tflite::GetModel(model_tflite);
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

// Function to load a random sample from the CSV file on SD card
bool loadRandomSampleFromSD(float* input_buffer, int num_samples, int num_channels) {
  // Open the CSV file
  dataFile = SD.open(CSV_FILENAME);
  if (!dataFile) {
    Serial.println("Failed to open test data file.");
    return false;
  }

  // Calculate a random starting point
  int random_index = random(0, TOTAL_SAMPLES);
  int sample_start = random_index * num_samples * num_channels;

  // Seek to the random starting point in the CSV file
  // This assumes each float is separated by a comma and optionally a newline
  // Adjust the parsing method if your CSV format differs
  dataFile.seek(0); // Reset to start
  // Skip to the sample_start
  for (int i = 0; i < sample_start && dataFile.available(); ++i) {
    // Read and discard characters until reaching the start
    // This is a simplistic approach and may need to be adjusted based on CSV formatting
    while (dataFile.available()) {
      char c = dataFile.read();
      if (c == ',') {
        break;
      }
    }
  }

  // Read the sample data
  int i = 0;
  while (dataFile.available() && i < num_samples * num_channels) {
    float value = dataFile.parseFloat();
    input_buffer[i++] = value;
    // Skip the comma after each float
    if (dataFile.peek() == ',') {
      dataFile.read();
    }
  }
  dataFile.close();

  if (i != num_samples * num_channels) {
    Serial.println("Incomplete sample data read.");
    return false;
  }
  return true;
}

// Function to perform inference
float performInference(int num_samples, int num_channels, int num_classes, int num_iterations) {
  float input_buffer[num_samples * num_channels];
  float total_time = 0.0;

  for (int i = 0; i < num_iterations; ++i) {
    // Load a random sample
    if (!loadRandomSampleFromSD(input_buffer, num_samples, num_channels)) {
      Serial.println("Failed to load sample.");
      return -1.0;
    }

    // Copy data to input tensor
    for (int j = 0; j < num_samples * num_channels; ++j) {
      input->data.f[j] = input_buffer[j];
    }

    // Measure inference time
    unsigned long start_time = micros();
    if (interpreter->Invoke() != kTfLiteOk) {
      Serial.println("Inference failed.");
      return -1.0;
    }
    unsigned long end_time = micros();

    // Calculate and display inference time for this iteration
    float inference_time = (end_time - start_time) / 1000.0; // Convert to ms
    Serial.print("Completed iteration ");
    Serial.print(i + 1);
    Serial.print(" - Inference Time: ");
    Serial.print(inference_time, 4);
    Serial.println(" ms");

    // Accumulate total time for averaging
    total_time += inference_time;
  }

  // Return average inference time
  return total_time / num_iterations;
}

void setup() {
  // Initialize random seed for reproducibility
  randomSeed(RANDOM_SEED);

  // Initialize serial communication
  Serial.begin(BAUDRATE);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  // Initialize SD card
  Serial.println("Initializing SD card...");
  while (!SD.begin()) {
    Serial.println("Insert SD card.");
    delay(1000); // Wait before retrying
  }
  Serial.println("SD card initialized.");

  // Initialize TensorFlow Lite model
  setupModel();
}

void loop() {
  // Define inference parameters (adjust as needed)
  const int num_samples = 128;      // Number of samples per inference
  const int num_channels = 2;       // Number of channels (e.g., I and Q)
  const int num_classes = 4;        // Number of output classes
  const int num_iterations = 1000;  // Number of inferences to perform

  float fold_avg_times[NUM_FOLDS];
  float total_fold_time = 0.0;

  for (int fold = 0; fold < NUM_FOLDS; ++fold) {
    Serial.print("Running fold ");
    Serial.println(fold + 1);

    float avg_time = performInference(num_samples, num_channels, num_classes, num_iterations);
    if (avg_time < 0) {
      Serial.println("Error occurred during inference.");
      return;
    }

    Serial.print("Average Inference Time for Fold ");
    Serial.print(fold + 1);
    Serial.print(": ");
    Serial.print(avg_time, 4);
    Serial.println(" ms");

    fold_avg_times[fold] = avg_time;
    total_fold_time += avg_time;
  }

  float overall_avg_time = total_fold_time / NUM_FOLDS;
  Serial.print("\nOverall Avg. Inference Time across ");
  Serial.print(NUM_FOLDS);
  Serial.print(" folds: ");
  Serial.print(overall_avg_time, 4);
  Serial.println(" ms");

  while (true); // Stop after reporting
}