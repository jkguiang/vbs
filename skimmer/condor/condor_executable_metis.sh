#!/bin/bash

# Parse the arguments
OUTPUTDIR=$1
OUTPUTNAME=$2
INPUTFILENAMES=$3
IFILE=$4
CMSSWVERSION=$5
SCRAMARCH=$6

function getjobad {
    grep -i "^$1" "$_CONDOR_JOB_AD" | cut -d= -f2- | xargs echo
}
function setup_chirp {
    if [ -e ./condor_chirp ]; then
    # Note, in the home directory
        mkdir chirpdir
        mv condor_chirp chirpdir/
        export PATH="$PATH:$(pwd)/chirpdir"
        echo "[chirp] Found and put condor_chirp into $(pwd)/chirpdir"
    elif [ -e /usr/libexec/condor/condor_chirp ]; then
        export PATH="$PATH:/usr/libexec/condor"
        echo "[chirp] Found condor_chirp in /usr/libexec/condor"
    else
        echo "[chirp] No condor_chirp :("
    fi
}
function chirp {
    # Note, $1 (the classad name) must start with Chirp
    condor_chirp set_job_attr_delayed $1 $2
    ret=$?
    echo "[chirp] Chirped $1 => $2 with exit code $ret"
}
function stageout {
    COPY_SRC=$1
    COPY_DEST=$2
    retries=0
    COPY_STATUS=1
    until [ $retries -ge 10 ]
    do
        echo "Stageout attempt $((retries+1)): env -i X509_USER_PROXY=${X509_USER_PROXY} gfal-copy -p -f -t 7200 --verbose --checksum ADLER32 ${COPY_SRC} ${COPY_DEST}"
        env -i X509_USER_PROXY=${X509_USER_PROXY} gfal-copy -p -f -t 7200 --verbose --checksum ADLER32 ${COPY_SRC} ${COPY_DEST}
        COPY_STATUS=$?
        if [ $COPY_STATUS -ne 0 ]; then
            echo "Failed stageout attempt $((retries+1))"
        else
            echo "Successful stageout with $retries retries"
            break
        fi
        retries=$[$retries+1]
        echo "Sleeping for 5m"
        sleep 5m
    done
    if [ $COPY_STATUS -ne 0 ]; then
        echo "Removing output file because gfal-copy crashed with code $COPY_STATUS"
        env -i X509_USER_PROXY=${X509_USER_PROXY} gfal-rm --verbose ${COPY_DEST}
        REMOVE_STATUS=$?
        if [ $REMOVE_STATUS -ne 0 ]; then
            echo "Uhh, gfal-copy crashed and then the gfal-rm also crashed with code $REMOVE_STATUS"
            echo "You probably have a corrupt file sitting on hadoop now."
            exit 1
        fi
    fi
}

# Fetch custom ClassAds values
XROOTDHOST="$(getjobad XRootDHost)"
XROOTDPROTOCOL="$(getjobad XRootDProtocol)"
USEPYTHON2="$(getjobad UsePython2)"
SCITOKENFILENAME="$(getjobad SciTokenFilename)"

if [[ "${SCITOKENFILENAME}" != "" ]]; then
    export BEARER_TOKEN_FILE="$PWD/$SCITOKENFILENAME"
fi

# Make sure OUTPUTNAME doesn't have .root since we add it manually
OUTPUTNAME=$(echo $OUTPUTNAME | sed 's/\.root//')

setup_chirp

echo -e "\n--- begin header output ---\n" #                     <----- section division
echo "OUTPUTDIR: $OUTPUTDIR"
echo "OUTPUTNAME: $OUTPUTNAME"
echo "INPUTFILENAMES: $INPUTFILENAMES"
echo "IFILE: $IFILE"
echo "CMSSWVERSION: $CMSSWVERSION"
echo "SCRAMARCH: $SCRAMARCH"

echo "BEARER_TOKEN_FILE: $BEARER_TOKEN_FILE"
echo "GLIDEIN_CMSSite: $GLIDEIN_CMSSite"
echo "hostname: $(hostname)"
echo "uname -a: $(uname -a)"
echo "time: $(date +%s)"
echo "args: $@"

echo -e "\n--- end header output ---\n" #                       <----- section division

echo "before unpacking: ls -lrth"
ls -lrth 

if [ -r "$OSGVO_CMSSW_Path"/cmsset_default.sh ]; then
    echo "sourcing environment: source $OSGVO_CMSSW_Path/cmsset_default.sh"
    source "$OSGVO_CMSSW_Path"/cmsset_default.sh
