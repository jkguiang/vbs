DATACARDS=../datacards/VBSWH_*Lambda/*.dat
WORKSPACES=workspaces
RESULTS=results

if [[ "$1" == "--unblind" ]]; then
    DATACARDS=../datacards/VBSWH_*Lambda_unblinded/*.dat
    WORKSPACES=workspaces_unblinded
    RESULTS=results_unblinded
elif [[ "$1" != "" ]]; then
    echo "ERROR: $1 not a recognized argument"
    exit 1
fi

mkdir -p $WORKSPACES
mkdir -p $RESULTS
total=$(ls $DATACARDS | wc -l)
counter=1
for datacard in $DATACARDS; do
    name=$(basename ${datacard%%.dat})
    workspace=$WORKSPACES/${name}_workspace.root
    result=$RESULTS/${name}_result.root
    log=$RESULTS/${name}_logs.txt
    # Make workspace
    text2workspace.py $datacard -o $workspace
    echo "[$counter/$total] Running limit on $datacard"
    # Run limit
    if [[ "$1" == "--unblind" ]]; then
        # combine -M MultiDimFit -d $workspace -m 125 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --name _${name} --saveNLL &> $log
        combine -M MultiDimFit -d $workspace -m 125 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --name _${name} &> $log
    else
        combine -M MultiDimFit -d $workspace -m 125 -t -1 --expectSignal=0 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --name _${name} &> $log
    fi
    # Move result
    mv higgsCombine_${name}.MultiDimFit.mH125.root $result
    counter=$(($counter + 1))
done

