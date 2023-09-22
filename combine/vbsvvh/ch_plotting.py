from __future__ import absolute_import
from __future__ import print_function
import ROOT as R
import math
from array import array
import re
import json
import types
import six
import ctypes
from six.moves import range

def OnePad():
    pad = R.TPad('pad', 'pad', 0., 0., 1., 1.)
    pad.Draw()
    pad.cd()
    result = [pad]
    return result

def SetTDRStyle():
    """Sets the PubComm recommended style

    Just a copy of <http://ghm.web.cern.ch/ghm/plots/MacroExample/tdrstyle.C>
    @sa ModTDRStyle() to use this style with some additional customisation.
    """
    # For the canvas:
    R.gStyle.SetCanvasBorderMode(0)
    R.gStyle.SetCanvasColor(R.kWhite)
    R.gStyle.SetCanvasDefH(600)  # Height of canvas
    R.gStyle.SetCanvasDefW(600)  # Width of canvas
    R.gStyle.SetCanvasDefX(0)    # POsition on screen
    R.gStyle.SetCanvasDefY(0)

    # For the Pad:
    R.gStyle.SetPadBorderMode(0)
    # R.gStyle.SetPadBorderSize(Width_t size = 1)
    R.gStyle.SetPadColor(R.kWhite)
    R.gStyle.SetPadGridX(False)
    R.gStyle.SetPadGridY(False)
    R.gStyle.SetGridColor(0)
    R.gStyle.SetGridStyle(3)
    R.gStyle.SetGridWidth(1)

    # For the frame:
    R.gStyle.SetFrameBorderMode(0)
    R.gStyle.SetFrameBorderSize(1)
    R.gStyle.SetFrameFillColor(0)
    R.gStyle.SetFrameFillStyle(0)
    R.gStyle.SetFrameLineColor(1)
    R.gStyle.SetFrameLineStyle(1)
    R.gStyle.SetFrameLineWidth(1)

    # For the histo:
    # R.gStyle.SetHistFillColor(1)
    # R.gStyle.SetHistFillStyle(0)
    R.gStyle.SetHistLineColor(1)
    R.gStyle.SetHistLineStyle(0)
    R.gStyle.SetHistLineWidth(1)
    # R.gStyle.SetLegoInnerR(Float_t rad = 0.5)
    # R.gStyle.SetNumberContours(Int_t number = 20)

    R.gStyle.SetEndErrorSize(2)
    # R.gStyle.SetErrorMarker(20)
    # R.gStyle.SetErrorX(0.)

    R.gStyle.SetMarkerStyle(20)

    # For the fit/function:
    R.gStyle.SetOptFit(1)
    R.gStyle.SetFitFormat('5.4g')
    R.gStyle.SetFuncColor(2)
    R.gStyle.SetFuncStyle(1)
    R.gStyle.SetFuncWidth(1)

    # For the date:
    R.gStyle.SetOptDate(0)
    # R.gStyle.SetDateX(Float_t x = 0.01)
    # R.gStyle.SetDateY(Float_t y = 0.01)

    # For the statistics box:
    R.gStyle.SetOptFile(0)
    R.gStyle.SetOptStat(0)
    # To display the mean and RMS:   SetOptStat('mr')
    R.gStyle.SetStatColor(R.kWhite)
    R.gStyle.SetStatFont(42)
    R.gStyle.SetStatFontSize(0.025)
    R.gStyle.SetStatTextColor(1)
    R.gStyle.SetStatFormat('6.4g')
    R.gStyle.SetStatBorderSize(1)
    R.gStyle.SetStatH(0.1)
    R.gStyle.SetStatW(0.15)
    # R.gStyle.SetStatStyle(Style_t style = 1001)
    # R.gStyle.SetStatX(Float_t x = 0)
    # R.gStyle.SetStatY(Float_t y = 0)

    # Margins:
    R.gStyle.SetPadTopMargin(0.05)
    R.gStyle.SetPadBottomMargin(0.13)
    R.gStyle.SetPadLeftMargin(0.16)
    R.gStyle.SetPadRightMargin(0.02)

    # For the Global title:
    R.gStyle.SetOptTitle(0)
    R.gStyle.SetTitleFont(42)
    R.gStyle.SetTitleColor(1)
    R.gStyle.SetTitleTextColor(1)
    R.gStyle.SetTitleFillColor(10)
    R.gStyle.SetTitleFontSize(0.05)
    # R.gStyle.SetTitleH(0); # Set the height of the title box
    # R.gStyle.SetTitleW(0); # Set the width of the title box
    # R.gStyle.SetTitleX(0); # Set the position of the title box
    # R.gStyle.SetTitleY(0.985); # Set the position of the title box
    # R.gStyle.SetTitleStyle(Style_t style = 1001)
    # R.gStyle.SetTitleBorderSize(2)

    # For the axis titles:
    R.gStyle.SetTitleColor(1, 'XYZ')
    R.gStyle.SetTitleFont(42, 'XYZ')
    R.gStyle.SetTitleSize(0.06, 'XYZ')
    # Another way to set the size?
    # R.gStyle.SetTitleXSize(Float_t size = 0.02)
    # R.gStyle.SetTitleYSize(Float_t size = 0.02)
    R.gStyle.SetTitleXOffset(0.9)
    R.gStyle.SetTitleYOffset(1.25)
    # R.gStyle.SetTitleOffset(1.1, 'Y'); # Another way to set the Offset

    # For the axis labels:

    R.gStyle.SetLabelColor(1, 'XYZ')
    R.gStyle.SetLabelFont(42, 'XYZ')
    R.gStyle.SetLabelOffset(0.007, 'XYZ')
    R.gStyle.SetLabelSize(0.05, 'XYZ')

    # For the axis:

    R.gStyle.SetAxisColor(1, 'XYZ')
    R.gStyle.SetStripDecimals(True)
    R.gStyle.SetTickLength(0.03, 'XYZ')
    R.gStyle.SetNdivisions(510, 'XYZ')
    R.gStyle.SetPadTickX(1)
    R.gStyle.SetPadTickY(1)

    # Change for log plots:
    R.gStyle.SetOptLogx(0)
    R.gStyle.SetOptLogy(1)
    R.gStyle.SetOptLogz(0)

    # Postscript options:
    R.gStyle.SetPaperSize(20., 20.)
    # R.gStyle.SetLineScalePS(Float_t scale = 3)
    # R.gStyle.SetLineStyleString(Int_t i, const char* text)
    # R.gStyle.SetHeaderPS(const char* header)
    # R.gStyle.SetTitlePS(const char* pstitle)

    # R.gStyle.SetBarOffset(Float_t baroff = 0.5)
    # R.gStyle.SetBarWidth(Float_t barwidth = 0.5)
    # R.gStyle.SetPaintTextFormat(const char* format = 'g')
    # R.gStyle.SetPalette(Int_t ncolors = 0, Int_t* colors = 0)
    # R.gStyle.SetTimeOffset(Double_t toffset)
    # R.gStyle.SetHistMinimumZero(kTRUE)

    R.gStyle.SetHatchesLineWidth(5)
    R.gStyle.SetHatchesSpacing(0.05)


