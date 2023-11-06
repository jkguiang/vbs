mkdir -p workspaces
mkdir -p results
total=$(ls datacards/VBSVVH/*.dat | wc -l)
counter=1
for datacard in datacards/VBSVVH/*.dat; do
    name=$(basename ${datacard%%.dat})
    workspace=workspaces/${name}_workspace.root
    result=results/${name}_result.root
    log=results/${name}_logs.txt
    # Make workspace
    text2workspace.py $datacard -o $workspace
    echo "[$counter/$total] Running limit on $datacard"
    # Run limit
    combine -m 125 -M AsymptoticLimits -d ${workspace} -n .${name} --run blind &> $log
    # Move result
    mv higgsCombine.${name}.AsymptoticLimits.mH125.root $result
    counter=$(($counter + 1))
done

