# Note: make sure CMSSW is > 11.X so that ROOT is available in Python 3

RESULTS=$1

if [[ "$RESULTS" == "" ]]; then
    echo "ERROR: no results directory provided"
    exit 1
fi

echo "Collecting limits..."
python3 CollectC2VLimits.py $RESULTS
echo "Plotting limits..."
python3 PlotC2VLimits.py $RESULTS
