#!/bin/bash

# HiPerGator module setup for cuda
module load cuda git torch-geometric

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export CUDA_HOME=${HPC_CUDA_DIR}
export PYTHONPATH=${PWD}/python:$PYTHONPATH
