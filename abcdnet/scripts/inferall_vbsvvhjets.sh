TAG=$1
CONFIG=configs/ABCDNet_30DisCo_mediumPresel_newSignal_veryLongRun_qcdNorm_allFeatNorm_leakyReLU_dCorr.json
MODEL=/ceph/cms/store/user/$USER/ABCDNet/ABCDNet_30DisCo_mediumPresel_newSignal_veryLongRun_qcdNorm_allFeatNorm_leakyReLU_dCorr_modelLeakyNeuralNetwork_nhidden3_hiddensize64_lrConstantLR0.001_discolambda30_epoch900_model.pt

if [[ "$TAG" == "" ]]; then
    echo "No tag provided"
    exit 1
fi

python scripts/infer_ucsd.py $CONFIG $MODEL --baby_glob="/data/userdata/$USER/vbs_studies/vbsvvhjets/output_${TAG}*/Run2"