def ModTDRStyle(width=600, height=600, t=0.06, b=0.12, l=0.16, r=0.04):
    """Modified version of the tdrStyle

    Args:
        width (int): Canvas width in pixels
        height (int): Canvas height in pixels
        t (float): Pad top margin [0-1]
        b (float): Pad bottom margin [0-1]
        l (float): Pad left margin [0-1]
        r (float): Pad right margin [0-1]
    """
    SetTDRStyle()

    # Set the default canvas width and height in pixels
    R.gStyle.SetCanvasDefW(width)
    R.gStyle.SetCanvasDefH(height)

    # Set the default margins. These are given as fractions of the pad height
    # for `Top` and `Bottom` and the pad width for `Left` and `Right`. But we
    # want to specify all of these as fractions of the shortest length.
    def_w = float(R.gStyle.GetCanvasDefW())
    def_h = float(R.gStyle.GetCanvasDefH())

    scale_h = (def_w / def_h) if (def_h > def_w) else 1.
    scale_w = (def_h / def_w) if (def_w > def_h) else 1.

    def_min = def_h if (def_h < def_w) else def_w

    R.gStyle.SetPadTopMargin(t * scale_h)
    # default 0.05
    R.gStyle.SetPadBottomMargin(b * scale_h)
    # default 0.13
    R.gStyle.SetPadLeftMargin(l * scale_w)
    # default 0.16
    R.gStyle.SetPadRightMargin(r * scale_w)
    # default 0.02
    # But note the new CMS style sets these:
    # 0.08, 0.12, 0.12, 0.04

    # Set number of axis tick divisions
    R.gStyle.SetNdivisions(506, 'XYZ')  # default 510

    # Some marker properties not set in the default tdr style
    R.gStyle.SetMarkerColor(R.kBlack)
    R.gStyle.SetMarkerSize(1.0)

    R.gStyle.SetLabelOffset(0.007, 'YZ')
    # This is an adhoc adjustment to scale the x-axis label
    # offset when we stretch plot vertically
    # Will also need to increase if first x-axis label has more than one digit
    R.gStyle.SetLabelOffset(0.005 * (3. - 2. / scale_h), 'X')

    # In this next part we do a slightly involved calculation to set the axis
    # title offsets, depending on the values of the TPad dimensions and
    # margins. This is to try and ensure that regardless of how these pad
    # values are set, the axis titles will be located towards the edges of the
    # canvas and not get pushed off the edge - which can often happen if a
    # fixed value is used.
    title_size = 0.05
    title_px = title_size * def_min
    label_size = 0.04
    R.gStyle.SetTitleSize(title_size, 'XYZ')
    R.gStyle.SetLabelSize(label_size, 'XYZ')

    R.gStyle.SetTitleXOffset(0.5 * scale_h * (1.2 * (def_h * b * scale_h - 0.6 * title_px)) / title_px)
    R.gStyle.SetTitleYOffset(0.5 * scale_w * (1.2 * (def_w * l * scale_w - 0.6 * title_px)) / title_px)

    # Only draw ticks where we have an axis
    R.gStyle.SetPadTickX(0)
    R.gStyle.SetPadTickY(0)
    R.gStyle.SetTickLength(0.02, 'XYZ')

    R.gStyle.SetLegendBorderSize(0)
    R.gStyle.SetLegendFont(42)
    R.gStyle.SetLegendFillColor(0)
    R.gStyle.SetFillColor(0)

    R.gROOT.ForceStyle()
    return

