cp vbswh.root vbswh_impacts.root
combineTool.py -M Impacts -d vbswh_impacts.root -m 125 --doInitialFit --robustFit 1
combineTool.py -M Impacts -d vbswh_impacts.root -m 125 --robustFit 1 --doFits
combineTool.py -M Impacts -d vbswh_impacts.root -m 125 -o impacts.json
plotImpacts.py -i impacts.json -o impacts
cp impacts.pdf $HOME/public_html/vbswh_plots/limits/impacts.pdf
