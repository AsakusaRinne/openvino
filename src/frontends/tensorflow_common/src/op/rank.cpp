// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "common_op_table.hpp"
#include "helper_ops/complex_type_mark.hpp"
#include "openvino/op/constant.hpp"
#include "openvino/op/shape_of.hpp"
#include "openvino/op/squeeze.hpp"
#include "openvino/util/log.hpp"

using namespace std;
using namespace ov::op;

namespace ov {
namespace frontend {
namespace tensorflow {
namespace op {

ov::OutputVector translate_rank_op(const NodeContext &node) {
  OPENVINO_DEBUG << "Translating Rank...............";
  default_op_checks(node, 1, {"Rank"}, true);
  auto input = node.get_input(0);
  auto complex_type_mark =
      as_type_ptr<ComplexTypeMark>(input.get_node_shared_ptr());
  auto input_shape = make_shared<v3::ShapeOf>(input, ov::element::i32);
  if (complex_type_mark) {
    element::Type complex_part_type =
        complex_type_mark->get_complex_part_type();
    input = complex_type_mark->input_value(0);

    auto unsqueeze_input_rank =
        make_shared<v3::ShapeOf>(input_shape, ov::element::i32);
    auto input_rank = make_shared<v0::Squeeze>(unsqueeze_input_rank);
    input_rank = make_shared<v0::Squeeze>(
        input_rank,
        make_shared<v0::Constant>(ov::element::i32, Shape{1},
                                  input_shape->get_input_size() - 1));
    set_node_name(node.get_name(), input_rank);
    auto complex_rank =
        make_shared<ComplexTypeMark>(input_rank, complex_part_type);
    return {complex_rank->output(0)};
  }
  auto unsqueeze_input_rank =
      make_shared<v3::ShapeOf>(input_shape, ov::element::i32);
  auto input_rank = make_shared<v0::Squeeze>(unsqueeze_input_rank);
  set_node_name(node.get_name(), input_rank);
  return {input_rank};
}

} // namespace op
} // namespace tensorflow
} // namespace frontend
} // namespace ov