def Set(obj, **kwargs):
    for key, value in six.iteritems(kwargs):
        if value is None:
            getattr(obj, 'Set' + key)()
        elif isinstance(value, (list, tuple)):
            getattr(obj, 'Set' + key)(*value)
        else:
            getattr(obj, 'Set' + key)(value)
    return

def CreateAxisHist(src, at_limits=True):
    backup = R.gPad
    tmp = R.TCanvas()
    tmp.cd()
    src.Draw('AP')
    result = src.GetHistogram().Clone('tmp')
    if (at_limits):
        min = 0.
        max = 0.
        x = ctypes.c_double(0.)
        y = ctypes.c_double(0.)
        src.GetPoint(0, x, y)
        min = float(x.value)
        max = float(x.value)
        for i in range(1, src.GetN()):
            src.GetPoint(i, x, y)
            if x.value < min:
                min = float(x.value)
            if x.value > max:
                max = float(x.value)
        result.GetXaxis().SetLimits(min, max)
    R.gPad = backup
    return result


def CreateAxisHists(n, src, at_limits):
    res = []
    h = CreateAxisHist(src, at_limits)
    for i in range(n):
        res.append(h.Clone('tmp%i'%i))
    return res


def GetAxisHist(pad):
    pad_obs = pad.GetListOfPrimitives()
    if pad_obs is None:
        return None
    obj = None
    for obj in pad_obs:
        if obj.InheritsFrom(R.TH1.Class()):
            return obj
        if obj.InheritsFrom(R.TMultiGraph.Class()):
            return obj.GetHistogram()
        if obj.InheritsFrom(R.TGraph.Class()):
            return obj.GetHistogram()
        if obj.InheritsFrom(R.THStack.Class()):
            return obj.GetHistogram()
    return None


