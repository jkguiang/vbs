# Code written by Lara Zygala

import numpy as np
import json
import ROOT

v_C2V = ["-2.0", "-1.75", "-1.5", "-1.25", "-1.0", "-0.75", "-0.5", "-0.25", "0.0", "0.1", "0.2", "0.3", "0.4", "0.5", "0.6", "0.7", "0.8", "0.9", "1.0", "1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7", "1.8", "1.9", "2.0", "2.25", "2.5", "2.75", "3.0", "3.25", "3.5", "3.75"]#,"4.0"]

eos_path = "results"
f_xs = "xsecs_C2V.json"

with open(f_xs, 'r') as f:
    xs_c2v = json.load(f)

d_limit = {}
for c in v_C2V:
    reweight_name = "scan_CV_1p0_C2V_{}_C3_1p0".format(c.replace("-", "m").replace(".", "p"))

    d_limit[c] = {}
    
    # fn_al = "{0}/higgsCombine.{1}.AsymptoticLimits.mH125.root".format(eos_path, reweight_name)
    fn_al = "{0}/{1}_result.root".format(eos_path, reweight_name)
    
    fo_al = ROOT.TFile(fn_al, "READ")
    fTree_al = fo_al.Get("limit")

    scale = (xs_c2v["WWH_OS"][c] + xs_c2v["WWH_SS"][c] + xs_c2v["WZH"][c] + xs_c2v["ZZH"][c]) / (xs_c2v["WWH_OS"]["1.0"] + xs_c2v["WWH_SS"]["1.0"] + xs_c2v["WZH"]["1.0"] + xs_c2v["ZZH"]["1.0"])
    #scale = (xs_c2v["WWH_OS"][c] + xs_c2v["WWH_SS"][c] + xs_c2v["WZH"][c]) / (xs_c2v["WWH_OS"]["1.0"] + xs_c2v["WWH_SS"]["1.0"] + xs_c2v["WZH"]["1.0"])
    #scale = xs_c2v["WWH_OS"][c] / xs_c2v["WWH_OS"]["1.0"]
    #scale = xs_c2v["WWH_OS"][c]
    
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
