#ifndef CUTS_H
#define CUTS_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// VBS WH
#include "../core.h"

bool isSFOS(int pdgID_1, int pdgID_2)
{
    return (pdgID_1 == (-1)*pdgID_2);
};

class Has3LepsPresel : public VBSWHCut
{
public:
    Has3LepsPresel(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
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
        return (n_lep_pt_gt_20 >= 3);
    };

    float weight()
    {
        return 1.0;
    };
};

class VBSNoBTagPresel : public VBSWHCut
{
public:
    VBSNoBTagPresel(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        return (arbol.getLeaf<int>("n_jets_pt30") >= 2) && (arbol.getLeaf<int>("n_tight_b_jets") == 0);
    };
};

class Has3Leps : public VBSWHCut
{
public:
    Has3Leps(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        Integers good_lep_pdgIDs = globals.getVal<Integers>("good_lep_pdgIDs");
        Integers good_lep_idxs = globals.getVal<Integers>("good_lep_idxs");
        int n_loose_leps = 0;
        Integers trilep_idxs;
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
                    trilep_idxs.push_back(good_lep_i);
                }
                break;
            case 13:
                if (ttH::muonID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH::muonID(lep_i, ttH::IDtight, nt.year())) 
                {
                    trilep_idxs.push_back(good_lep_i);
                }
                break;
            }
        }
        // Require 3 and only 3 leptons (all passing tight ID)
        if (trilep_idxs.size() != 3 || n_loose_leps != 3) { return false; }
        // Sort into leading, middling, and trailing
        std::sort(
            trilep_idxs.begin(), trilep_idxs.end(),
            [&](int i, int j) -> bool { return good_lep_p4s.at(i).pt() > good_lep_p4s.at(j).pt(); }
        );
        int ld_i = trilep_idxs.at(0); // leading
        int md_i = trilep_idxs.at(1); // middling
        int tr_i = trilep_idxs.at(2); // trailing
        int ld_lep_pdgID = good_lep_pdgIDs.at(ld_i);
        int md_lep_pdgID = good_lep_pdgIDs.at(md_i);
        int tr_lep_pdgID = good_lep_pdgIDs.at(tr_i);
        arbol.setLeaf<int>("ld_lep_pdgID", ld_lep_pdgID);
        arbol.setLeaf<int>("md_lep_pdgID", md_lep_pdgID);
        arbol.setLeaf<int>("tr_lep_pdgID", tr_lep_pdgID);
        LorentzVector ld_lep_p4 = good_lep_p4s.at(ld_i);
        LorentzVector md_lep_p4 = good_lep_p4s.at(md_i);
        LorentzVector tr_lep_p4 = good_lep_p4s.at(tr_i);
        globals.setVal<LorentzVector>("ld_lep_p4", ld_lep_p4);
        globals.setVal<LorentzVector>("md_lep_p4", md_lep_p4);
        globals.setVal<LorentzVector>("tr_lep_p4", tr_lep_p4);
        arbol.setLeaf<float>("ld_lep_pt", ld_lep_p4.pt());
        arbol.setLeaf<float>("md_lep_pt", md_lep_p4.pt());
        arbol.setLeaf<float>("tr_lep_pt", tr_lep_p4.pt());
        arbol.setLeaf<float>("ld_lep_eta", ld_lep_p4.eta());
        arbol.setLeaf<float>("md_lep_eta", md_lep_p4.eta());
        arbol.setLeaf<float>("tr_lep_eta", tr_lep_p4.eta());
        arbol.setLeaf<float>("ld_lep_phi", ld_lep_p4.phi());
        arbol.setLeaf<float>("md_lep_phi", md_lep_p4.phi());
        arbol.setLeaf<float>("tr_lep_phi", tr_lep_p4.phi());
        arbol.setLeaf<float>(
            "LT", 
            (ld_lep_p4.pt() 
             + md_lep_p4.pt() 
             + tr_lep_p4.pt()
             + arbol.getLeaf<float>("MET"))
        );
        // Gather Z-hypotheses for Z-veto (and count SFOS pairs implicitly)
        LorentzVectors Z_hyp_p4s;
        if (isSFOS(ld_lep_pdgID, md_lep_pdgID)) 
        { 
            Z_hyp_p4s.push_back(ld_lep_p4 + md_lep_p4);
        }
        if (isSFOS(md_lep_pdgID, tr_lep_pdgID)) 
        {
            Z_hyp_p4s.push_back(md_lep_p4 + tr_lep_p4);
        }
        if (isSFOS(tr_lep_pdgID, ld_lep_pdgID)) 
        {
            Z_hyp_p4s.push_back(tr_lep_p4 + ld_lep_p4);
        }
        arbol.setLeaf<int>("n_SFOS_lep_pairs", Z_hyp_p4s.size());
        globals.setVal<LorentzVectors>("Z_hyp_p4s", Z_hyp_p4s);
        return true;
    };
};

class SMLike3Leps : public VBSWHCut
{
public:
    SMLike3Leps(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        float ld_lep_pt = arbol.getLeaf<float>("ld_lep_pt");
        float md_lep_pt = arbol.getLeaf<float>("md_lep_pt");
        float tr_lep_pt = arbol.getLeaf<float>("tr_lep_pt");
        return (ld_lep_pt > 25) && (md_lep_pt > 15) && (tr_lep_pt > 10);
    };
};

class BSMLike3Leps : public VBSWHCut
{
public:
    BSMLike3Leps(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        float ld_lep_pt = arbol.getLeaf<float>("ld_lep_pt");
        float md_lep_pt = arbol.getLeaf<float>("md_lep_pt");
        float tr_lep_pt = arbol.getLeaf<float>("tr_lep_pt");
        return (ld_lep_pt > 40) && (md_lep_pt > 40) && (tr_lep_pt > 40);
    };
};

class ZVeto3Leps : public VBSWHCut
{
public:
    ZVeto3Leps(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors non_Z_hyp_p4s;
        LorentzVectors Z_hyp_p4s = globals.getVal<LorentzVectors>("Z_hyp_p4s");
        for (auto& Z_hyp_p4 : Z_hyp_p4s)
        {
            if (Z_hyp_p4.mass() > 110 || Z_hyp_p4.mass() < 70)
            {
                non_Z_hyp_p4s.push_back(Z_hyp_p4);
            }
        }
        bool passed_Z_veto = non_Z_hyp_p4s.size() == Z_hyp_p4s.size();
        if (non_Z_hyp_p4s.size() == 1)
        {
            arbol.setLeaf<float>("non_Z_dilep_M_T", non_Z_hyp_p4s.at(0).mt());
        }
        arbol.setLeaf<bool>("passed_Z_veto", passed_Z_veto);
        return passed_Z_veto;
    };
};

#endif
