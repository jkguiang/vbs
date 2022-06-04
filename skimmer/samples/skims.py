from metis.Sample import DirectorySample
import glob

nanoaodv9_1lep_1ak8_2ak4 = [
    DirectorySample(location=d, dataset=d.split("/")[-1].split("_1lep_1ak8")[0], use_xrootd=True) for d in glob.glob("/ceph/cms/store/user/jguiang/VBSVHSkim/1lep_1ak8_2ak4_v1/*")        
]
