#!/bin/env python
from metis.Sample import DBSSample, DirectorySample

nanoaodv9_bkg = [
    # TTToHadronic: /TTToHadronic_TuneCP5_13TeV*/*NanoAOD*v9-106X*/NANOAODSIM
    DBSSample(dataset="/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/TTToHadronic_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),

    # QCD (HT-binned): /QCD_HT*_TuneCP5_PSWeights_13TeV-madgraph-pythia8/*NanoAOD*v9-106X*/NANOAODSIM
    # DBSSample(dataset="/QCD_HT50to100_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"), # low stats
    DBSSample(dataset="/QCD_HT100to200_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v1/NANOAODSIM"),

    # DBSSample(dataset="/QCD_HT50to100_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"), # low stats
    DBSSample(dataset="/QCD_HT100to200_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v1/NANOAODSIM"),

    # DBSSample(dataset="/QCD_HT50to100_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"), # low stats
    DBSSample(dataset="/QCD_HT100to200_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v1/NANOAODSIM"),

    # DBSSample(dataset="/QCD_HT50to100_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"), # low stats
    DBSSample(dataset="/QCD_HT100to200_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT200to300_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT300to500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT500to700_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT700to1000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v2/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1000to1500_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT1500to2000_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),
    DBSSample(dataset="/QCD_HT2000toInf_TuneCP5_PSWeights_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1/NANOAODSIM"),

]

nanoaodv9_sig = [
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        dataset="VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        dataset="VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        dataset="VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        dataset="VBSOSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        dataset="VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        dataset="VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        dataset="VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        dataset="VBSWWH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        dataset="VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        dataset="VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        dataset="VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        dataset="VBSWZH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        dataset="VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL16-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        dataset="VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL16APV-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        dataset="VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL17-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    ),
    DirectorySample(
        location="/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v1/VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        dataset="VBSZZH_Inclusive_4f_TuneCP5_RunIISummer20UL18-106X_privateMC_NANOGEN_v1",
        globber="merged*.root",
        use_xrootd=True
    )
]
