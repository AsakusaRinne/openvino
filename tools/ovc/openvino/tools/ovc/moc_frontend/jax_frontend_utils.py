# Copyright (C) 2018-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import logging as log
import sys

import numpy as np
# pylint: disable=no-name-in-module,import-error
from openvino.runtime import Tensor, PartialShape
from openvino.tools.ovc.error import Error


def get_jax_decoder(model, example_inputs, args):
    try:
        from openvino.frontend.jax.jaxpr_decoder import JaxprPythonDecoder
        import jax
    except Exception as e:
        log.error("Jax frontend loading failed")
        raise e
    
    inputs = prepare_jax_inputs(example_inputs)
    if not isinstance(model, JaxprPythonDecoder):
        decoder = JaxprPythonDecoder(model, inputs)
    else:
        decoder = model
    
    args['input_model'] = decoder
    args['example_input'] = inputs

def prepare_jax_inputs(example_inputs):
    # TODO: check the corner cases
    return example_inputs