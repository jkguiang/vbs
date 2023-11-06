# # Make workspace
# rm -f vbswh_SM.root
# text2workspace.py ../datacards/VBSWH_posLambda/scan_kW_1p0_kZ_1p0.dat -o vbswh_SM.root
# # Run limit
# combine -M MultiDimFit -d vbswh_SM.root -m 125 -t -1 --expectSignal=1 --setParameters r=0 --setParameterRanges r=10.0,30.0 --saveNLL --algo grid --points 101 --rMin 10 --rMax 30 --alignEdges 1 --name _vbswh_SM

NAME=vbswh_SM
if [[ "$1" == "--unblind" ]]; then
    NAME=vbswh_SM_unblinded
elif [[ "$1" != "" ]]; then
    echo "ERROR: $1 not a recognized argument"
    exit 1
fi

# Make workspace
rm -f ${NAME}.root
if [[ "$1" == "--unblind" ]]; then
    text2workspace.py ../datacards/VBSWH_posLambda_unblinded/scan_kW_1p0_kZ_1p0.dat -o ${NAME}.root
else
    text2workspace.py ../datacards/VBSWH_posLambda/scan_kW_1p0_kZ_1p0.dat -o ${NAME}.root
fi
# Run limit
if [[ "$1" == "--unblind" ]]; then
    combine -M MultiDimFit -d ${NAME}.root -m 125 --setParameters r=0 --setParameterRanges r=0.0,100.0 --algo grid --points 1001 --alignEdges 1 --name _$NAME 
else
    combine -M MultiDimFit -d ${NAME}.root -m 125 -t -1 --expectSignal=0 --setParameters r=0 --setParameterRanges r=0.0,100.0 --algo grid --points 1001 --alignEdges 1 --name _$NAME 
fi

if [[ "$1" == "--unblind" ]]; then
    echo ""
    combine -M AsymptoticLimits -d ${NAME}.root

    echo ""
    combine -M Significance -d ${NAME}.root
else
    echo ""
    combine -M AsymptoticLimits -d ${NAME}.root

    echo ""
    combine -M Significance -d ${NAME}.root -t -1 --expectSignal=1
fi
