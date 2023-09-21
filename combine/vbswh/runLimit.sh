# Make workspace
rm -f vbswh.root
text2workspace.py datacards/vbswh.dat -o vbswh.root
# Run limit
combine -M MultiDimFit -d vbswh.root -m 125 -t -1 --expectSignal=0 --setParameters r=0 --setParameterRanges r=0.0,2.0 --saveNLL --algo grid --points 101 --rMin 0 --rMax 5 --alignEdges 1 --name _vbswh
