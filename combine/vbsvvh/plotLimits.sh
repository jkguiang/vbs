# Note: run this in a "fresh" terminal!

cwd=${PWD}
cd /cvmfs/cms.cern.ch/slc7_amd64_gcc900/cms/cmssw/CMSSW_12_2_0/src
cmsenv
cd $cwd
echo "Now using CMSSW v12.2.0"

echo "Collecting limits..."
python3 CollectC2VLimits.py
echo "Plotting limits..."
python3 PlotC2VLimits.py
