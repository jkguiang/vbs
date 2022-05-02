#ifndef CUTS_H
#define CUTS_H

#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort
// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// VBS WH
#include "../core.h"

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
};

class VBSHbbPresel : public VBSWHCut
{
public:
    VBSHbbPresel(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {

        return (arbol.getLeaf<int>("n_jets_pt30") >= 2) && (globals.getVal<Integers>("good_fatjet_idxs").size() >= 1);
    };
};

class SelectHbbFatJet : public VBSWHCut
{
public:
    SelectHbbFatJet(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        Floats good_fatjet_hbbtags = globals.getVal<Floats>("good_fatjet_hbbtags");
        Integers indices(good_fatjet_hbbtags.size());
        iota(indices.begin(), indices.end(), 0); // fill indices vector
        std::sort(
            indices.begin(), indices.end(),
            [&](int i, int j) -> bool { 
                return good_fatjet_hbbtags.at(i) > good_fatjet_hbbtags.at(j); 
            }
        );
        int best_fatjet_i = indices.at(0);
        LorentzVector best_fatjet_p4 = globals.getVal<LorentzVectors>("good_fatjet_p4s").at(best_fatjet_i);
        arbol.setLeaf<float>("hbbjet_score", good_fatjet_hbbtags.at(best_fatjet_i));
        arbol.setLeaf<float>("hbbjet_pt", best_fatjet_p4.pt());
        arbol.setLeaf<float>("hbbjet_eta", best_fatjet_p4.eta());
        arbol.setLeaf<float>("hbbjet_phi", best_fatjet_p4.phi());
        return true;
    };
};

class Has1Lep : public VBSWHCut
{
public:
    Has1Lep(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        Integers good_lep_pdgIDs = globals.getVal<Integers>("good_lep_pdgIDs");
        Integers good_lep_idxs = globals.getVal<Integers>("good_lep_idxs");
        int n_loose_leps = 0;
        int tight_lep_idx = -999;
        for (unsigned int good_lep_i = 0; good_lep_i < good_lep_p4s.size(); ++good_lep_i)
        {
            // Count loose and tight leptons
            unsigned int lep_i = good_lep_idxs.at(good_lep_i);
            int lep_pdgID = good_lep_pdgIDs.at(good_lep_i);
            switch (abs(lep_pdgID))
            {
            case 11:
                if (ttH::electronID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH::electronID(lep_i, ttH::IDtight, nt.year())) 
                {
                    tight_lep_idx = good_lep_i;
                }
                break;
            case 13:
                if (ttH::muonID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH::muonID(lep_i, ttH::IDtight, nt.year())) 
                {
                    tight_lep_idx = good_lep_i;
                }
                break;
            }
        }
        // Require 1 and only 1 lepton (all passing tight ID)
        if (tight_lep_idx == -999 || n_loose_leps != 1) { return false; }
        arbol.setLeaf<int>("lep_pdgID", good_lep_pdgIDs.at(tight_lep_idx));
        LorentzVector lep_p4 = good_lep_p4s.at(tight_lep_idx);
        globals.setVal<LorentzVector>("lep_p4", lep_p4);
        arbol.setLeaf<float>("lep_pt", lep_p4.pt());
        arbol.setLeaf<float>("lep_eta", lep_p4.eta());
        arbol.setLeaf<float>("lep_phi", lep_p4.phi());
        arbol.setLeaf<float>("LT", (lep_p4.pt() + arbol.getLeaf<float>("MET")));
        arbol.setLeaf<float>(
            "ST", 
            (arbol.getLeaf<float>("LT") + arbol.getLeaf<float>("hbbjet_pt"))
        );
        return true;
    };
};

#endif
