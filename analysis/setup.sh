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
chmod u+x $VBSPWD/bin/*
