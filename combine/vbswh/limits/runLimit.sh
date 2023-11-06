NAME=vbswh
if [[ "$1" == "--unblind" ]]; then
    NAME=vbswh_unblinded
elif [[ "$1" != "" ]]; then
    echo "ERROR: $1 not a recognized argument"
    exit 1
fi

# Make workspace
rm -f ${NAME}.root
text2workspace.py ../datacards/${NAME}.dat -o ${NAME}.root
# Run limit
if [[ "$1" == "--unblind" ]]; then
    combine -M MultiDimFit -d ${NAME}.root -m 125 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --name _$NAME 
else
    combine -M MultiDimFit -d ${NAME}.root -m 125 -t -1 --expectSignal=0 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --name _$NAME 
fi
