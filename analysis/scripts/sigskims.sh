# SAMPLES="
# /data/userdata/jguiang/nanoaod/VBSWHSignalGeneration/private/VBSWH_posLambdaWZ_kWkZscan_m2to2_fromBSM_Mjj100toInf_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v3/merged.root
# /data/userdata/jguiang/nanoaod/VBSWHSignalGeneration/private/VBSWH_posLambdaWZ_kWkZscan_m2to2_fromBSM_Mjj100toInf_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v3/merged.root
# /data/userdata/jguiang/nanoaod/VBSWHSignalGeneration/private/VBSWH_posLambdaWZ_kWkZscan_m2to2_fromBSM_Mjj100toInf_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v3/merged.root
# /data/userdata/jguiang/nanoaod/VBSWHSignalGeneration/private/VBSWH_posLambdaWZ_kWkZscan_m2to2_fromBSM_Mjj100toInf_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v3/merged.root
# "
# SKIMDIR=/data/userdata/jguiang/nanoaod/VBSWHSkim
# SKIMTAG=sig_1lep_1ak8_2ak4_pku
# SKIMMER=skim_vbswh_pku

SAMPLES="
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_OS_VBSCuts_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_OS_VBSCuts_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_OS_VBSCuts_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_OS_VBSCuts_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_SS_VBSCuts_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_SS_VBSCuts_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_SS_VBSCuts_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWWH_SS_VBSCuts_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWZH_VBSCuts_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWZH_VBSCuts_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWZH_VBSCuts_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSWZH_VBSCuts_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSZZH_VBSCuts_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSZZH_VBSCuts_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSZZH_VBSCuts_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v2/merged_tthfix.root
/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBSZZH_VBSCuts_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v2/merged_tthfix.root
"
SKIMDIR=/data/userdata/jguiang/nanoaod/VBSVVHSkim
SKIMTAG=sig_0lep_2ak4_2ak8_ttH
SKIMMER=skim_vbsvvhjets

if [[ "$SAMPLES" != "" && "$SKIMDIR" != "" && "$SKIMTAG" != "" ]]; then
    for sample in $SAMPLES; do
        echo "Skimming $sample"
        ./bin/$SKIMMER -t Events -d . -n output -T Events $sample
        sampledir=$(dirname $sample)
        targetdir=$SKIMDIR/$SKIMTAG/${sampledir##*/}_$SKIMTAG
        mkdir -p $targetdir
        echo "Wrote skim to $targetdir/merged.root"
        mv output.root $targetdir/merged.root
    done
fi
