cp -R /cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/* .
for json in $(ls */*/*.json.gz); do 
    echo $json
    gunzip $json
done