def Get(file, obj):
    R.TH1.AddDirectory(False)
    f_in = R.TFile(file)
    res = R.gDirectory.Get(obj)
    f_in.Close()
    return res


def LimitTGraphFromJSON(js, label):
    xvals = []
    yvals = []
    for key in js:
        xvals.append(float(key))
        yvals.append(js[key][label])
    graph = R.TGraph(len(xvals), array('d', xvals), array('d', yvals))
    graph.Sort()
    return graph


def LimitTGraphFromJSONFile(jsfile, label):
    with open(jsfile) as jsonfile:
        js = json.load(jsonfile)
    return LimitTGraphFromJSON(js, label)

def LimitBandTGraphFromJSON(js, central, lo, hi):
    xvals = []
    yvals = []
    yvals_lo = []
    yvals_hi = []
    for key in js:
        xvals.append(float(key))
        yvals.append(js[key][central])
        yvals_lo.append(js[key][central] - js[key][lo])
        yvals_hi.append(js[key][hi] - js[key][central])
    graph = R.TGraphAsymmErrors(len(xvals), array('d', xvals), array('d', yvals), array('d', [0]), array('d', [0]), array('d', yvals_lo), array('d', yvals_hi))
    graph.Sort()
    return graph


def StandardLimitsFromJSONFile(json_file, draw=['obs', 'exp0', 'exp1', 'exp2', 'theo']):
    graphs = {}
    data = {}
    with open(json_file) as jsonfile:
        data = json.load(jsonfile)
    if 'obs' in draw:
        graphs['obs'] = LimitTGraphFromJSON(data, 'obs')
    if 'theo' in draw:
        graphs['theo'] = LimitTGraphFromJSON(data, 'theo')
    if 'exp0' in draw or 'exp' in draw:
        graphs['exp0'] = LimitTGraphFromJSON(data, 'exp0')
    if 'exp1' in draw or 'exp' in draw:
        graphs['exp1'] = LimitBandTGraphFromJSON(data, 'exp0', 'exp-1', 'exp+1')
    if 'exp2' in draw or 'exp' in draw:
        graphs['exp2'] = LimitBandTGraphFromJSON(data, 'exp0', 'exp-2', 'exp+2')
    return graphs

def FixBothRanges(pad, fix_y_lo, frac_lo, fix_y_hi, frac_hi):
    """Adjusts y-axis range such that a lower and a higher value are located a
    fixed fraction of the frame height away from a new minimum and maximum
    respectively.

    This function is useful in conjunction with GetPadYMax which returns the
    maximum or minimum y value of all histograms and graphs drawn on the pad.

    In the example below, the minimum and maximum values found via this function
    are used as the `fix_y_lo` and `fix_y_hi` arguments, and the spacing fractions
    as 0.15 and 0.30 respectively.

    @code
    FixBothRanges(pad, GetPadYMin(pad), 0.15, GetPadYMax(pad), 0.30)
    @endcode

    ![](figures/FixBothRanges.png)

    Args:
        pad (TPad): A TPad on which histograms and graphs have already been drawn
        fix_y_lo (float): The y value which will end up a fraction `frac_lo` above
                          the new axis minimum.
        frac_lo (float): A fraction of the y-axis height
        fix_y_hi (float): The y value which will end up a fraction `frac_hi` below
                         from the new axis maximum.
        frac_hi (float): A fraction of the y-axis height
    """
    hobj = GetAxisHist(pad)
    ymin = fix_y_lo
    ymax = fix_y_hi
    if R.gPad.GetLogy():
        if ymin == 0.:
            print('Cannot adjust log-scale y-axis range if the minimum is zero!')
            return
        ymin = math.log10(ymin)
        ymax = math.log10(ymax)
    fl = frac_lo
    fh = frac_hi

    ymaxn = (
        (1. / (1. - (fh*fl/((1.-fl)*(1.-fh))))) *
        (1. / (1. - fh)) *
        (ymax - fh*ymin)
        )
    yminn = (ymin - fl*ymaxn) / (1. - fl)
    if R.gPad.GetLogy():
        yminn = math.pow(10, yminn)
        ymaxn = math.pow(10, ymaxn)
    hobj.SetMinimum(yminn)
    hobj.SetMaximum(ymaxn)

