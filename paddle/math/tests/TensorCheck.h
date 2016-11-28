/* Copyright (c) 2016 Baidu, Inc. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <cmath>
#include <gtest/gtest.h>
#include "paddle/math/Matrix.h"

using namespace paddle;  // NOLINT
using namespace std;     // NOLINT

namespace autotest {

class CheckEqual {
public:
  inline int operator()(real a, real b) {
    if (a != b) {
      return 1;
    }

    return 0;
  }
};

class CheckWithErr {
public:
  CheckWithErr() {
#ifndef PADDLE_TYPE_DOUBLE
    err_ = 1e-5;
#else
    err_ = 1e-10;
#endif
  }

  inline int operator()(real a, real b) {
    if (std::fabs(a - b) > err_) {
      if ((std::fabs(a - b) / std::fabs(a)) > (err_ / 10.0f)) {
        return 1;
      }
    }
    return 0;
  }

private:
  real err_;
};

template<typename Check>
void TensorCheck(Check op, const CpuMatrix& matrix1, const CpuMatrix& matrix2) {
  CHECK(matrix1.getHeight() == matrix2.getHeight());
  CHECK(matrix1.getWidth() == matrix2.getWidth());

  int height = matrix1.getHeight();
  int width = matrix1.getWidth();
  const real* data1 = matrix1.getData();
  const real* data2 = matrix2.getData();
  int count = 0;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      real a = data1[i * width + j];
      real b = data2[i * width + j];
      count += op(a, b);
    }
  }
  EXPECT_EQ(count, 0) << "There are " << count << " different element.";
}

template <typename Tensor>
class CopyToCpu;

template <>
class CopyToCpu<CpuMatrix> {
public:
  explicit CopyToCpu(const CpuMatrix& arg) : arg_(arg) {}
  const CpuMatrix& copiedArg() const { return arg_; }

private:
  const CpuMatrix& arg_;
};

template <>
class CopyToCpu<GpuMatrix> {
public:
  explicit CopyToCpu(const GpuMatrix& arg)
    : arg_(arg.getHeight(), arg.getWidth()) {
    arg_.copyFrom(arg);
  }
  CpuMatrix& copiedArg() { return arg_; }

private:
  CpuMatrix arg_;
};

template<typename Tensor1, typename Tensor2>
extern void TensorCheckErr(const Tensor1& tensor1, const Tensor2& tensor2) {
  TensorCheck(
    CheckWithErr(),
    CopyToCpu<Tensor1>(tensor1).copiedArg(),
    CopyToCpu<Tensor2>(tensor2).copiedArg());
}

}  // namespace autotest

