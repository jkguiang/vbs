from metis.Sample import DirectorySample
import glob

sig_1lep_1ak8_2ak4_v1 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_sig_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/sig_1lep_1ak8_2ak4_v1/*")
]

bkg_1lep_1ak8_2ak4_v1 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_bkg_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/bkg_1lep_1ak8_2ak4_v1/*")
]

data_1lep_1ak8_2ak4_v1 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_data_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/data_1lep_1ak8_2ak4_v1/*")
]


sig_0lep_2ak4_2ak8_ttH = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_sig_0lep_2ak4_2ak8_ttH")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVVHSkim/sig_0lep_2ak4_2ak8_ttH/*")
]

bkg_0lep_2ak4_2ak8_ttH = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_bkg_0lep_2ak4_2ak8_ttH")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVVHSkim/bkg_0lep_2ak4_2ak8_ttH/*")
]

data_0lep_2ak4_2ak8_ttH = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_data_0lep_2ak4_2ak8_ttH")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVVHSkim/data_0lep_2ak4_2ak8_ttH/*")
]
