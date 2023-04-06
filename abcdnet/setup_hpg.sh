#!/bin/bash

# HiPerGator module setup for cuda
module load cuda git torch-geometric

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export SCRAM_ARCH=slc7_amd64_gcc10
export CMSSW_VERSION=CMSSW_12_5_0_pre2
export CUDA_HOME=${HPC_CUDA_DIR}

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /cvmfs/cms.cern.ch/$SCRAM_ARCH/cms/cmssw/$CMSSW_VERSION/src
eval `scramv1 runtime -sh`
cd - > /dev/null

