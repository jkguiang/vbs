TAG=$1
CONFIG=configs/ABCDNet_30DisCo_mediumPresel_newSignal_veryLongRun_qcdNorm_allFeatNorm_leakyReLU_dCorr.json
MODEL=/ceph/cms/store/user/jguiang/ABCDNet/ABCDNet_30DisCo_mediumPresel_newSignal_veryLongRun_qcdNorm_allFeatNorm_leakyReLU_dCorr_modelLeakyNeuralNetwork_nhidden3_hiddensize64_lrConstantLR0.001_discolambda30_epoch900_model.pt

for BABYDIR in /data/userdata/jguiang/vbs_studies/vbsvvhjets/output_${TAG}*; do
    python scripts/infer_ucsd.py $CONFIG $MODEL $BABYDIR/Run2
done
