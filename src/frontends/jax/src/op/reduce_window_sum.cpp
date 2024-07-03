// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

#include "openvino/core/node.hpp"
#include "openvino/core/node_output.hpp"
#include "openvino/frontend/jax/node_context.hpp"
#include "openvino/op/avg_pool.hpp"
#include "openvino/op/constant.hpp"
#include "openvino/op/multiply.hpp"
#include "openvino/op/transpose.hpp"
#include "utils.hpp"

namespace ov {
namespace frontend {
namespace jax {
namespace op {

using namespace ov::op;

OutputVector translate_reduce_window_sum(const NodeContext& context) {
    num_inputs_check(context, 1, 1);
    Output<Node> input = context.get_input(0);

    auto window_dimensions = context.const_named_param<std::vector<int64_t>>("window_dimensions");
    auto window_strides = context.const_named_param<std::vector<int64_t>>("window_strides");
    auto padding = context.const_named_param<std::vector<std::vector<int64_t>>>("padding");
    auto base_dilation = context.const_named_param<std::vector<int64_t>>("base_dilation");
    auto window_dilation = context.const_named_param<std::vector<int64_t>>("window_dilation");
    size_t total_dim = window_dimensions.size();

    JAX_OP_CONVERSION_CHECK(window_strides.size() == total_dim,
                            "Internal error: window_strides must have the same size as window_dimensions, but got " +
                                std::to_string(window_strides.size()) + " and " + std::to_string(total_dim));
    JAX_OP_CONVERSION_CHECK(padding.size() == total_dim,
                            "Internal error: padding must have the same size as window_dimensions, but got " +
                                std::to_string(padding.size()) + " and " + std::to_string(total_dim));
    JAX_OP_CONVERSION_CHECK(base_dilation.size() == total_dim,
                            "Internal error: base_dilation must have the same size as window_dimensions, but got " +
                                std::to_string(base_dilation.size()) + " and " + std::to_string(total_dim));
    JAX_OP_CONVERSION_CHECK(window_dilation.size() == total_dim,
                            "Internal error: window_dilation must have the same size as window_dimensions, but got " +
                                std::to_string(window_dilation.size()) + " and " + std::to_string(total_dim));

    Strides strides(total_dim - 2);
    Shape pads_begin(total_dim - 2);
    Shape pads_end(total_dim - 2);
    Shape kernel(total_dim - 2);
    for (size_t ind = 0; ind < total_dim; ++ind) {
        if (ind != 0 && ind != total_dim - 1) {
            kernel[ind - 1] = static_cast<size_t>(window_dimensions[ind]);
            pads_begin[ind - 1] = padding[ind][0];
            pads_end[ind - 1] = padding[ind][1];
            strides[ind - 1] = static_cast<size_t>(window_strides[ind]);
        } else {
            // only support NHWC format input now.
            JAX_OP_CONVERSION_CHECK(window_dimensions[ind] == 1, "Internal error: unsupported layout.");
            JAX_OP_CONVERSION_CHECK(window_strides[ind] == 1, "Internal error: unsupported layout.");
            JAX_OP_CONVERSION_CHECK(padding[ind][0] == 0 && padding[ind][1] == 0,
                                    "Internal error: unsupported layout.");
            JAX_OP_CONVERSION_CHECK(base_dilation[ind] == 1, "Internal error: unsupported layout.");
        }
        JAX_OP_CONVERSION_CHECK(window_dilation[ind] == 1, "Internal error: only window_dilation 1 is supported.");
    }

    std::vector<int64_t> in_transpose_vector(total_dim);
    std::vector<int64_t> out_transpose_vector(total_dim);
    in_transpose_vector[0] = 0;
    in_transpose_vector[1] = total_dim - 1;
    out_transpose_vector[0] = 0;
    out_transpose_vector[total_dim - 1] = 1;
    for (size_t i = 2; i < total_dim; i++) {
        in_transpose_vector[i] = i - 1;
        out_transpose_vector[i - 1] = i;
    }
    auto input_transpose_order =
        std::make_shared<v0::Constant>(element::i64, Shape{in_transpose_vector.size()}, in_transpose_vector);
    auto output_transpose_order =
        std::make_shared<v0::Constant>(element::i64, Shape{out_transpose_vector.size()}, out_transpose_vector);

    input = std::make_shared<v1::Transpose>(input, input_transpose_order);
    Output<Node> res = std::make_shared<v14::AvgPool>(input, strides, pads_begin, pads_end, kernel, true);
    res = std::make_shared<v1::Transpose>(res, output_transpose_order);
    auto kernel_size = std::accumulate(kernel.begin(), kernel.end(), 1, std::multiplies<size_t>());
    Output<Node> kernel_size_constant =
        std::make_shared<v0::Constant>(res.get_element_type(), Shape{}, std::vector<int64_t>{kernel_size});
    res = std::make_shared<v1::Multiply>(res, kernel_size_constant);
    return {res};
};

}  // namespace op
}  // namespace jax
}  // namespace frontend
}  // namespace ov