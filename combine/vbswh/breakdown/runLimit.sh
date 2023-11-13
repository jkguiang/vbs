NAME=vbswh_unblinded_grouped
# Make workspace
rm -f *.root
rm -f breakdown.*
text2workspace.py ../datacards/${NAME}.dat -o ${NAME}.root

combine ${NAME}.root -M MultiDimFit -m 125 --saveWorkspace --name _${NAME}_postfit

combine higgsCombine_${NAME}_postfit.MultiDimFit.mH125.root -M MultiDimFit -m 125 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --snapshotName MultiDimFit --name _${NAME}_total

SYST_GROUPS="
signal_theory
abcd_syst
abcd_stat
signal_experimental
"

FREEZE=""
FREEZE_NAME="_${NAME}_freeze"
for group in $SYST_GROUPS; do
    if [[ "$FREEZE" == "" ]]; then
        FREEZE="$group"
    else
        FREEZE="$FREEZE,$group"
    fi
    FREEZE_NAME="${FREEZE_NAME}_$group"
    combine higgsCombine_${NAME}_postfit.MultiDimFit.mH125.root -M MultiDimFit -m 125 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --snapshotName MultiDimFit --freezeNuisanceGroups $FREEZE --name $FREEZE_NAME
done
combine higgsCombine_${NAME}_postfit.MultiDimFit.mH125.root -M MultiDimFit -m 125 --setParameters r=0 --setParameterRanges r=0.0,2.0 --algo grid --points 101 --alignEdges 1 --snapshotName MultiDimFit --freezeParameters allConstrainedNuisances --name _${NAME}_freeze_all

# --others format is as follows: file.root:"label":color
#                                                  ^^^^^
#                                                  This is an integer (ROOT colors I think)
plot1DScan.py higgsCombine_${NAME}_total.MultiDimFit.mH125.root --main-label "Total Uncert."  \
    --others higgsCombine_${NAME}_freeze_signal_theory.MultiDimFit.mH125.root:"freeze signal_theory":4 \
             higgsCombine_${NAME}_freeze_signal_theory_abcd_syst.MultiDimFit.mH125.root:"freeze signal_theory+abcd_stat":7 \
             higgsCombine_${NAME}_freeze_signal_theory_abcd_syst_abcd_stat.MultiDimFit.mH125.root:"freeze signal_theory+abcd_stat+abcd_syst":5 \
             higgsCombine_${NAME}_freeze_signal_theory_abcd_syst_abcd_stat_signal_experimental.MultiDimFit.mH125.root:"freeze signal_theory+abcd_stat+abcd_syst+signal_experimental":2 \
             higgsCombine_${NAME}_freeze_all.MultiDimFit.mH125.root:"stat only":6 \
    --output breakdown \
    --y-max 10 \
    --y-cut 40 \
    --breakdown "signal_theory,abcd_syst,abcd_stat,signal_experimental,signal_stat,data_stat"