elif [ -r "$OSG_APP"/cmssoft/cms/cmsset_default.sh ]; then
    echo "sourcing environment: source $OSG_APP/cmssoft/cms/cmsset_default.sh"
    source "$OSG_APP"/cmssoft/cms/cmsset_default.sh
elif [ -r /cvmfs/cms.cern.ch/cmsset_default.sh ]; then
    echo "sourcing environment: source /cvmfs/cms.cern.ch/cmsset_default.sh"
    source /cvmfs/cms.cern.ch/cmsset_default.sh
else
    echo "ERROR! Couldn't find $OSGVO_CMSSW_Path/cmsset_default.sh or /cvmfs/cms.cern.ch/cmsset_default.sh or $OSG_APP/cmssoft/cms/cmsset_default.sh"
    exit 1
fi

# Setup environment and build
export SCRAM_ARCH=${SCRAMARCH} && scramv1 project CMSSW ${CMSSWVERSION}
cd ${CMSSWVERSION}/src/
eval `scramv1 runtime -sh`
tar xvf ../../package.tar.gz
cd NanoAODTools/
bash standalone/env_standalone.sh build
source standalone/env_standalone.sh

echo "before running: ls -lrth"
ls -lrth 

echo -e "\n--- begin running ---\n" #                           <----- section division

# Edit .rootrc
cat > ${HOME}/.rootrc << EOL
Davix.GSI.CACheck: n
EOL

#------------------------------------------------------------------------------------------------------------------------------>
cat > check_xrd.C << EOL
void check_xrd(TString filename) { 
    TFile* f = TFile::Open(filename.Data()); 
    TTree* t = (TTree*) f->Get("Events"); 
    ((TTreePlayer*)(t->GetPlayer()))->SetScanRedirect(true); 
    ((TTreePlayer*)(t->GetPlayer()))->SetScanFileName("output.dat"); 
    t->Scan("*", "", "", 10); 
}
EOL

cat > count_all_events.C << EOL
void count_all_events(TString filename) { 
    TFile* f = TFile::Open(filename.Data()); 
    TTree* Events = (TTree*) f->Get("Events"); 
    std::cout << Events->GetEntries() << std::endl; 
    std::cout << Events->GetEntries() << std::endl;  
    std::cout << Events->GetEntries() << std::endl; 
}
EOL

cat > count_eff_events.C << EOL
void count_eff_events(TString filename) { 
    TFile* f = TFile::Open(filename.Data()); 
    TTree* Events = (TTree*) f->Get("Events"); 
    std::cout << Events->Draw("(Generator_weight>0)-(Generator_weight<0)", "", "goff") << std::endl; 
    std::cout << Events->Draw("(Generator_weight>0)-(Generator_weight<0)", "((Generator_weight>0)-(Generator_weight<0))>0", "goff") << std::endl;  
    std::cout << Events->Draw("(Generator_weight>0)-(Generator_weight<0)", "((Generator_weight>0)-(Generator_weight<0))<0", "goff") << std::endl; 
}
EOL
#------------------------------------------------------------------------------------------------------------------------------>

#------------------------------------------------------------------------------------------------------------------------------>
localpath=$(echo ${INPUTFILENAMES} | sed 's/^.*\(\/store.*\).*$/\1/')
if [[ "${XROOTDPROTOCOL}" == *"http"* ]]; then
    INPUTFILE=${XROOTDPROTOCOL}://${XROOTDHOST}${localpath}
else
    INPUTFILE=${XROOTDPROTOCOL}://${XROOTDHOST}/${localpath}
fi
#------------------------------------------------------------------------------------------------------------------------------>

