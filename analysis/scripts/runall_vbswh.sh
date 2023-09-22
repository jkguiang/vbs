TAG=btagsf_fix
N_WORKERS=32

BASEDIR=/data/userdata/jguiang/vbs_studies
SKIMDIR=/data/userdata/jguiang/nanoaod/VBSWHSkim
SKIMTAG=1lep_1ak8_2ak4_pku

# Compute b-tagging efficiencies for scale factors (just write these to /home, i.e. default basedir)
rm -rf studies/btageff/output_vbswh
./bin/run btageff --n_workers=$N_WORKERS --skimdir=$SKIMDIR --skimtag=$SKIMTAG --tag=vbswh
# Run main analysis code
./bin/run vbswh --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=$TAG
./bin/merge_vbswh vbswh --basedir=$BASEDIR --tag=$TAG
# Run JEC/JER variations
./bin/run vbswh --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jec_up --var=jec_up --no_make
./bin/run vbswh --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jec_dn --var=jec_dn --no_make
./bin/run vbswh --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jer_up --var=jer_up --no_make
./bin/run vbswh --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jer_dn --var=jer_dn --no_make
./bin/merge_vbswh vbswh --basedir=$BASEDIR --tag=${TAG}_jec_up
./bin/merge_vbswh vbswh --basedir=$BASEDIR --tag=${TAG}_jec_dn
./bin/merge_vbswh vbswh --basedir=$BASEDIR --tag=${TAG}_jer_up
./bin/merge_vbswh vbswh --basedir=$BASEDIR --tag=${TAG}_jer_dn

# Keep a copy on /home
cp -R $BASEDIR/vbswh/output_${TAG}* studies/vbswh/
