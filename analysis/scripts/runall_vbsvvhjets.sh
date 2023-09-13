TAG=abcdnet_v1
N_WORKERS=48

# Run main analysis code
./bin/run vbsvvhjets --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSVVHSkim --skimtag=0lep_2ak4_2ak8_ttH --data --tag=$TAG
./bin/merge_vbsvvhjets --tag=$TAG vbsvvhjets
# Run JEC/JER variations
./bin/run vbsvvhjets --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSVVHSkim --skimtag=0lep_2ak4_2ak8_ttH --data --tag=${TAG}_jec_up --var=jec_up --no_make
./bin/run vbsvvhjets --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSVVHSkim --skimtag=0lep_2ak4_2ak8_ttH --data --tag=${TAG}_jec_dn --var=jec_dn --no_make
./bin/run vbsvvhjets --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSVVHSkim --skimtag=0lep_2ak4_2ak8_ttH --data --tag=${TAG}_jer_up --var=jer_up --no_make
./bin/run vbsvvhjets --n_workers=$N_WORKERS --skimdir=/data/userdata/jguiang/nanoaod/VBSVVHSkim --skimtag=0lep_2ak4_2ak8_ttH --data --tag=${TAG}_jer_dn --var=jer_dn --no_make
./bin/merge_vbsvvhjets vbsvvhjets --tag=${TAG}_jec_up
./bin/merge_vbsvvhjets vbsvvhjets --tag=${TAG}_jec_dn
./bin/merge_vbsvvhjets vbsvvhjets --tag=${TAG}_jer_up
./bin/merge_vbsvvhjets vbsvvhjets --tag=${TAG}_jer_dn
