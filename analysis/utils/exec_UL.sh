# Help
usage()
{
  echo "ERROR - Usage:"
  echo
  echo "      sh $(basename $0) STUDYNAME SKIMVERSION BABYVERSION SPECIFICSAMPLE [DEBUG]"
  echo
  exit
}

if [ -z ${1} ]; then usage; fi
STUDY=${1}
if [ -z ${2} ]; then usage; fi
SKIMVERSION=${2}
if [ -z ${3} ]; then usage; fi
BABYVERSION=${3}

SPECIFICSAMPLE=${4}
# Third argument is debug
DEBUG=${5}

LOGFILE=.${STUDY}_${SKIMVERSION}_${BABYVERSION}_debug${DEBUG}.log

rm -f ${LOGFILE}

# YEARS="2016 2017 2018"
YEARS="2017 2018"

EXECUTABLE=./studies/${STUDY}/doAnalysis

if [[ ${SPECIFICSAMPLE} == *"all"* ]]; then
    SAMPLES="TTTo2L2Nu \
    TTToSemiLeptonic \
    ST_tW_antitop_5f \
    ST_tW_top_5f \
    ST_s-channel_4f \
    ST_t-channel_antitop_4f \
    ST_t-channel_top_4f \
    TTWJetsToLNu \
    TTZToLLNuNu_M-10 \
    DYJetsToLL_M-50 \
    DYJetsToLL_M-10to50 \
    WJetsToLNu \
    SSWW \
    WW \
    WZTo3LNu \
    ZZTo4L \
    ZZ \
    ttHJetToNonbb \
    ttHJetTobb \
    TWZToLL_thad_Wlept_5f_DR \
    TWZToLL_tlept_Whad_5f_DR \
    TTWW \
    TTWZ \
    VBSWWHToLNuLNubb_C2V \
    VBSWWHToLNuLNubb_CV \
    VBSWWHToLNuLNubb_C3 \
    DoubleEG_Run2017 \
    DoubleMuon_Run2017 \
    MuonEG_Run2017 \
    SingleElectron_Run2017 \
    SingleMuon_Run2017 \
    EGamma_Run2018 \
    DoubleMuon_Run2018 \
    MuonEG_Run2018 \
    SingleMuon_Run2018"
else
    SAMPLES=${SPECIFICSAMPLE}
fi

NANOSKIMDIR=/nfs-7/userdata/phchang/VBSHWWNanoSkim_${SKIMVERSION}/

rm -f .jobs.txt

