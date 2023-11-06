NAME=vbswh_forPlots
if [[ "$1" == "--unblind" ]]; then
    NAME=vbswh_unblinded
elif [[ "$1" != "" ]]; then
    echo "ERROR: $1 not a recognized argument"
    exit 1
fi

# Make workspace
rm -f ${NAME}_forPlots.root
text2workspace.py ../datacards/${NAME}_forPlots.dat -o ${NAME}_forPlots.root
# Run limit
combine -M FitDiagnostics ${NAME}_forPlots.root -m 125 --rMin 0 --rMax 5 --name _$NAME --saveShapes --saveWithUncertainties
