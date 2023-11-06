NAME=vbswh
if [[ "$1" == "--unblind" ]]; then
    NAME=vbswh_unblinded
elif [[ "$1" != "" ]]; then
    echo "ERROR: $1 not a recognized argument"
    exit 1
fi

# Make workspace
rm -f ${NAME}_coarse.root
text2workspace.py ../datacards/${NAME}.dat -o ${NAME}_coarse.root
# Run limit
if [[ "$1" == "--unblind" ]]; then
    # combine -M MultiDimFit -d ${NAME}.root -m 125 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --name _$NAME 
    combine -M MultiDimFit -d ${NAME}_coarse.root -m 125 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 1001 --alignEdges 1 --name _${NAME}_coarse
else
    combine -M MultiDimFit -d ${NAME}_coarse.root -m 125 -t -1 --expectSignal=0 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 1001 --alignEdges 1 --name _${NAME}_coarse
fi
