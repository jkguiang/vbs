SR_X="abcdnet_score > 0.89"
SR_Y="abs_deta_jj > 5"
SR_GLOBAL="hbbfatjet_xbb > 0.8 and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7"
TAG=$1
if [[ "$TAG" == "" ]]; then
    echo "No tag provided"
    exit 1
fi
BASEDIR=/data/userdata/$USER/vbs_studies

if [[ "$BASEDIR" != "studies" ]]; then
    rm -rf studies/vbsvvhjets/output_${TAG}*
fi

for BABYDIR in $BASEDIR/vbsvvhjets/output_${TAG}*; do 
    BABYTAG=${BABYDIR##*output_}
    # Add ABCD regions to cutflows
    python scripts/add_abcd_regions.py vbsvvhjets --basedir=$BASEDIR --tag=$BABYTAG --SR_x="$SR_X" --SR_y="$SR_Y" --SR_global="$SR_GLOBAL"
    echo "Copying from $BABYDIR to $PWD/studies/vbsvvhjets"
    cp -R $BABYDIR studies/vbsvvhjets/
done

cp /data/userdata/$USER/bfscan_results/* studies/vbsvvhjets/output_$TAG/
