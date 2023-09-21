import glob

import ROOT as r
r.gROOT.SetBatch(True)
r.gStyle.SetOptStat(0)

def plot(file_name, hist_name):
    f = r.TFile(file_name)
    eff = f.Get(hist_name)

    c1 = r.TCanvas("", "", 1200, 900)
    c1.SetRightMargin(0.14)
    c1.SetTopMargin(0.14)
    c1.SetBottomMargin(0.14)
    c1.SetLeftMargin(0.14)
    c1.SetFillStyle(4000);

    r.gPad.SetTickx(1)
    r.gPad.SetTicky(1)
    r.gStyle.SetPaintTextFormat(".2f");

    eff.SetTitle("")
    # eff.SetMarkerStyle(20)
    # eff.SetMarkerColor(1)
    # eff.SetLineColor(1)

    eff.GetXaxis().SetTitleSize(0.045)
    eff.GetYaxis().SetTitleSize(0.050)
    eff.GetXaxis().SetTitleOffset(1.4)
    eff.GetYaxis().SetTitleOffset(1.4)
    eff.GetXaxis().SetLabelSize(0.045)
    eff.GetYaxis().SetLabelSize(0.045)
    eff.GetXaxis().SetTitle("p_{T} [GeV]")
    eff.GetYaxis().SetTitle("|#eta|")

    eff.Draw("colztexte")

    t = r.TLatex()
    t.SetTextAlign(11)
    t.SetTextFont(42)
    t.SetTextColor(r.kBlack)
    # t.SetTextSize(0.045)
    t.SetTextSize(0.035)
    ts = t.GetTextSize()
    cms_label = "Simulation Preliminary"
    t.DrawLatexNDC(0.14, 0.875, "#scale[1.375]{#font[61]{CMS}} #scale[1.21]{#font[52]{%s}}" % cms_label)

    name = baby.split("/")[-1].replace(".root", "")
    year = baby.split("/")[-2]

    t = r.TLatex()
    t.SetTextAlign(31)
    t.SetTextFont(42)
    t.SetTextColor(r.kBlack)
    # t.SetTextSize(0.045)
    t.SetTextSize(0.035)
    ts = t.GetTextSize()
    if year == "2018":
        year_label = "59.83 fb^{-1}"
    elif year == "2017":
        year_label = "41.48 fb^{-1}"
    elif year == "2016preVFP":
        year_label = "19.52 fb^{-1}"
    elif year == "2016postVFP":
        year_label = "16.81 fb^{-1}"

    t.DrawLatexNDC(0.865, 0.875, "#scale[1.21]{#font[42]{%s (13 TeV)}}" % year_label)

    c1.SaveAs(f"/home/users/jguiang/public_html/vbswh_plots/btageff/{name}_{hist_name}_{year}.png")
    c1.SaveAs(f"/home/users/jguiang/public_html/vbswh_plots/btageff/{name}_{hist_name}_{year}.pdf")

if __name__ == "__main__":
    for baby in glob.glob("studies/btageff/output_vbswh/*/*.root"):
        if "/TTToSemiLeptonic" in baby:
            plot(baby, "deepjet_eff_b_medium")
            plot(baby, "deepjet_eff_c_medium")
            plot(baby, "deepjet_eff_light_medium")
            plot(baby, "n_b_medium")
            plot(baby, "n_b_total")
            plot(baby, "n_c_medium")
            plot(baby, "n_c_total")
            plot(baby, "n_light_medium")
            plot(baby, "n_light_total")