for SAMPLE in ${SAMPLES}; do

    XSEC=""

    for YEAR in ${YEARS}; do

        HISTDIR=hists/${SKIMVERSION}/${BABYVERSION}/${STUDY}_${YEAR}
        mkdir -p ${HISTDIR}

        # if [[ ${YEAR} == *"2016"* ]]; then NANOTAG=RunIISummer16NanoAOD*; fi
        if [[ ${YEAR} == *"2017"* ]]; then NANOTAG=RunIISummer*UL17NanoAODv2*; fi
        if [[ ${YEAR} == *"2018"* ]]; then NANOTAG=RunIISummer*UL18NanoAODv2*; fi

        if [[ ${SAMPLE} == *"Run201"* ]]; then NANOTAG=""; fi

        if [[ ${SAMPLE} == *"WW"* ]]; then XSEC=118.71; fi # This line needs to be at the front
        if [[ ${SAMPLE} == *"ZZ"* ]]; then XSEC=10.32; fi # This line needs to be at the front
        if [[ ${SAMPLE} == *"tZq_ll_4f_ckm_NLO"* ]]; then XSEC=0.0758; fi
        if [[ ${SAMPLE} == *"ST_s-channel_4f"* ]]; then XSEC=3.74; fi
        if [[ ${SAMPLE} == *"ST_t-channel_antitop_4f"* ]]; then XSEC=80.95; fi
        if [[ ${SAMPLE} == *"ST_t-channel_top_4f"* ]]; then XSEC=136.02; fi
        if [[ ${SAMPLE} == *"ST_tW_antitop_5f"* ]]; then XSEC=19.559; fi
        if [[ ${SAMPLE} == *"ST_tW_top_5f"* ]]; then XSEC=19.559; fi
        if [[ ${SAMPLE} == *"ST_tWll_5f_LO"* ]]; then XSEC=0.01123; fi
        if [[ ${SAMPLE} == *"TWZToLL_tlept_Whad_5f_DR"* ]]; then XSEC=0.003004; fi
        if [[ ${SAMPLE} == *"TWZToLL_thad_Wlept_5f_DR"* ]]; then XSEC=0.003004; fi
        if [[ ${SAMPLE} == *"ZZTo4L"* ]]; then XSEC=1.256; fi
        if [[ ${SAMPLE} == *"GluGluHToZZTo4L"* ]]; then XSEC=0.0082323; fi
        if [[ ${SAMPLE} == *"DYJetsToLL_M-10to50"* ]]; then XSEC=20657.0; fi
        if [[ ${SAMPLE} == *"DYJetsToLL_M-50"* ]]; then XSEC=6198.0; fi
        if [[ ${SAMPLE} == *"WJetsToLNu"* ]]; then XSEC=61335.0; fi
        if [[ ${SAMPLE} == *"TTWJetsToLNu"* ]]; then XSEC=0.2043; fi
        if [[ ${SAMPLE} == *"TTZToLLNuNu_M-10"* ]]; then XSEC=0.2529; fi
        if [[ ${SAMPLE} == *"TTZToLL_M-1to10"* ]]; then XSEC=0.2529; fi
        if [[ ${SAMPLE} == *"TTJets_DiLept"* ]]; then XSEC=91.044; fi
        if [[ ${SAMPLE} == *"TTJets_SingleLeptFromT"* ]]; then XSEC=182.96; fi
        if [[ ${SAMPLE} == *"TTJets_SingleLeptFromTbar"* ]]; then XSEC=182.96; fi
        if [[ ${SAMPLE} == *"TTTo2L2Nu"* ]]; then XSEC=87.315; fi
        if [[ ${SAMPLE} == *"TTToSemiLeptonic"* ]]; then XSEC=365.34; fi
        if [[ ${SAMPLE} == *"WpWpJJ_EWK"* ]]; then XSEC=0.0539; fi
        if [[ ${SAMPLE} == *"SSWW"* ]]; then XSEC=0.02794; fi
        if [[ ${SAMPLE} == *"WZTo3LNu"* ]]; then XSEC=4.4297; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_4p5_TuneCP5"* ]]; then XSEC=0.01021; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_4_TuneCP5"* ]]; then XSEC=0.007546; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_3_TuneCP5"* ]]; then XSEC=0.003559; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_1_TuneCP5"* ]]; then XSEC=0.0002943; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_0_TuneCP5"* ]]; then XSEC=0.001037; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_m1_TuneCP5"* ]]; then XSEC=0.003359; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_m2_TuneCP5"* ]]; then XSEC=0.007276; fi
        if [[ ${SAMPLE} == *"VBSWmpWmpH_C2V_m2p5_TuneCP5"* ]]; then XSEC=0.009797; fi
        if [[ ${SAMPLE} == *"VBSWWHToLNuLNubb_C2V"* ]]; then XSEC=0.0009246749816; fi # 0.01483 * 0.3272 * 0.3272 * 0.5824
        if [[ ${SAMPLE} == *"VBSWWHToLNuLNubb_CV"* ]]; then XSEC=0.0009246749816; fi # 0.01483 * 0.3272 * 0.3272 * 0.5824 # This is becuase the default was set to C2V = 4.5 and the rest was reweighted
        if [[ ${SAMPLE} == *"VBSWWHToLNuLNubb_C3"* ]]; then XSEC=0.001364877636; fi # 0.02189 * 0.3272 * 0.3272 * 0.5824
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_0.log     : Cross-section : 0.001037  + - 2.915e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_0p5.log   : Cross-section : 0.0004695 + - 1.431e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_1.log     : Cross-section : 0.0002943 + - 1.232e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_1p5.log   : Cross-section : 0.0005193 + - 1.475e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_2.log     : Cross-section : 0.001137  + - 3.029e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_2p5.log   : Cross-section : 0.002152  + - 6.152e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_3.log     : Cross-section : 0.003559  + - 9.886e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_3p5.log   : Cross-section : 0.00538   + - 2.129e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_4.log     : Cross-section : 0.007546  + - 2.186e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_4p5.log   : Cross-section : 0.01021   + - 3.775e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_5.log     : Cross-section : 0.01317   + - 5.58e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_5p5.log   : Cross-section : 0.01645   + - 5.464e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_6.log     : Cross-section : 0.02032   + - 6.382e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m0p5.log  : Cross-section : 0.002008  + - 6.648e-06 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m1.log    : Cross-section : 0.003359  + - 1.015e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m1p5.log  : Cross-section : 0.005077  + - 1.826e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m2.log    : Cross-section : 0.007276  + - 2.386e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m2p5.log  : Cross-section : 0.009797  + - 3.6e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m3.log    : Cross-section : 0.01277   + - 4.599e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m3p5.log  : Cross-section : 0.01614   + - 6.389e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_m4.log    : Cross-section : 0.01981   + - 6.805e-05 pb
        # VBSHWWBabyLooper/mystudies/genULsamples/gridpacks/VBSWWH_incl_v2_C2V_rewgt.log : Cross-section : 0.007546  + - 2.186e-05 pb
        if [[ ${SAMPLE} == *"ttHJetTobb"* ]]; then XSEC=0.1279; fi
        if [[ ${SAMPLE} == *"ttHJetToNonbb"* ]]; then XSEC=0.215; fi
        if [[ ${SAMPLE} == *"TTTT"* ]]; then XSEC=0.009103; fi
        if [[ ${SAMPLE} == *"TTWW"* ]]; then XSEC=0.0115; fi
        if [[ ${SAMPLE} == *"TTWZ"* ]]; then XSEC=0.003884; fi
        if [[ ${SAMPLE} == *"TTZZ"* ]]; then XSEC=0.001982; fi
        if [[ ${SAMPLE} == *"TTWH"* ]]; then XSEC=0.001582; fi
        if [[ ${SAMPLE} == *"TTZH"* ]]; then XSEC=0.001535; fi
        if [[ ${SAMPLE} == *"WWW_4F"* ]]; then XSEC=0.2086; fi
        if [[ ${SAMPLE} == *"WWZ"* ]]; then XSEC=0.1651; fi
        if [[ ${SAMPLE} == *"WZG"* ]]; then XSEC=0.04123; fi
        if [[ ${SAMPLE} == *"WZZ"* ]]; then XSEC=0.05565; fi
        if [[ ${SAMPLE} == *"ZZZ"* ]]; then XSEC=0.01398; fi
        if [[ ${SAMPLE} == *"WWToLNuQQ"* ]]; then XSEC=49.997; fi
        if [[ ${SAMPLE} == *"WWTo2L2Nu"* ]]; then XSEC=12.178; fi
        if [[ ${SAMPLE} == *"Run201"* ]]; then XSEC=1; fi # data

        if [[ -z "${XSEC}" ]]; then
            echo ${SAMPLE} cross section missing!
            echo "here is my guess"
            grep ${SAMPLE} NanoTools/NanoCORE/datasetinfo/scale1fbs.txt | awk '{print $1, $5}'
            exit
        fi

        if [[ ${SAMPLE} == *"Run201"* ]]; then
            if [[ ${SAMPLE} == *"${YEAR}"* ]]; then
                :
            else
                continue
            fi
        fi

        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_4p5_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_4_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_3_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_1_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_0_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_m1_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_m2_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWmpWmpHToLNuLNu_C2V_m2p5_TuneCP5_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWWHToLNuLNubb_C2V_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWWHToLNuLNubb_CV_2016"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"VBSWWHToLNuLNubb_C3_2016"* ]]; then continue; fi

        if [[ ${SAMPLE}_${YEAR} == *"ZZ_2018"* ]]; then continue; fi
        if [[ ${SAMPLE}_${YEAR} == *"ZZTo4L_2017"* ]]; then continue; fi

        if [[ ${STUDY} == "os" ]]; then
            if [[ ${SAMPLE}_${YEAR} == *"SingleElectron_Run2016_2016"* ]]; then continue; fi  #   803.8
            if [[ ${SAMPLE}_${YEAR} == *"SingleElectron_Run2017_2017"* ]]; then continue; fi  #   971.4
            if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2016_2016"* ]]; then continue; fi      #   1751.2
            if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2017_2017"* ]]; then continue; fi      #   2026.6
            if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2018_2018"* ]]; then continue; fi      #   2765.8
        fi

        EXTRATAG=""
        # if [[ ${SAMPLE} == *"VBSWmpWmpHToLNuLNu_C2V_4p5_TuneCP5"* ]]; then EXTRATAG=ext1; fi
        # if [[ ${SAMPLE}_${YEAR} == *"TTToSemiLeptonic_2018"* ]]; then EXTRATAG=ext3; fi
        # if [[ ${SAMPLE}_${YEAR} == *"TTToSemiLeptonic_2017"* ]]; then EXTRATAG=ext1; fi

        # Last bit modification
        if [[ ${SAMPLE} == *"Run201"* ]]; then
            XSEC=1;
            SAMPLEWITHUNDERSCORE=${SAMPLE} # Data does not get the underscore
        else
            # If it is not data then the SAMPLE string gets a "_" subscript in order to loop over them
            SAMPLEWITHUNDERSCORE=${SAMPLE}_
        fi

        NEVENTSINFOFILE=${NANOSKIMDIR}/${SAMPLEWITHUNDERSCORE}*${NANOTAG}*${EXTRATAG}*/merged/nevents.txt
        NFILES_NEVENTSINFOFILE=$(ls ${NEVENTSINFOFILE} | wc -l)
        if [[ ${SAMPLE} == *"Run201"* ]]; then
            NTOTALEVENTS=1
            NEFFEVENTS=1
            SCALE1FB=1
        else
            if [ "${NFILES_NEVENTSINFOFILE}" -gt "1" ]; then
                NTOTALEVENTS=0
                NEFFEVENTS=0
                for i in $(ls ${NEVENTSINFOFILE}); do
                    TMPNTOTALEVENTS=$(head -n1 ${i})
                    TMPNEFFEVENTS=$(tail -n1 ${i})
                    NTOTALEVENTS=$(($NTOTALEVENTS + $TMPNTOTALEVENTS))
                    NEFFEVENTS=$(($NEFFEVENTS + $TMPNEFFEVENTS))
                done
                SCALE1FB=$(echo "${XSEC} / ${NEFFEVENTS} * 1000" | bc -l)
            else
                NTOTALEVENTS=$(head -n1 ${NEVENTSINFOFILE})
                NEFFEVENTS=$(tail -n1 ${NEVENTSINFOFILE})
                SCALE1FB=$(echo "${XSEC} / ${NEFFEVENTS} * 1000" | bc -l)
            fi
        fi

        echo "" >> ${LOGFILE}
        echo "==========================================================================================" >> ${LOGFILE}
        echo "Preparing command lines to process ..." >> ${LOGFILE}
        echo "Sample                            :" ${SAMPLE} >> ${LOGFILE}
        echo "Year                              :" ${YEAR} >> ${LOGFILE}
        echo "Nano tag                          :" ${NANOTAG} >> ${LOGFILE}
        echo "N events information file         :" ${NEVENTSINFOFILE} >> ${LOGFILE}
        echo "N total events                    :" ${NTOTALEVENTS} >> ${LOGFILE}
        echo "N eff total events (i.e. pos-neg) :" ${NEFFEVENTS} >> ${LOGFILE}
        echo "Cross section (pb)                :" ${XSEC} >> ${LOGFILE}
        echo "Scale1fb                          :" ${SCALE1FB} >> ${LOGFILE}
        echo "" >> ${LOGFILE}

        #
        # More than 1 jobs
        #
        NJOBS=1
        if [[ ${SAMPLE}_${YEAR} == *"ttHToNonbb_2016"* ]]; then NJOBS=2; fi
        if [[ ${SAMPLE}_${YEAR} == *"ttHToNonbb_2017"* ]]; then NJOBS=4; fi
        if [[ ${SAMPLE}_${YEAR} == *"ttHToNonbb_2018"* ]]; then NJOBS=4; fi
        if [[ ${SAMPLE}_${YEAR} == *"ttHJetToNonbb_2018"* ]]; then NJOBS=4; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTWJetsToLNu_2016"* ]]; then NJOBS=6; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTWJetsToLNu_2017"* ]]; then NJOBS=12; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTWJetsToLNu_2018"* ]]; then NJOBS=12; fi
        if [[ ${SAMPLE}_${YEAR} == *"WZTo3LNu_2016"* ]]; then NJOBS=1; fi
        if [[ ${SAMPLE}_${YEAR} == *"WZTo3LNu_2017"* ]]; then NJOBS=6; fi
        if [[ ${SAMPLE}_${YEAR} == *"WZTo3LNu_2018"* ]]; then NJOBS=6; fi
        if [[ ${SAMPLE}_${YEAR} == *"tZq_ll_4f_ckm_NLO_2016"* ]]; then NJOBS=7; fi
        if [[ ${SAMPLE}_${YEAR} == *"tZq_ll_4f_ckm_NLO_2017"* ]]; then NJOBS=7; fi
        if [[ ${SAMPLE}_${YEAR} == *"tZq_ll_4f_ckm_NLO_2018"* ]]; then NJOBS=7; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTZToLLNuNu_M-10_2016"* ]]; then NJOBS=7; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTZToLLNuNu_M-10_2017"* ]]; then NJOBS=15; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTZToLLNuNu_M-10_2018"* ]]; then NJOBS=28; fi
        if [[ ${SAMPLE}_${YEAR} == *"TWZToLL_thad_Wlept_5f_DR_2017"* ]]; then NJOBS=3; fi
        if [[ ${SAMPLE}_${YEAR} == *"TWZToLL_tlept_Whad_5f_DR_2018"* ]]; then NJOBS=3; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTTT_2017"* ]]; then NJOBS=2; fi
        if [[ ${SAMPLE}_${YEAR} == *"TTTT_2018"* ]]; then NJOBS=7; fi
        if [[ ${SAMPLE}_${YEAR} == *"ZZTo4L_2016"* ]]; then NJOBS=2; fi
        if [[ ${SAMPLE}_${YEAR} == *"ZZTo4L_2017"* ]]; then NJOBS=20; fi
        if [[ ${SAMPLE}_${YEAR} == *"ZZTo4L_2018"* ]]; then NJOBS=20; fi

        if [[ ${STUDY} == "os" ]]; then
            if [[ ${SAMPLE}_${YEAR} == *"DYJetsToLL_M-50_2016"* ]]; then NJOBS=10; fi        #   1886.3
            if [[ ${SAMPLE}_${YEAR} == *"DYJetsToLL_M-50_2016"* ]]; then NJOBS=10; fi        #   1900.1
            if [[ ${SAMPLE}_${YEAR} == *"DYJetsToLL_M-50_2016"* ]]; then NJOBS=5; fi         #   946.1
            if [[ ${SAMPLE}_${YEAR} == *"DoubleEG_Run2016_2016"* ]]; then NJOBS=5; fi        #   1031.0
            if [[ ${SAMPLE}_${YEAR} == *"DoubleEG_Run2017_2017"* ]]; then NJOBS=6; fi        #   1201.3
            if [[ ${SAMPLE}_${YEAR} == *"EGamma_Run2018_2018"* ]]; then NJOBS=9; fi          #   1831.5
            if [[ ${SAMPLE}_${YEAR} == *"DoubleMuon_Run2016_2016"* ]]; then NJOBS=11; fi     #   2208.1
            if [[ ${SAMPLE}_${YEAR} == *"DoubleMuon_Run2017_2017"* ]]; then NJOBS=12; fi     #   2449.8
            if [[ ${SAMPLE}_${YEAR} == *"DoubleMuon_Run2018_2018"* ]]; then NJOBS=35; fi     #   3474.0
            # if [[ ${SAMPLE}_${YEAR} == *"SingleElectron_Run2016_2016"* ]]; then NJOBS=4; fi  #   803.8
            # if [[ ${SAMPLE}_${YEAR} == *"SingleElectron_Run2017_2017"* ]]; then NJOBS=5; fi  #   971.4
            # if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2016_2016"* ]]; then NJOBS=9; fi      #   1751.2
            # if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2017_2017"* ]]; then NJOBS=10; fi     #   2026.6
            # if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2018_2018"* ]]; then NJOBS=10; fi     #   2765.8
            if [[ ${SAMPLE}_${YEAR} == *"TTJets_DiLept_2016"* ]]; then NJOBS=2; fi           #   247.9
            if [[ ${SAMPLE}_${YEAR} == *"TTJets_DiLept_2017"* ]]; then NJOBS=5; fi           #   930.3
            if [[ ${SAMPLE}_${YEAR} == *"TTJets_DiLept_2018"* ]]; then NJOBS=5; fi           #   968.1
        fi

        if [[ ${STUDY} == "createMini" ]]; then
            if [[ ${SAMPLE}_${YEAR} == *"DYJetsToLL_M-50_2016"* ]]; then NJOBS=10; fi        #   1886.3
            if [[ ${SAMPLE}_${YEAR} == *"DYJetsToLL_M-50_2016"* ]]; then NJOBS=10; fi        #   1900.1
            if [[ ${SAMPLE}_${YEAR} == *"DYJetsToLL_M-50_2016"* ]]; then NJOBS=10; fi         #   946.1
            if [[ ${SAMPLE}_${YEAR} == *"DoubleEG_Run2016_2016"* ]]; then NJOBS=10; fi        #   1031.0
            if [[ ${SAMPLE}_${YEAR} == *"DoubleEG_Run2017_2017"* ]]; then NJOBS=10; fi        #   1201.3
            if [[ ${SAMPLE}_${YEAR} == *"EGamma_Run2018_2018"* ]]; then NJOBS=10; fi          #   1831.5
            if [[ ${SAMPLE}_${YEAR} == *"DoubleMuon_Run2016_2016"* ]]; then NJOBS=11; fi     #   2208.1
            if [[ ${SAMPLE}_${YEAR} == *"DoubleMuon_Run2017_2017"* ]]; then NJOBS=12; fi     #   2449.8
            if [[ ${SAMPLE}_${YEAR} == *"DoubleMuon_Run2018_2018"* ]]; then NJOBS=35; fi     #   3474.0
            if [[ ${SAMPLE}_${YEAR} == *"SingleElectron_Run2016_2016"* ]]; then NJOBS=10; fi  #   803.8
            if [[ ${SAMPLE}_${YEAR} == *"SingleElectron_Run2017_2017"* ]]; then NJOBS=10; fi  #   971.4
            if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2016_2016"* ]]; then NJOBS=10; fi      #   1751.2
            if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2017_2017"* ]]; then NJOBS=10; fi     #   2026.6
            if [[ ${SAMPLE}_${YEAR} == *"SingleMuon_Run2018_2018"* ]]; then NJOBS=10; fi     #   2765.8
            if [[ ${SAMPLE}_${YEAR} == *"TTJets_DiLept_2016"* ]]; then NJOBS=10; fi           #   247.9
            if [[ ${SAMPLE}_${YEAR} == *"TTJets_DiLept_2017"* ]]; then NJOBS=10; fi           #   930.3
            if [[ ${SAMPLE}_${YEAR} == *"TTJets_DiLept_2018"* ]]; then NJOBS=10; fi           #   968.1
            if [[ ${SAMPLE}_${YEAR} == *"TTTo2L2Nu_2016"* ]]; then NJOBS=30; fi           #   247.9
            if [[ ${SAMPLE}_${YEAR} == *"TTTo2L2Nu_2017"* ]]; then NJOBS=30; fi           #   930.3
            if [[ ${SAMPLE}_${YEAR} == *"TTTo2L2Nu_2018"* ]]; then NJOBS=30; fi           #   968.1
        fi

        NJOBSMAXIDX=$((NJOBS - 1))

        FILELIST=$(ls ${NANOSKIMDIR}/${SAMPLEWITHUNDERSCORE}*${NANOTAG}*${EXTRATAG}*/merged/output.root | tr '\n' ',')
        FILENAME=output
        for IJOB in $(seq 0 ${NJOBSMAXIDX}); do
            if [ -z "${DEBUG}" ]; then
                if [[ ${SAMPLE} == *"Run201"* ]]; then
                    echo "rm -f ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root; ${EXECUTABLE} -e  0 -t Events -o ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root --scale1fb ${SCALE1FB} -j ${NJOBS} -I ${IJOB} -i ${FILELIST} > ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.log 2>&1" >> .jobs.txt
                else
                    if [[ ${BABYVERSION} == *"_jecUp"* ]]; then
                        echo "rm -f ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root; ${EXECUTABLE} -e  1 -t Events -o ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root --scale1fb ${SCALE1FB} -j ${NJOBS} -I ${IJOB} -i ${FILELIST} > ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.log 2>&1" >> .jobs.txt
                    elif [[ ${BABYVERSION} == *"_jecDn"* ]]; then
                        echo "rm -f ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root; ${EXECUTABLE} -e -1 -t Events -o ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root --scale1fb ${SCALE1FB} -j ${NJOBS} -I ${IJOB} -i ${FILELIST} > ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.log 2>&1" >> .jobs.txt
                    else
                        echo "rm -f ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root; ${EXECUTABLE} -e  0 -t Events -o ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root --scale1fb ${SCALE1FB} -j ${NJOBS} -I ${IJOB} -i ${FILELIST} > ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.log 2>&1" >> .jobs.txt
                    fi
                fi
            else
                echo "rm -f ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root; ${EXECUTABLE} -n 50000 -t Events -o ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.root --scale1fb ${SCALE1FB} -j ${NJOBS} -I ${IJOB} -i ${FILELIST} > ${HISTDIR}/${SAMPLE}_${FILENAME}_${IJOB}.log 2>&1" >> .jobs.txt
            fi
        done


    done

done

xargs.sh .jobs.txt
