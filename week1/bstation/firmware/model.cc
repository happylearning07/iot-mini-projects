/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <math.h>

#include "tensorflow/lite/core/c/common.h"
#include "model.h"
#include "../../model/lstm/model_data.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
//#include "tensorflow/lite/micro/testing/micro_test.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace {
using HelloWorldOpResolver = tflite::MicroMutableOpResolver<1>;

TfLiteStatus RegisterOps(HelloWorldOpResolver& op_resolver) {
  TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
  return kTfLiteOk;
}
}  // namespace

TfLiteStatus LoadFloatModelAndPerformInference(float *output_array) {
  const tflite::Model* model =
      ::tflite::GetModel(model_tflite);
  TFLITE_CHECK_EQ(model->version(), TFLITE_SCHEMA_VERSION);

  HelloWorldOpResolver op_resolver;
  TF_LITE_ENSURE_STATUS(RegisterOps(op_resolver));

  // Arena size just a round number. The exact arena usage can be determined
  // using the RecordingMicroInterpreter.
  constexpr int kTensorArenaSize = 3000;
  uint8_t tensor_arena[kTensorArenaSize];

  tflite::MicroInterpreter interpreter(model, op_resolver, tensor_arena,
                                       kTensorArenaSize);
  TF_LITE_ENSURE_STATUS(interpreter.AllocateTensors());

  // Check if the predicted output is within a small range of the
  // expected output
  float golden_inputs[4] = {20.f, 40.f, 1.007f, 150.f};

  // Obtain a pointer to the model's input tensor
  TfLiteTensor* input = interpreter.input(0);

// Make sure the input has the properties we expect
//TF_LITE_MICRO_EXPECT_NE(nullptr, input);
// The property "dims" tells us the tensor's shape. It has one element for
// each dimension. Our input is a 2D tensor containing 1 element, so "dims"
// should have size 2.
//TF_LITE_MICRO_EXPECT_EQ(2, input->dims->size);
// The value of each element gives the length of the corresponding tensor.
// We should expect two single element tensors (one is contained within the
// other).
//TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[0]);
//TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[1]);
// The input is a 32 bit floating point value
//TF_LITE_MICRO_EXPECT_EQ(kTfLiteFloat32, input->type);

  input->data.f[0] = golden_inputs[0];
  input->data.f[1] = golden_inputs[1];
  input->data.f[2] = golden_inputs[2];
  input->data.f[3] = golden_inputs[3];
  TF_LITE_ENSURE_STATUS(interpreter.Invoke());

  TfLiteTensor* output = interpreter.output(0);
//TF_LITE_MICRO_EXPECT_EQ(2, output->dims->size);
//TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[0]);
//TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[1]);
//TF_LITE_MICRO_EXPECT_EQ(kTfLiteFloat32, output->type);

  output_array[0] = output->data.f[0];
  output_array[1] = output->data.f[1];
  output_array[2] = output->data.f[2];
  output_array[3] = output->data.f[3];
  return kTfLiteOk;
}

void modelCode(float *output_array) {
  tflite::InitializeTarget();
  TF_LITE_ENSURE_STATUS(LoadFloatModelAndPerformInference(output_array));
}
