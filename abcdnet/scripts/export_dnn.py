#!/bin/env python

import os
import argparse
from time import time

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from torch.nn import functional as F

import models
from utils import VBSConfig

def trim_leading_newline(multiline_str):
    return "\n".join(multiline_str.split("\n")[1:-1])

SUPPORTED_ACTIVATIONS = [
    nn.ReLU,
    nn.LeakyReLU,
    nn.Sigmoid
]

NEURALNETWORKWEIGHTS_H = """
#ifndef ABCDNetWeights_h
#define ABCDNetWeights_h

namespace ABCDNet
{{
{matrices}
}}

#endif
"""
NEURALNETWORKWEIGHTS_H = trim_leading_newline(NEURALNETWORKWEIGHTS_H)

NEURALNETWORK_H = """
#ifndef ABCDNet_h
#define ABCDNet_h

#include "ABCDNetWeights.h"

namespace ABCDNet
{{
{working_points}

    float run(float hbbfatjet_pt, float hbbfatjet_eta, float hbbfatjet_phi, float hbbfatjet_mass,
              float ld_vqqfatjet_pt, float ld_vqqfatjet_eta, float ld_vqqfatjet_phi, float ld_vqqfatjet_mass,
              float tr_vqqfatjet_pt, float tr_vqqfatjet_eta, float tr_vqqfatjet_phi, float tr_vqqfatjet_mass,
              float M_jj);
}}
#endif
"""
NEURALNETWORK_H = trim_leading_newline(NEURALNETWORK_H)

NEURALNETWORK_CC = """
#include "ABCDNet.h"

float ABCDNet::run(float hbbfatjet_pt, float hbbfatjet_eta, float hbbfatjet_phi, float hbbfatjet_mass,
                   float ld_vqqfatjet_pt, float ld_vqqfatjet_eta, float ld_vqqfatjet_phi, float ld_vqqfatjet_mass,
                   float tr_vqqfatjet_pt, float tr_vqqfatjet_eta, float tr_vqqfatjet_phi, float tr_vqqfatjet_mass,
                   float M_jj);
{{

{features_cpp}

{neural_network_cpp}
    return {output_layer}[0];
}}
"""
NEURALNETWORK_CC = trim_leading_newline(NEURALNETWORK_CC)

MATMUL = """
float {result}[{N2}];
for (unsigned int col = 0; col < {N2}; ++col)
{{
    {result}[col] = 0;
    for (unsigned int inner = 0; inner < {M}; ++inner)
    {{
        {result}[col] += {matrix1}[inner]*{matrix2}[inner][col];
    }}
    {result}[col] += {bias}[col];
}}
"""
MATMUL = trim_leading_newline(MATMUL)

RELU = """
float {new}[{N2}];
for (unsigned int col = 0; col < {N2}; ++col)
{{
    {new}[col] = ({old}[col] > 0.f) ? {old}[col] : 0.f;
}}
"""
RELU = trim_leading_newline(RELU)

LEAKYRELU = """
float {new}[{N2}];
for (unsigned int col = 0; col < {N2}; ++col)
{{
    {new}[col] = ({old}[col] > 0.f) ? {old}[col] : {slope}f*{old}[col];
}}
"""
LEAKYRELU = trim_leading_newline(LEAKYRELU)

SIGMOID = """
float {new}[{N2}];
for (unsigned int col = 0; col < {N2}; ++col)
{{
    {new}[col] = exp({old}[col])/(exp({old}[col]) + 1);
}}
"""
SIGMOID = trim_leading_newline(SIGMOID)

