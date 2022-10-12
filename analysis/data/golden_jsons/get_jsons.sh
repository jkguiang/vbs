#!/bin/bash

jsons="
/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/Legacy_2018/Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON.txt 
/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/Legacy_2017/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt
/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/Legacy_2016/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt 
"

scp_jsons=""
for json in $jsons; do
    scp_jsons="$scp_jsons,$json"
done
scp_jsons=${scp_jsons:1}

username=$(read -p "LXPLUS Username: ")
if [[ "$username" == "" ]]; then
    username=$USER
fi

scp $username@lxplus.cern.ch:\{$scp_jsons\} .

# Run silly SNT script
for json in $jsons; do
    python2 ../../NanoTools/NanoCORE/Tools/jsontojson.py --filename=$(basename $json)
done
