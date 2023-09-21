mkdir -p workspaces
mkdir -p results
total=$(ls datacards/VBSWH_*Lambda/*.dat | wc -l)
# total=$(ls datacards/VBSWH_kWkZscan/*.dat | wc -l)
counter=1
for datacard in datacards/VBSWH_*Lambda/*.dat; do
# for datacard in datacards/VBSWH_kWkZscan/*.dat; do
    name=$(basename ${datacard%%.dat})
    workspace=workspaces/${name}_workspace.root
    result=results/${name}_result.root
    log=results/${name}_logs.txt
    # Make workspace
    text2workspace.py $datacard -o $workspace
    echo "[$counter/$total] Running limit on $datacard"
    # Run limit
    combine -M MultiDimFit -d $workspace -m 125 -t -1 --expectSignal=0 --setParameters r=0 --setParameterRanges r=0.0,2.0 --saveNLL --algo grid --points 101 --rMin 0 --rMax 5 --alignEdges 1 --name _${name} &> $log
    # Move result
    mv higgsCombine_${name}.MultiDimFit.mH125.root $result
    counter=$(($counter + 1))
done

