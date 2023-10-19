SR_X="abcdnet_score > 0.89"
SR_Y="abs_deta_jj > 5"
SR_GLOBAL="hbbfatjet_xbb > 0.8 and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7"
# TAG=abcdnet_v1
TAG=sparse_hlt
BASEDIR=/data/userdata/jguiang/vbs_studies

python scripts/add_abcd_regions.py vbsvvhjets --basedir=$BASEDIR --tag=$TAG          --SR_x="$SR_X" --SR_y="$SR_Y" --SR_global="$SR_GLOBAL"
# python scripts/add_abcd_regions.py vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jec_up --SR_x="$SR_X" --SR_y="$SR_Y" --SR_global="$SR_GLOBAL"
# python scripts/add_abcd_regions.py vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jec_dn --SR_x="$SR_X" --SR_y="$SR_Y" --SR_global="$SR_GLOBAL"
# python scripts/add_abcd_regions.py vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jer_up --SR_x="$SR_X" --SR_y="$SR_Y" --SR_global="$SR_GLOBAL"
# python scripts/add_abcd_regions.py vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jer_dn --SR_x="$SR_X" --SR_y="$SR_Y" --SR_global="$SR_GLOBAL"

# Keep a copy on /home
if [[ "$BASEDIR" != "studies" ]]; then
    rm -rf studies/vbsvvhjets/output_${TAG}*
fi
cp -R $BASEDIR/vbsvvhjets/output_${TAG}* studies/vbsvvhjets/