echo "Checking XRootD host's health..." | tee >(cat >&2)
root -l -b -q check_xrd.C\(\"${INPUTFILE}\"\) > >(tee check_xrd.txt) 2> >(tee check_xrd_stderr.txt >&2)
rm -f output.dat # Delete the file as it is not needed

# If the file had error
if grep -q "badread" check_xrd_stderr.txt; then
    echo "ERROR: bad read, dumping stderr logs below."
    cat check_xrd_stderr.txt
    exit 1
else
    echo "XRootD host seems to be in working order :)"
fi

# Figuring out nevents and neff_weights
# Note: Relies on "/NANOAOD/" being present for data files. Perhaps not the brightest idea, however it seems to work for now.
if [[ "${INPUTFILE}" == *"/NANOAOD/"* ]]; then
    echo "Running count_all_events on all events" | tee >(cat >&2)
    root -l -b -q count_all_events.C\(\"${INPUTFILE}\"\) > >(tee nevents.txt) 2> >(tee nevents_stderr.txt >&2)
else
    echo "Running count_eff_events on all events" | tee >(cat >&2)
    root -l -b -q count_eff_events.C\(\"${INPUTFILE}\"\) > >(tee nevents.txt) 2> >(tee nevents_stderr.txt >&2)
fi

RUN_STATUS=$?

if [[ $RUN_STATUS != 0 ]]; then
    echo "Error: event-counting macro on all events crashed with exit code $?" | tee >(cat >&2)
    echo "Exiting..."
    exit 1
fi

# Run the postprocessor
if [[ "$USEPYTHON2" != "" ]]; then PYTHONX="python2"; else PYTHONX="python3"; fi
CMD="$PYTHONX scripts/nano_postproc.py \
    ./ \
    ${INPUTFILE} \
    -b python/postprocessing/examples/keep_and_drop.txt \
    -I PhysicsTools.NanoAODTools.postprocessing.examples.skimModule skimModuleConstr"
echo $CMD
echo "Running nano_postproc.py" | tee >(cat >&2)
$CMD > >(tee nano_postproc.txt) 2> >(tee nano_postproc_stderr.txt >&2)

RUN_STATUS=$?

if [[ $RUN_STATUS != 0 ]]; then
    echo "Removing output file because scripts/nano_postproc.py crashed with exit code $?"
    rm ${NANOPOSTPROCOUTPUTFILENAME}_Skim.root
    echo "Exiting..."
    exit 1
fi

# Rename the output file
NANOPOSTPROCOUTPUTFILE=$(basename ${localpath})
NANOPOSTPROCOUTPUTFILENAME=${NANOPOSTPROCOUTPUTFILE%.root}
echo "Renaming the output file"
echo "mv ${NANOPOSTPROCOUTPUTFILENAME}_Skim.root output.root"
mv ${NANOPOSTPROCOUTPUTFILENAME}_Skim.root output.root

# Figuring out nevents and neff_weights
# Note: Relies on "/NANOAOD/" being present for data files. Perhaps not the brightest idea, however it seems to work for now.
if [[ "${INPUTFILE}" == *"/NANOAOD/"* ]]; then
    echo "Running count_all_events on skimmed events" | tee >(cat >&2)
    root -l -b -q count_all_events.C\(\"output.root\"\) > >(tee nevents_skimmed.txt) 2> >(tee nevents_skimmed_stderr.txt >&2)
else
    echo "Running count_eff_events on skimmed events" | tee >(cat >&2)
    root -l -b -q count_eff_events.C\(\"output.root\"\) > >(tee nevents_skimmed.txt) 2> >(tee nevents_skimmed_stderr.txt >&2)
fi

RUN_STATUS=$?

if [[ $RUN_STATUS != 0 ]]; then
    echo "Error: event-counting macro on skimmed events crashed with exit code $?" | tee >(cat >&2)
    echo "Exiting..."
    exit 1
fi

echo -e "\n--- end running ---\n" #                             <----- section division

echo "after running: ls -lrth"
ls -lrth

echo -e "\n--- begin copying output ---\n" #                    <----- section division
echo "Sending output file $OUTPUTNAME.root"
# Get local filepath name
OUTPUTDIRPATHNEW=$(echo ${OUTPUTDIR} | sed 's/^.*\(\/store.*\).*$/\1/')

# Copying the output file
COPY_SRC="file://`pwd`/${OUTPUTNAME}.root"
COPY_DEST="davs://redirector.t2.ucsd.edu:1094//${OUTPUTDIRPATHNEW}/${OUTPUTNAME}_${IFILE}.root"
stageout $COPY_SRC $COPY_DEST

# Copying n events
COPY_SRC="file://`pwd`/nevents.txt"
COPY_DEST="davs://redirector.t2.ucsd.edu:1094//${OUTPUTDIRPATHNEW}/${OUTPUTNAME}_${IFILE}_nevents.txt"
stageout $COPY_SRC $COPY_DEST

echo -e "\n--- end copying output ---\n" #                    <----- section division


echo -e "\n--- begin cleaning area ---\n" #                    <----- section division

echo "rm -rf mc/"
rm -rf mc/

echo "rm -rf data/"
rm -rf data/

echo -e "\n--- end cleaning output ---\n" #                    <----- section division