class Cpp:
    def __init__(self, tab="    "):
        self.cpp = []
        self.__tab = tab
        self.__n_tabs = 0

    def indent(self, n=1):
        self.__n_tabs += n

    def dedent(self, n=1):
        self.__n_tabs -= n
        self.__n_tabs = max(self.__n_tabs, 0)

    def newline(self):
        self.add("")

    def __ingress_cpp(self, cpp):
        if type(cpp) == str:
            cpp = cpp.split("\n")
        elif type(cpp) != list:
            raise ValueError("can only add a single line or list of lines")

        expand = []
        for line_i, line in enumerate(cpp):
            if "\n" in line:
                expand.append((line_i, line.split("\n")))

        for insert_i, lines in expand:
            cpp = cpp[:insert_i] + lines + cpp[insert_i+1:]

        return cpp

    def comment(self, cpp):
        cpp = self.__ingress_cpp(cpp)
        if len(cpp) == 1:
            # Single-line comment
            self.add(f"// {cpp[0]}")
        else:
            # Multi-line comment
            self.add("/**")
            for line in cpp:
                self.add(f" * {line}")
            self.add(" */")

    def add(self, cpp):
        cpp = self.__ingress_cpp(cpp)
        for line_i in range(len(cpp)):
            cpp[line_i] = self.__tab*self.__n_tabs + cpp[line_i]
        self.cpp += cpp
    
    def render(self):
        return "\n".join(self.cpp)

def fmt(num):
    decimals = 7
    return f"{num:>{decimals+3}.{decimals}f}f"

def vector_to_cpp(name, vector, init=True):
    cpp = Cpp()
    vec = [fmt(val) for val in vector.tolist()]
    if init:
        cpp.add(f"float {name}[{len(vec)}] = {{")
    else:
        cpp.add(f"{name} = {{")

    cpp.indent()
    cpp.add(",".join(vec))
    cpp.dedent()
    cpp.add("};")

    return cpp

def matrix_to_cpp(name, matrix, flat=False, init=True):
    n_x, n_y = matrix.size()
    cpp = Cpp()
    if init:
        if flat:
            cpp.add(f"float {name}[{n_x}*{n_y}] = {{")
        else:
            cpp.add(f"float {name}[{n_x}][{n_y}] = {{")
    else:
        cpp.add(f"{name} = {{")
    cpp.indent()

    for row_i in range(n_x):
        row = [fmt(val) for val in matrix[row_i].tolist()]
        if flat:
            if row_i == n_x - 1:
                cpp.add(f"{','.join(row)}")
            else:
                cpp.add(f"{','.join(row)},")
        else:
            cpp.add(f"{{ {','.join(row)} }},")

    cpp.dedent()
    cpp.add("};")
    return cpp

def nn_to_cpp(config, model, namespace="ABCDNet"):
    input_size = len(config.ingress.features)

    matmul_cpp = Cpp()
    matrix_cpp = Cpp()
    matmul_cpp.indent()
    matrix_cpp.indent()

    prev_arr = "x"
    N1, M = 1, input_size
    for layer_i, layer in enumerate(model.layers):
        if type(layer) == nn.Linear:
            # x = torch.matmul(x, layer.weight.T) + layer.bias
            matmul_cpp.comment(f"({layer_i}): {layer} => x = x*W_T + b")
            this_arr = f"x_{layer_i}"
            bias_arr = f"bias_{layer_i}"
            matrix_cpp.add(vector_to_cpp(bias_arr, layer.bias).cpp)
            wgts_arr = f"wgtT_{layer_i}"
            matrix_cpp.add(matrix_to_cpp(wgts_arr, layer.weight.T).cpp)
            M, N2 = layer.weight.T.size()

            # add namespace
            bias_arr = f"{namespace}::{bias_arr}"
            wgts_arr = f"{namespace}::{wgts_arr}"

            matmul_cpp.add(MATMUL.format(
                result=this_arr,
                matrix1=prev_arr,
                matrix2=wgts_arr,
                bias=bias_arr,
                N1=N1,
                N2=N2,
                M=M
            ))
            prev_arr = this_arr
            matmul_cpp.newline()
        elif type(layer) in SUPPORTED_ACTIVATIONS:
            if type(layer) == nn.Sigmoid:
                matmul_cpp.comment(f"({layer_i}): {layer}")
                this_arr = f"x_{layer_i}"
                matmul_cpp.add(SIGMOID.format(
                    new=this_arr,
                    old=prev_arr,
                    N1=N1,
                    N2=N2
                ))
                prev_arr = this_arr
                matmul_cpp.newline()
            elif type(layer) == nn.ReLU:
                matmul_cpp.comment(f"({layer_i}): {layer}")
                this_arr = f"x_{layer_i}"
                matmul_cpp.add(RELU.format(
                    new=this_arr,
                    old=prev_arr,
                    N1=N1,
                    N2=N2
                ))
                prev_arr = this_arr
                matmul_cpp.newline()
            elif type(layer) == nn.LeakyReLU:
                matmul_cpp.comment(f"({layer_i}): {layer}")
                this_arr = f"x_{layer_i}"
                matmul_cpp.add(LEAKYRELU.format(
                    new=this_arr,
                    old=prev_arr,
                    N1=N1,
                    N2=N2,
                    slope=0.01
                ))
                prev_arr = this_arr
                matmul_cpp.newline()
        else:
            raise Exception(
                f"{layer} is not a Linear layer, nor is it in the list of supported activations"
            )

    return matmul_cpp, matrix_cpp, prev_arr

