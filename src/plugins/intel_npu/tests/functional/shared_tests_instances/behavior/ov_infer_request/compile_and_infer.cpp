// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "overload/compile_and_infer.hpp"
#include <npu_private_properties.hpp>
#include "common/utils.hpp"
#include "common/npu_test_env_cfg.hpp"

namespace {

using namespace ov::test::behavior;

const std::vector<ov::AnyMap> configs = {{}};

INSTANTIATE_TEST_SUITE_P(smoke_BehaviorTests, OVCompileAndInferRequest,
                         ::testing::Combine(::testing::Values(getConstantGraph(ov::element::f32)),
                                            ::testing::Values(ov::test::utils::DEVICE_NPU),
                                            ::testing::ValuesIn(configs)),
                         ov::test::utils::appendPlatformTypeTestName<OVCompileAndInferRequest>);

}  // namespace
