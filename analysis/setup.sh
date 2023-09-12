# Determine the RHEL (Red Hat Enterprise Linux) version
RHELREL=$(cat /etc/redhat-release)
if [[ "$(hostname)" == "uaf-10.t2.ucsd.edu" ]]; then
    RHELVER=7
elif [[ "$(hostname)" == "uaf-8.t2.ucsd.edu" ]]; then
    RHELVER=7
elif [[ "$(hostname)" == "uaf-1.t2.ucsd.edu" ]]; then
    RHELVER=7
elif [[ $RHELREL == 'CentOS Linux release 7.9.2009 (Core)' ]]; then
    RHELVER=7
elif [[ "$(hostname)" == "uaf-2.t2.ucsd.edu" ]]; then
    RHELVER=8
elif [[ "$(hostname)" == "uaf-3.t2.ucsd.edu" ]]; then
    RHELVER=8
elif [[ $RHELREL == 'AlmaLinux release 8.8 (Sapphire Caracal)' ]]; then
    RHELVER=8
else
    echo "WARNING: guessing that the OS is RHEL7; please check and adjust accordingly!"
    RHELVER=7
fi

# Set up CMSSW 'environment'
if [[ $RHELVER == 7 ]]; then
    cd /cvmfs/cms.cern.ch/slc7_amd64_gcc900/cms/cmssw/CMSSW_12_2_0/src
    cmsenv
    echo "Now using CMSSW v12.2.0 (RHEL7)"
elif [[ $RHELVER == 8 ]]; then
    cd /cvmfs/cms.cern.ch/el8_amd64_gcc11/cms/cmssw/CMSSW_12_5_0/src
    cmsenv
    echo "Now using CMSSW v12.5.0 (RHEL8)"
fi
cd - &> /dev/null

# Modify PATH
export VBSPWD=$PWD
LIBPATHS="
$VBSPWD/rapido/src
$VBSPWD/NanoTools/NanoCORE
$CMSSW_BASE/lib/$SCRAM_ARCH
"
for LIBPATH in $LIBPATHS; do
    # Add library to LD_LIBRARY_PATH
    if [[ "$LD_LIBRARY_PATH" != *"$LIBPATH"* ]]; then
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBPATH
    fi
    # Add library to ROOT_INCLUDE_PATH
    if [[ "$ROOT_INCLUDE_PATH" != *"$LIBPATH"* ]]; then
        export ROOT_INCLUDE_PATH=$ROOT_INCLUDE_PATH:$LIBPATH
    fi
done

# Make all executables executable
chmod u+x $VBSPWD/bin/*

# Export some useful CMSSW paths
CMSSW_EXT=$CMSSW_BASE/../../../external
if [[ $RHELVER == 7 ]]; then
    export CORRECTIONLIBDIR=$CMSSW_EXT/py3-correctionlib/2.0.0-0c4f44c8dd5561d8c0660135feeb81f4/lib/python3.9/site-packages/correctionlib
    export BOOSTDIR=$CMSSW_EXT/boost/1.67.0
elif [[ $RHELVER == 8 ]]; then
    export CORRECTIONLIBDIR=$CMSSW_EXT/py3-correctionlib/2.1.0-6711d4d6283f175d9b6fd011bbaad506/lib/python3.9/site-packages/correctionlib
    export BOOSTDIR=$CMSSW_EXT/boost/1.80.0-7f4aeae1bffcf24aa4723f09435633c2
fi
