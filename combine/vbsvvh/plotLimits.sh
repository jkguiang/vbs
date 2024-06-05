# Note: make sure CMSSW is > 11.X so that ROOT is available in Python 3

echo "Collecting limits..."
python3 CollectC2VLimits.py
echo "Plotting limits..."
python3 PlotC2VLimits.py
