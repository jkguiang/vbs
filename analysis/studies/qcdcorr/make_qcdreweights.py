import glob
import os
import sys
import ROOT
from tqdm import tqdm

def get_hists(root_files, skip=None):
    skip = skip or []
    hists = {}
    for root_file in root_files:

        fh = ROOT.TFile.Open(root_file)

        for key in fh.GetListOfKeys():
            obj = key.ReadObj()
            name = obj.GetName()
            if name in skip:
                continue
            if name in hists:
                hists[name].Add(obj)
            else:
                obj.SetDirectory(0)
                hists[name] = obj

        fh.Close()

    return hists

output_dir = f"studies/qcdcorr/output_{sys.argv[1]}/Run2"
babies = glob.glob(f"{output_dir}/*.root")
if not babies:
    print(f"No ROOT files in {output_dir}")
    exit()
sig_babies = [b for b in babies if "VBSVVH" in b]
qcd_babies = [b for b in babies if "QCD" in b]
data_babies = [b for b in babies if "data" in b]
other_babies = [b for b in babies if not b in sig_babies and not b in data_babies and not b in qcd_babies]

qcd_hists = get_hists(qcd_babies, skip=["tree"])
other_hists = get_hists(other_babies, skip=["tree"])
data_hists = get_hists(data_babies, skip=["tree"])

output_dir = f"{os.getenv('VBSPWD')}/data/vbsvvhjets_sfs"
os.makedirs(output_dir, exist_ok=True)

# Make naive QCD sfs
print("Making naive ParticleNet sfs")
fh = ROOT.TFile(f"{output_dir}/qcd_pnet_sfs.root", "RECREATE")
for hist_name in data_hists:
    ratio_hist = data_hists[hist_name].Clone("RatioDataMC__"+hist_name)
    ratio_hist.Add(other_hists[hist_name], -1)
    ratio_hist.Divide(qcd_hists[hist_name])
    ratio_hist.Write()
fh.Close()

# Make QCD ParticleNet "PDFs"
print("Making QCD ParticleNet 'PDFs'")
fh = ROOT.TFile(f"{output_dir}/qcd_pnet_pdfs.root", "RECREATE")
hists = {"3D": {}, "2D": {}}
for hist_name in data_hists:
    if "SemiMerged_SelectVBSJets" in hist_name:
        pnet = None
        if "xbbscore" in hist_name:
            outname = "ParticleNet_Xbb_PDF"
            pnet = "xbb"
        elif "xvqqscore" in hist_name:
            outname = "ParticleNet_XVqq_PDF"
            pnet = "xvqq"
        elif "xwqqscore" in hist_name:
            outname = "ParticleNet_XWqq_PDF"
            pnet = "xwqq"

        ndim = None
        if "3D" in hist_name:
            outname += "_3D"
            ndim = "3D"
        elif "2D" in hist_name:
            outname += "_2D"
            ndim = "2D"

        if "3Dalt" in hist_name:
            outname += "alt"
            pnet += "alt"

        if pnet in hists[ndim]:
            data_hist = data_hists[hist_name].Clone("TEMP__"+hist_name)
            data_hist.Add(other_hists[hist_name], -1)
            hists[ndim][pnet].Add(data_hist)
        else:
            data_hist = data_hists[hist_name].Clone(outname)
            data_hist.SetTitle("")
            data_hist.Add(other_hists[hist_name], -1)
            hists[ndim][pnet] = data_hist

for ndim, hists_ndim in hists.items():
    for h in hists_ndim.values():
        """
        Bins: Underflow Bin, Bin 1, Bin 2, ..., Bin N, Overflow Bin
              |  (Bin 0)     |______________________|   (Bin N+1) |
              |                       N bins                      |
              |___________________________________________________|
                                  N bins + 2
        """
        for xbin in range(0, h.GetNbinsX()+2):
            for ybin in range(0, h.GetNbinsY()+2):
                for zbin in range(0, h.GetNbinsZ()+2):
                    if h.GetBinContent(xbin, ybin, zbin) < 0:
                        h.SetBinContent(xbin, ybin, zbin, 0)
        h.Write()

fh.Close()
