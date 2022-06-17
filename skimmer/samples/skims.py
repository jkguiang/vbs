from metis.Sample import DirectorySample
import glob

sig_1lep_1ak8_2ak4_v1 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("sig_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/sig_1lep_1ak8_2ak4_v1/*")
]

bkg_1lep_1ak8_2ak4_v1 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("bkg_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/bkg_1lep_1ak8_2ak4_v1/*")
]

data_1lep_1ak8_2ak4_v1 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("data_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/data_1lep_1ak8_2ak4_v1/*")
]
