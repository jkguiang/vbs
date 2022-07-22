#!/bin/env python
from metis.Sample import DBSSample

# DIS searches:
# SingleMuon 2016: /SingleMuon/Run2016*UL*MiniAODv2_NanoAODv9*/NANOAOD
# SingleMuon 2017: /SingleMuon/Run2017*UL*MiniAODv2_NanoAODv9*/NANOAOD
# SingleMuon 2018: /SingleMuon/Run2018*UL*MiniAODv2_NanoAODv9-v*/NANOAOD
# SingleElectron 2016: /SingleElectron/Run2016*UL*MiniAODv2_NanoAODv9*/NANOAOD
# SingleElectron 2017: /SingleElectron/Run2017*UL*MiniAODv2_NanoAODv9*/NANOAOD
# SingleElectron 2018: /EGamma/Run2018*UL*MiniAODv2_NanoAODv9*/NANOAOD

nanoaodv9 = [
    # 2016 pre-VFP samples
    DBSSample(dataset="/SingleMuon/Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016B-ver1_HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016B-ver2_HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016C-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016D-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016E-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016F-HIPM_UL2016_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    # 2016 post-VFP samples
    DBSSample(dataset="/SingleMuon/Run2016F-UL2016_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016G-UL2016_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2016H-UL2016_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016F-UL2016_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016G-UL2016_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2016H-UL2016_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    # 2017 samples
    DBSSample(dataset="/SingleMuon/Run2017B-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2017C-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2017D-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2017E-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2017F-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2017G-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2017H-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2017B-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2017C-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2017D-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2017E-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/SingleElectron/Run2017F-UL2017_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    # 2018 samples
    DBSSample(dataset="/SingleMuon/Run2018A-UL2018_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2018B-UL2018_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2018C-UL2018_MiniAODv2_NanoAODv9-v2/NANOAOD"),
    DBSSample(dataset="/SingleMuon/Run2018D-UL2018_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/EGamma/Run2018A-UL2018_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/EGamma/Run2018B-UL2018_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/EGamma/Run2018C-UL2018_MiniAODv2_NanoAODv9-v1/NANOAOD"),
    DBSSample(dataset="/EGamma/Run2018D-UL2018_MiniAODv2_NanoAODv9-v3/NANOAOD"),
]
