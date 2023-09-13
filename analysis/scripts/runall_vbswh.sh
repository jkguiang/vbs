TAG=kscans
N_WORKERS=32

# Compute b-tagging efficiencies for scale factors
rm -rf studies/btageff/output_vbswh
./bin/run btageff --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSWHSkim --skimtag=1lep_1ak8_2ak4_pku --tag=vbswh
# Run main analysis code
./bin/run vbswh --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSWHSkim --skimtag=1lep_1ak8_2ak4_pku --data --tag=$TAG
./bin/merge_vbswh --tag=$TAG vbswh
# Run JEC/JER variations
./bin/run vbswh --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSWHSkim --skimtag=1lep_1ak8_2ak4_pku --data --tag=${TAG}_jec_up --var=jec_up --no_make
./bin/run vbswh --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSWHSkim --skimtag=1lep_1ak8_2ak4_pku --data --tag=${TAG}_jec_dn --var=jec_dn --no_make
./bin/run vbswh --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSWHSkim --skimtag=1lep_1ak8_2ak4_pku --data --tag=${TAG}_jer_up --var=jer_up --no_make
./bin/run vbswh --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSWHSkim --skimtag=1lep_1ak8_2ak4_pku --data --tag=${TAG}_jer_dn --var=jer_dn --no_make
./bin/merge_vbswh vbswh --tag=${TAG}_jec_up
./bin/merge_vbswh vbswh --tag=${TAG}_jec_dn
./bin/merge_vbswh vbswh --tag=${TAG}_jer_up
./bin/merge_vbswh vbswh --tag=${TAG}_jer_dn
