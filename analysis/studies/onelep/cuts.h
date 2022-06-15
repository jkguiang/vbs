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
        // Select fatjet with best (highest) ParticleNet Hbb score
        Doubles good_fatjet_hbbtags = globals.getVal<Doubles>("good_fatjet_hbbtags");
        int best_hbbjet_i = std::distance(
            good_fatjet_hbbtags.begin(), 
            std::max_element(good_fatjet_hbbtags.begin(), good_fatjet_hbbtags.end())
        );
        // Store the fatjet
        LorentzVector best_hbbjet_p4 = globals.getVal<LorentzVectors>("good_fatjet_p4s").at(best_hbbjet_i);
        globals.setVal<LorentzVector>("hbbjet_p4", best_hbbjet_p4);
        arbol.setLeaf<double>("hbbjet_score", globals.getVal<Doubles>("good_fatjet_hbbtags").at(best_hbbjet_i));
        arbol.setLeaf<double>("hbbjet_pt", best_hbbjet_p4.pt());
        arbol.setLeaf<double>("hbbjet_eta", best_hbbjet_p4.eta());
        arbol.setLeaf<double>("hbbjet_phi", best_hbbjet_p4.phi());
        arbol.setLeaf<double>("hbbjet_mass", globals.getVal<Doubles>("good_fatjet_masses").at(best_hbbjet_i));
        arbol.setLeaf<double>("hbbjet_msoftdrop", globals.getVal<Doubles>("good_fatjet_msoftdrops").at(best_hbbjet_i));
        arbol.setLeaf<double>(
            "ST", 
            (arbol.getLeaf<double>("LT") + best_hbbjet_p4.pt())
        );
        return true;
    };
};

class SelectJetsNoHbbOverlap : public SelectJets
{
public:
    LorentzVector hbbjet_p4;

    SelectJetsNoHbbOverlap(std::string name, VBSWHAnalysis& analysis) : SelectJets(name, analysis) 
    {
        // Do nothing
    };

    void loadOverlapVars()
    {
        good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        good_lep_jet_idxs = globals.getVal<Integers>("good_lep_jet_idxs");
        hbbjet_p4 = globals.getVal<LorentzVector>("hbbjet_p4");
    };

    bool overlapsHbbJet(LorentzVector jet_p4)
    {
        return ROOT::Math::VectorUtil::DeltaR(hbbjet_p4, jet_p4) < 0.8;
    };

    bool isOverlap(int jet_i, LorentzVector jet_p4)
    {
        return overlapsLepton(jet_i, jet_p4) or overlapsHbbJet(jet_p4);
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
        int n_tight_leps = 0;
        int tight_lep_idx = -999;
        for (unsigned int good_lep_i = 0; good_lep_i < good_lep_p4s.size(); ++good_lep_i)
        {
            // Count loose and tight leptons
            unsigned int lep_i = good_lep_idxs.at(good_lep_i);
            int lep_pdgID = good_lep_pdgIDs.at(good_lep_i);
            if (abs(lep_pdgID) == 11)
            {
                if (ttH::electronID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH::electronID(lep_i, ttH::IDtight, nt.year())) 
                {
                    n_tight_leps++;
                    tight_lep_idx = good_lep_i;
                }
            }
            else if (abs(lep_pdgID) == 13)
            {
                if (ttH::muonID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH::muonID(lep_i, ttH::IDtight, nt.year())) 
                {
                    n_tight_leps++;
                    tight_lep_idx = good_lep_i;
                }
            }
        }
        // Require 1 and only 1 lepton (all passing tight ID)
        if (n_tight_leps != 1 || n_loose_leps != 1) { return false; }
        arbol.setLeaf<int>("lep_pdgID", good_lep_pdgIDs.at(tight_lep_idx));
        LorentzVector lep_p4 = good_lep_p4s.at(tight_lep_idx);
        globals.setVal<LorentzVector>("lep_p4", lep_p4);
        arbol.setLeaf<double>("lep_pt", lep_p4.pt());
        arbol.setLeaf<double>("lep_eta", lep_p4.eta());
        arbol.setLeaf<double>("lep_phi", lep_p4.phi());
        arbol.setLeaf<double>("LT", (lep_p4.pt() + arbol.getLeaf<double>("MET")));
        return true;
    };
};

#endif
