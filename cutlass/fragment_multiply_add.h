/***************************************************************************************************
 * Copyright (c) 2017-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of
 *       conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TOR (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************************************/
/*! \file
    \brief Defines multiply-add operations on fragments within a thread.
*/
#pragma once

#include <cutlass/fragment.h>

namespace cutlass {
namespace gemm {

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Scalar_>
struct FragmentMultiplyAdd {
  /// The shape of the instruction.
  typedef Shape<1, 1, 1, 1> InstructionShape;
  /// The type for A.
  typedef Scalar_ ScalarA;
  /// The type for B.
  typedef Scalar_ ScalarB;
  /// The type for C and D.
  typedef Scalar_ ScalarC;

  /// Ctor.
  CUTLASS_DEVICE FragmentMultiplyAdd() {}

  /// Multiply : d = a*b.
  template <typename Fragment_>
  CUTLASS_DEVICE void multiply(Scalar_ a, Fragment_ const& b, Fragment_& d) {
    for (int j = 0; j < Fragment_::kElements; ++j) {
      d[j] = a * b[j];
    }
  }

  /// Multiply : d = a*b + c.
  template <typename Fragment_>
  CUTLASS_DEVICE void multiply_add(Scalar_ a,
                                   Fragment_ const& b,
                                   Fragment_ const& c,
                                   Fragment_& d) {
    for (int j = 0; j < Fragment_::kElements; ++j) {
      d[j] = a * b[j] + c[j];
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(__CUDACC_RTC__) || defined(CUTLASS_NVRTC_HAS_FP16)
template <>
struct FragmentMultiplyAdd<half> {
  /// The shape of the instruction.
  typedef Shape<1, 1, 1, 1> InstructionShape;
  /// The type for A.
  typedef half ScalarA;
  /// The type for B.
  typedef half ScalarB;
  /// The type for C and D.
  typedef half ScalarC;

  /// Ctor.
  CUTLASS_DEVICE FragmentMultiplyAdd() {}

  /// Multiply : d = a*b.
  template <typename Fragment_>
  CUTLASS_DEVICE void multiply(half a, Fragment_ const& b, Fragment_& d) {
#if defined(__CUDACC__) && __CUDA_ARCH__ >= 530
    // The input.
    __half2 const* b_half2 = reinterpret_cast<__half2 const*>(&b[0]);
    // The output.
    __half2* d_half2 = reinterpret_cast<__half2*>(&d[0]);

    // Assemble a half2 from a.
    __half2 const a_half2 = __half2half2(a);

    for (int i = 0; i < Fragment_::kElements / 2; ++i) {
      d_half2[i] = __hmul2(a_half2, b_half2[i]);
    }
#endif
  }

  /// Multiply : d = a*b + c.
  template <typename Fragment_>
  CUTLASS_DEVICE void multiply_add(half a, Fragment_ const& b, Fragment_ const& c, Fragment_& d) {
#if defined(__CUDACC__) && __CUDA_ARCH__ >= 530
    // The inputs.
    __half2 const* b_half2 = reinterpret_cast<__half2 const*>(&b[0]);
    __half2 const* c_half2 = reinterpret_cast<__half2 const*>(&c[0]);
    // The output.
    __half2* d_half2 = reinterpret_cast<__half2*>(&d[0]);

    // Assemble a half2 from a.
    __half2 const a_half2 = __half2half2(a);

    for (int i = 0; i < Fragment_::kElements / 2; ++i) {
      d_half2[i] = __hfma2(a_half2, b_half2[i], c_half2[i]);
    }
#endif
  }
};

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace gemm
}  // namespace cutlass