#ifndef CUTS_H
#define CUTS_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// VBS WH
#include "../core.h"

class Bookkeeping : public VBSWHCut
{
public:
    Bookkeeping(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        arbol.setLeaf<float>("xsec_sf", cli.is_data ? 1. : cli.scale_factor*nt.genWeight());
        arbol.setLeaf<int>("event", nt.event());
        arbol.setLeaf<float>("MET", nt.MET_pt());
        return true;
    };

    float weight()
    {
        return arbol.getLeaf<float>("xsec_sf");
    };
};

class Has1LepPresel : public VBSWHCut
{
public:
    Has1LepPresel(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        int n_lep_pt_gt_20 = 0;
        // Loop over electrons
        for (unsigned int i = 0; i < nt.nElectron(); ++i)
        {
            if (nt.Electron_pt().at(i) > 20) { n_lep_pt_gt_20++; }
        }
        // Loop over muons
        for (unsigned int i = 0; i < nt.nMuon(); ++i)
        {
            if (nt.Muon_pt().at(i) > 20) { n_lep_pt_gt_20++; }
        }
        return (n_lep_pt_gt_20 >= 1);
    };

    float weight()
    {
        return 1.0;
    };
};

#endif
