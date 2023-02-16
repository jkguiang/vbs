mkdir -p workspaces
mkdir -p results
for datacard in $(ls datacards/VBSWH_*Lambda*/*.dat); do
    echo $datacard
    workspace=workspaces/$(basename ${datacard%%.dat})_workspace.root
    result=results/$(basename ${datacard%%.dat})_result.root
    text2workspace.py $datacard -o $workspace
    combine -M MultiDimFit -d $workspace -m 125 -t -1 --expectSignal=0 --setParameters r_VBSWH_mKW=0 --setParameterRanges r_VBSWH_mKW=0.0,2.0 --saveNLL --algo grid --points 101 --rMin 0 --rMax 5 --alignEdges 1
    mv higgsCombineTest.MultiDimFit.mH125.root $result
done
