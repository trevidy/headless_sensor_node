// #include "anomaly_detector.h"
// #include "autoencoder_model.h"
// #include "logger.h"
// #include <stdio.h>
// #include <cmath>

// // TFLM headers from esp-tflite-micro component
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "tensorflow/lite/schema/schema_generated.h"

// // --- Scaler constants ---
// // hardcoded from scaler.pkl fit on training data (phase 2)
// // if retrained on new data, these must be updated to match
// static constexpr float TEMP_MIN = 22.5f;
// static constexpr float TEMP_MAX = 34.4f;
// static constexpr float HUMIDITY_MIN = 37.9f;
// static constexpr float HUMIDITY_MAX = 75.1;
// static constexpr float PRESSURE_MIN = 1002.1f;
// static constexpr float PRESSURE_MAX = 1016.7f;

// // --- Anomaly threshold ---
// // 99th percentile reconstruction error from int8 model validation (phase 3)
// // raise this value to reduce false positives; lower it for more sensitivity
// static constexpr float ANOMALY_THRESHOLD 0.007025f;

// // --- tensor arena ---
// // Static allocation - TFLM never uses malloc, all memory comes from this block
// // Estimated 8kB, can do 12kB for more headroom
// // after validating, trim this down using arena_used_bytes()
// // alignas(n) : makes the variable start at memory address that is a multiple of n bytes, or else performance will be slow or would crash.
// static constexpr int TENSOR_ARENA_SIZE = 12 * 1024;
// alignas(16) static uint8_t tensor_arena[TENSOR_ARENA_SIZE];

// // --- TFLM STATE ---
// // In c/c++, read pointer declaration from right to left. tflm_model is a pointer to a const tflite::Model.
// // This means the model data lives in flash, but the pointer itself is NOT const, it lives in RAM.
// //
// // 'tflite::' : a namespace, or a way to group classes, functions, and types under a specific prefix.
// static const tflite::Model* tflm_model = nullptr;
// static tflite::MicroInterpreter* interpreter = nullptr;
// static TfLiteTensor* input_tesnor = nullptr;
// static TfLiteTensor* output_tensor = nullptr;
// static bool initialized = false;

// // Op resolver - only register ops actually used by this model
// // To verify how many operators are needed, can go to netro.app and upload the .tflite file.
// // (python) Dense+ReLU -> (microcontroller) FullyConnected+Relu; (python) Dense+sigmoid -> (microcontroller) FullyConnected+Logistic
// // Dense : fully connected layer. every single neuron in the input layer is connected to every single neuron in the output layer
// // fully connected operator : a matrix multiplication followed by adding a bias: Y = X * W + b
// // ReLU (rectifier) : an activation function used to introduce non-linearity.
// // Logistic (sigmoid) : another activation function, usually used at the very end of a model for binary classification. It squashes any input value into between 0 and 1, which represents probability
// static tflite::MicroMutableOpResolver<3> resolver; //generates a version of MicroMutableOpResolver that has 3 slots allocated for operations at compile time.

// // --- Helpers ---
// static inline float normalize(float value, float min_val, float max_val)
// {
//     return (value - min_val) / (max_val - min_val)
// }

// // --- public API ---
// bool anomaly_detector_init()
// {
//     // Load model from the C array embedded in the autoencoder_model.h
//     tflm_model = tflite::GetModel(autoencoder_tflite);

//     // schema is the physical layout ruleset of the Flatbuffer (serialized byte array the autoencoder model turned into, autoencoder_tflite)
//     if (tflm_model->version() != TFLITE_SCHEMA_VERSION) // check it matches current schema version. google may make updates to schema layout and thus different versions
//     {
//         printf("TFLM: schema mismatch - expected %d, got %d\n",
//             TFLITE_SCHEMA_VERSION, (int)tflm_model->version());
//         return false;
//     }

//     // register only the ops this model uses - keeps binary size small
//     resolver.AddFullyConnected();
//     resolver.AddRelu();
//     resolver.AddLogistic();

//     // construct interpreter in static storage - avoids heap allocation
//     static tflite::MicroInterpreter static_interpreter(
//         tflm_model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
//     interpreter = &static_interpreter;

//     // allocate input/output tensors inside the arena
// }