def cpp_transform(feature, transf=None):
    if type(transf) == list:
        transf, params = transf
    if transf == None:
        return feature
    elif transf == "rescale":
        min_value, max_value = params
        if min_value > 0:
            return f"({feature} - {min_value})/({max_value} - {min_value})"
        elif min_value < 0:
            return f"({feature} + {abs(min_value)})/({max_value} + {abs(min_value)})"
        else:
            return f"{feature}/{max_value}"
    elif transf == "log":
        return f"log({feature})"
    elif transf == "log2":
        return f"log2({feature})"
    elif transf == "log10":
        return f"log10({feature})"
    else:
        raise ValueError(f"transformation '{transf}' not supported")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Export DNN as matrix multiplication")
    parser.add_argument("config_json", type=str, help="config .json file")
    parser.add_argument("weights_pt", type=str, help="weights .pt file")
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    Model = getattr(models, config.model.name)
    model = Model.from_config(config).to(device)
    model.load_state_dict(torch.load(args.weights_pt, map_location=device))
    model.eval()

    # Generate matrix multiplcation/declaration C++ code
    matmul_cpp, matrix_cpp, output_layer = nn_to_cpp(config, model)

    # Write features vector template
    features_cpp = Cpp()
    features_cpp.indent()
    features_cpp.comment("Build DNN input vector")
    features_cpp.add(f"float x[{len(config.ingress.features)}] = {{")
    features_cpp.indent()
    for feature in config.ingress.features:
        features_cpp.add(cpp_transform(feature, transf=config.ingress.transforms.get(feature, None)) + ",")
    features_cpp.dedent()
    features_cpp.add("};")

    # Write SR working point
    wps_cpp = Cpp()
    wps_cpp.indent()
    wps_cpp.add(f"const float WP_SR = 0.89; // Working point for all-hadronic signal region")

    cc_file = f"ABCDNet.cc"
    with open(cc_file, "w") as f:
        cc = NEURALNETWORK_CC.format(
            features_cpp=features_cpp.render(),
            neural_network_cpp=matmul_cpp.render(),
            output_layer=output_layer
        )
        f.write(cc)
        print(f"Wrote {cc_file}")

    h_file = f"ABCDNet.h"
    with open(h_file, "w") as f:
        h = NEURALNETWORK_H.format(
            working_points=wps_cpp.render()
        )
        f.write(h)
        print(f"Wrote {h_file}")

    h_file = f"ABCDNetWeights.h"
    with open(h_file, "w") as f:
        h = NEURALNETWORKWEIGHTS_H.format(
            matrices=matrix_cpp.render()
        )
        f.write(h)
        print(f"Wrote {h_file}")
