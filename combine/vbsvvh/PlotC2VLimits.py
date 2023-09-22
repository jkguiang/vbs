# Code written by Lara Zygala

import numpy as np
import ROOT

import ch_plotting

ROOT.gROOT.SetBatch(ROOT.kTRUE)

# Style and pads
ch_plotting.ModTDRStyle()
canv = ROOT.TCanvas('limit', 'limit')
pads = ch_plotting.OnePad()
 
# Get limit TGraphs as a dictionary
draw=[ 'exp0', 'exp1', 'exp2', 'theo']
graphs = ch_plotting.StandardLimitsFromJSONFile('limits_C2V.json', draw)

# Create an empty TH1 from the first TGraph to serve as the pad axis and frame
axis = ch_plotting.CreateAxisHist(list(graphs.values())[0])
axis.GetXaxis().SetTitle('#Kappa_{2V} Value')
axis.GetYaxis().SetTitle('95% CL limit on #sigma(VVH)/#sigma(SM)')
pads[0].cd()
axis.Draw('axis')
 
# Create a legend in the top left
legend = ch_plotting.PositionedLegend(0.3, 0.2, 3, 0.015)
 
# Set the standard green and yellow colors and draw
ch_plotting.StyleLimitBand(graphs)
ch_plotting.DrawLimitBand(pads[0], graphs, legend=legend)
legend.Draw()
 
# Adjust the y-axis range such that the maximum graph value sits 25% below
# the top of the frame. Fix the minimum to zero.
ch_plotting.FixBothRanges(pads[0], 0.0001, 0, 5000, 0.25)

# Re-draw the frame and tick marks
pads[0].RedrawAxis()
pads[0].GetFrame().Draw()
pads[0].Modified()
pads[0].Update()

#K2V Limit 
l_mins = lambda x, par : np.fabs(graphs["theo"].Eval(x[0]) - graphs["exp0"].Eval(x[0]))
tf1_mins = ROOT.TF1("f_mins", l_mins ,-2.0, 4.0, 0)

lim_lo = tf1_mins.GetMinimumX(-2.0, 1.0)
lim_hi = tf1_mins.GetMinimumX(1.0, 4.0)
ymax_lim = 10**(0.6*pads[0].GetUymax())

line_lo = ROOT.TLine(lim_lo, 0, lim_lo, ymax_lim)
line_hi = ROOT.TLine(lim_hi, 0, lim_hi, ymax_lim)

line_lo.Draw('same')
line_hi.Draw('same')

pt = ROOT.TPaveText(legend.GetX1NDC(),legend.GetY1NDC()-0.08,legend.GetX2NDC(),legend.GetY1NDC(), "NDC")
pt.AddText("#Kappa_{2V} = "+"({:.2f}, {:.2f})".format(lim_lo, lim_hi))
pt.Draw("same")

canv.SaveAs("C2V_Limit.png")
canv.SaveAs("C2V_Limit.pdf")
