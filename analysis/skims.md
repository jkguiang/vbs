# Skims
VBS studies can be used to run preliminary selections on CMS data, i.e. producing NanoAOD to NanoAOD after some selections. 
In fact, a number of branches can also be dropped as they are not used in the analysis. 
The output of these jobs are called skims, and they are tagged to help keep track of what is inside of each one. 
These tags are described in greater detail (when/where possible) here.

## VBS WH skims
- `*_1lep_1ak8_2ak4_v1`
    - Runs ttH UL MVA to create a custom branch that stores the updated MVA discriminator
    - Implements only a very loose selection on the objects
- `*_1lep_1ak8_2ak4_pku`
    - Uses PKU lepton ID
    - Also includes the selections that were originally applied in the postskim

## VBS VVH (all-hadronic) skims
- `*_0lep_2ak4_2ak8`
    - Uses PKU lepton ID
- `*_0lep_2ak4_2ak8_v2`
- `*_0lep_2ak4_2ak8_ttH`
    - Uses ttH lepton ID
    - Accidentally saved the output earlier in the cutflow: AK4 selections are not applied
- `*_0lep_2ak4_2ak8_ttH_v2`
    - Now apply >= 2 AK4 jet selection
        - No overlap removal with fat jets!
        - Why? I did not want to apply the HEM prescription in the skim, just in case...
    - Very loose VBS selection: at least one pair of AK4 jets with Mjj > 500 GeV and |detajj| > 2.5

