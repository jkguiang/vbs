# -*- coding: utf-8 -*-

from __future__ import absolute_import

import CombineHarvester.CombineTools.plotting as plot
import ROOT

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.gROOT.SetBatch(ROOT.kTRUE)

plot.ModTDRStyle()

canvas = ROOT.TCanvas()

fin = ROOT.TFile("fitDiagnostics_vbswh_unblinded.root")

table = {}

for first_dir in ["shapes_prefit", "shapes_fit_b", "shapes_fit_s"]:

    h_bkg = fin.Get(first_dir + "/bin1/total_background")
    h_sig = fin.Get(first_dir + "/bin1/total_signal")
    h_dat = fin.Get(first_dir + "/bin1/data")  # This is a TGraphAsymmErrors, not a TH1F

    table[first_dir] = {}
    table[first_dir]["bkg_count"] = h_bkg.GetBinContent(1)
    table[first_dir]["sig_count"] = h_sig.GetBinContent(1)
    table[first_dir]["data_count"] = h_dat.GetY()[0]
    table[first_dir]["bkg_error"] = h_bkg.GetBinError(1)
    table[first_dir]["sig_error"] = h_sig.GetBinError(1)
    # table[first_dir]["data_error"] = h_dat.GetErrorY(1)


    h_bkg.SetFillColor(ROOT.TColor.GetColor(100, 192, 232))
    h_bkg.Draw("HIST")

    h_err = h_bkg.Clone()
    h_err.SetFillColorAlpha(12, 0.3)  # Set grey colour (12) and alpha (0.3)
    h_err.SetMarkerSize(0)
    h_err.Draw("E2SAME")

    h_sig.SetLineColor(ROOT.kRed)
    h_sig.Draw("HISTSAME")

    h_dat.Draw("PSAME")

    h_bkg.SetMaximum(h_bkg.GetMaximum() * 1.75)
    h_bkg.SetMinimum(0)

    legend = ROOT.TLegend(0.60, 0.70, 0.90, 0.91, "", "NBNDC")
    legend.AddEntry(h_bkg, "Background", "F")
    legend.AddEntry(h_sig, "Signal", "L")
    legend.AddEntry(h_err, "Background uncertainty", "F")
    legend.Draw()

    canvas.SaveAs("/home/users/jguiang/public_html/vbswh_plots/limits/{}_plot.pdf".format(first_dir))
    canvas.SaveAs("/home/users/jguiang/public_html/vbswh_plots/limits/{}_plot.png".format(first_dir))

for title, content in table.items():
    print(title)
    print("Background: {0:>5.1f} ± {1:<3.2f}".format(content["bkg_count"], content["bkg_error"]))
    print("Signal:     {0:>5.1f} ± {1:<3.2f}".format(content["sig_count"], content["sig_error"]))
    print("Data:       {0:>5.1f}".format(content["data_count"]))
