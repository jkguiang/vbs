# Instructions

## Installing HiggsCombine
1. Set up CMSSW and clone HiggsCombine
```
export SCRAM_ARCH=slc7_amd64_gcc700
cmsrel CMSSW_10_2_13
cd CMSSW_10_2_13/src
cmsenv
git clone https://github.com/cms-analysis/HiggsAnalysis-CombinedLimit.git HiggsAnalysis/CombinedLimit
cd HiggsAnalysis/CombinedLimit
```
2. Check out a stable tag (v8.2.0 [recommended](http://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/) at the time of writing)
```
cd $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit
git fetch origin
git checkout v8.2.0
scramv1 b clean; scramv1 b # always make a clean build
```

## Producing the final result
1. Turn the datacard into a workspace
```
text2workspace.py  datacards/vbswh.dat -o vbswh.root
```
2. Run the following HiggsCombine command (`MultiDimFit` options can be found [here](http://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part3/commonstatsmethods/))
```
combine -M MultiDimFit -d vbswh.root -m 125 -t -1 --expectSignal=0 --setParameters r_VBSWH_mKW=0 --setParameterRanges r_VBSWH_mKW=0.0,2.0 --saveNLL --algo grid --points 101 --rMin 0 --rMax 5 --alignEdges 1
```
3. Check the output for the negative-log-likelihood data
```
$ root higgsCombineTest.MultiDimFit.mH125.root
root [0]
Attaching file higgsCombineTest.MultiDimFit.mH125.root as _file0...
(TFile *) 0x24b2ff0
root [1] limit->Scan("r:deltaNLL")
************************************
*    Row   *         r *  deltaNLL *
************************************
*        0 * 0.0005723 *         0 *
*        1 *         0 * -6.40e-05 *
*        2 * 0.0500000 * 0.3063060 *
*        3 * 0.1000000 * 1.1951272 *
*        4 * 0.1500000 * 2.4876313 *
*        5 * 0.2000000 * 3.9961650 *
*        6 *      0.25 * 5.6021165 *
*        7 * 0.3000000 * 7.2401137 *
*        8 * 0.3499999 * 8.8748912 *
*        9 * 0.4000000 * 10.487948 *
*       10 * 0.4499999 * 12.069160 *
*       11 *       0.5 * 13.613980 *
*       12 * 0.5500000 * 15.120312 *
*       13 * 0.6000000 * 16.587787 *
*       14 * 0.6499999 * 18.016937 *
*       15 * 0.6999999 * 19.408830 *
*       16 *      0.75 * 20.764822 *
*       17 * 0.8000000 * 22.086277 *
*       18 * 0.8500000 * 23.374904 *
*       19 * 0.8999999 * 24.632112 *
*       20 * 0.9499999 * 25.859384 *
*       21 *         1 * 27.058136 *
*       22 * 1.0499999 * 28.229707 *
*       23 * 1.1000000 * 29.375375 *
*       24 * 1.1499999 * 30.496334 *
Type <CR> to continue or q to quit ==> q
```
