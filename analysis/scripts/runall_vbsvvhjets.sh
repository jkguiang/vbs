TAG=abcdnet_v1
N_WORKERS=32

BASEDIR=/data/userdata/jguiang/vbs_studies
SKIMDIR=/data/userdata/jguiang/nanoaod/VBSVVHSkim
SKIMTAG=0lep_2ak4_2ak8_ttH

mkdir -p $BASEDIR

# Run main analysis code
./bin/run vbsvvhjets --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=$TAG
./bin/merge_vbsvvhjets vbsvvhjets --basedir=$BASEDIR --tag=$TAG
# Run JEC/JER variations
./bin/run vbsvvhjets --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jec_up --var=jec_up --no_make
./bin/run vbsvvhjets --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jec_dn --var=jec_dn --no_make
./bin/run vbsvvhjets --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jer_up --var=jer_up --no_make
./bin/run vbsvvhjets --n_workers=$N_WORKERS --basedir=$BASEDIR --skimdir=$SKIMDIR --skimtag=$SKIMTAG --data --tag=${TAG}_jer_dn --var=jer_dn --no_make
./bin/merge_vbsvvhjets vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jec_up
./bin/merge_vbsvvhjets vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jec_dn
./bin/merge_vbsvvhjets vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jer_up
./bin/merge_vbsvvhjets vbsvvhjets --basedir=$BASEDIR --tag=${TAG}_jer_dn

# Keep a copy on /home -- NOT NEEDED, done later (c.f. addall_vbsvvhjets.sh)
# cp -R $BASEDIR/vbsvvhjets/output_${TAG}* studies/vbsvvhjets/