def PositionedLegend(width, height, pos, offset, horizontaloffset=None):
    o = offset
    ho = horizontaloffset
    if not ho:
        ho = o
    w = width
    h = height
    l = R.gPad.GetLeftMargin()
    t = R.gPad.GetTopMargin()
    b = R.gPad.GetBottomMargin()
    r = R.gPad.GetRightMargin()
    if pos == 1:
        return R.TLegend(l + ho, 1 - t - o - h, l + ho + w, 1 - t - o, '', 'NBNDC')
    if pos == 2:
        c = l + 0.5 * (1 - l - r)
        return R.TLegend(c - 0.5 * w, 1 - t - o - h, c + 0.5 * w, 1 - t - o, '', 'NBNDC')
    if pos == 3:
        return R.TLegend(1 - r - ho - w, 1 - t - o - h, 1 - r - ho, 1 - t - o, '', 'NBNDC')
    if pos == 4:
        return R.TLegend(l + ho, b + o, l + ho + w, b + o + h, '', 'NBNDC')
    if pos == 5:
        c = l + 0.5 * (1 - l - r)
        return R.TLegend(c - 0.5 * w, b + o, c + 0.5 * w, b + o + h, '', 'NBNDC')
    if pos == 6:
        return R.TLegend(1 - r - ho - w, b + o, 1 - r - ho, b + o + h, '', 'NBNDC')


##@}


## @name Limit plotting
#
#  @details Common limit-plotting tasks, for example setting the Brazilian
#      colour scheme for expected limit bands and drawing the associated
#      TGraphs in the correct order
##@{

def StyleLimitBand(graph_dict, overwrite_style_dict=None):
    style_dict = {
            'obs' : { 'LineWidth' : 2},
            'theo' : { 'LineWidth' : 2, 'LineColor' : R.kBlue},
            'exp0' : { 'LineWidth' : 2, 'LineColor' : R.kRed},
            'exp1' : { 'FillColor' : R.kGreen},
            'exp2' : { 'FillColor' : R.kYellow}
            }
    if overwrite_style_dict is not None:
        for key in overwrite_style_dict:
            if key in style_dict:
                style_dict[key].update(overwrite_style_dict[key])
            else:
                style_dict[key] = overwrite_style_dict[key]
    for key in graph_dict:
        Set(graph_dict[key],**style_dict[key])

def DrawLimitBand(pad, graph_dict, draw=['exp2', 'exp1', 'exp0', 'obs', 'theo'], draw_legend=None,
                  legend=None, legend_overwrite=None):
    legend_dict = {
        'obs' : { 'Label' : 'Observed', 'LegendStyle' : 'LP', 'DrawStyle' : 'PLSAME'},
        'theo' : { 'Label' : 'Theory', 'LegendStyle' : 'L', 'DrawStyle' : 'LSAME'},
        'exp0' : { 'Label' : 'Expected', 'LegendStyle' : 'L', 'DrawStyle' : 'LSAME'},
        'exp1' : { 'Label' : '#pm1#sigma Expected', 'LegendStyle' : 'F', 'DrawStyle' : '3SAME'},
        'exp2' : { 'Label' : '#pm2#sigma Expected', 'LegendStyle' : 'F', 'DrawStyle' : '3SAME'}
    }
    if legend_overwrite is not None:
        for key in legend_overwrite:
            if key in legend_dict:
                legend_dict[key].update(legend_overwrite[key])
            else:
                legend_dict[key] = legend_overwrite[key]
    pad.cd()
    for key in draw:
        if key in graph_dict:
            graph_dict[key].Draw(legend_dict[key]['DrawStyle'])
    if legend is not None:
        if draw_legend is None:
            draw_legend = reversed(draw)
        for key in draw_legend:
            if key in graph_dict:
                legend.AddEntry(graph_dict[key],legend_dict[key]['Label'],legend_dict[key]['LegendStyle'])


