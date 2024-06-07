import sys
import numpy as np
import json
import ROOT

v_C2V = ["m2p0","m1p75","m1p5","m1p25","m1p0","m0p75","m0p5","m0p25","0p0","0p1","0p2","0p3","0p4","0p5","0p6","0p7","0p8","0p9","1p0","1p1","1p2","1p3","1p3","1p4","1p5","1p6","1p7","1p8","1p9","2p0","2p25","2p5","2p75","3p0","3p25","3p5","3p75","4p0"]

eos_path = sys.argv[1]
f_xs = "xsecs_C2V.json"

with open(f_xs, 'r') as f:
    xs_c2v = json.load(f)

d_limit = {}

for c in v_C2V:
    reweight_name = "scan_CV_1p0_C2V_{}_C3_1p0".format(c)

    d_limit[c] = {}
    
    outname_key = ".C2V_{}".format(c)
    # fn_al = eos_path+"higgsCombine"+outname_key+".AsymptoticLimits.mH120.root"
    fn_al = "{0}/{1}_result.root".format(eos_path, reweight_name)
    # fn_al = eos_path+"higgsCombine"+outname_key+".HybridNew.mH120.root"
    fo_al = ROOT.TFile(fn_al, "READ")
    fTree_al = fo_al.Get("limit")
    # scale = (xs_c2v["WWH_OS"][c] + xs_c2v["WWH_SS"][c] + xs_c2v["WZH"][c]) / (xs_c2v["WWH_OS"]["1p0"] + xs_c2v["WWH_SS"]["1p0"] + xs_c2v["WZH"]["1p0"])

    c_point = c.replace("p", ".").replace("m", "-")
    scale = xs_c2v["WWH_OS"][c] + xs_c2v["WWH_SS"][c] + xs_c2v["WZH"][c] + xs_c2v["ZZH"][c]
    # scale = (xs_c2v["WWH_OS"][c] + xs_c2v["WWH_SS"][c] + xs_c2v["WZH"][c])
    # scale = xs_c2v["WWH_OS"][c] / xs_c2v["WWH_OS"]["1.0"]
    # scale = xs_c2v["WWH_OS"][c]
    
    for evt in fTree_al:
        
        if evt.quantileExpected == -1:
            d_limit[c]['obs'] = evt.limit * scale
        elif abs(evt.quantileExpected - 0.5) < 1E-4:
            d_limit[c]["exp0"] = evt.limit * scale
        elif abs(evt.quantileExpected - 0.025) < 1E-4:
            d_limit[c]["exp-2"] = evt.limit * scale
        elif abs(evt.quantileExpected - 0.160) < 1E-4:
            d_limit[c]["exp-1"] = evt.limit * scale
        elif abs(evt.quantileExpected - 0.840) < 1E-4:
            d_limit[c]["exp+1"] = evt.limit * scale
        elif abs(evt.quantileExpected - 0.975) < 1E-4:
            d_limit[c]["exp+2"] = evt.limit * scale
            
        
    d_limit[c]['theo'] = scale
        
    fo_al.Close()

with open("limits_C2V.json", "w") as outfile:
    json.dump(d_limit, outfile)
