#ifndef CORE_H
#define CORE_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "ElectronSelections.h"
#include "MuonSelections.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<float> Floats;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

struct VBSWHAnalysis
{
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;

    VBSWHAnalysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbol(arbol_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;

        arbol.newBranch<float>("xsec_sf", -999);
        arbol.newBranch<int>("event", -999);
        arbol.newBranch<float>("MET", -999);
        arbol.newBranch<float>("ST", -999);
        // Lepton branches
        arbol.newBranch<float>("lep_sf", -999);
        arbol.newBranch<float>("lep_sf_up", -999);
        arbol.newBranch<float>("lep_sf_dn", -999);
        // Jet (AK4) branches
        arbol.newBranch<float>("HT", -999);
        arbol.newBranch<int>("n_jets_pt30", -999);
        // Jet (AK8) branches
        arbol.newBranch<int>("n_fatjets", -999);
        // b-jet branches
        arbol.newBranch<int>("n_loose_b_jets", -999);
        arbol.newBranch<int>("n_medium_b_jets", -999);
        arbol.newBranch<int>("n_tight_b_jets", -999);
        // VBS jet branches
        arbol.newBranch<float>("ld_vbs_jet_pt", -999);
        arbol.newBranch<float>("tr_vbs_jet_pt", -999);
        arbol.newBranch<float>("M_jj", -999);
        arbol.newBranch<float>("deta_jj", -999);
        arbol.newBranch<float>("dR_jj", -999);
        // Lepton globals
        cutflow.globals.newVar<LorentzVectors>("good_lep_p4s", {});
        cutflow.globals.newVar<Integers>("good_lep_pdgIDs", {});
        cutflow.globals.newVar<Integers>("good_lep_idxs", {});
        cutflow.globals.newVar<Integers>("good_lep_jet_idxs", {});
        // Jet globals
        cutflow.globals.newVar<LorentzVectors>("good_jet_p4s", {});
        cutflow.globals.newVar<Integers>("good_jet_idxs", {});
        cutflow.globals.newVar<Floats>("good_jet_btags", {});
        // Fat jet (AK8) globals
        cutflow.globals.newVar<LorentzVectors>("good_fatjet_p4s", {});
        cutflow.globals.newVar<Integers>("good_fatjet_idxs", {});
        cutflow.globals.newVar<Floats>("good_fatjet_hbbtags", {}); // ParticleNet
        cutflow.globals.newVar<Floats>("good_fatjet_masses", {});
        cutflow.globals.newVar<Floats>("good_fatjet_msoftdrops", {});
        // VBS jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vbs_jet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vbs_jet_p4");
        cutflow.globals.newVar<int>("ld_vbs_jet_idx");
        cutflow.globals.newVar<int>("tr_vbs_jet_idx");
    };
};

class VBSWHCut : public Cut
{
public:
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;

    VBSWHCut(std::string new_name, VBSWHAnalysis& a) 
    : Cut(new_name), arbol(a.arbol), nt(a.nt), cli(a.cli), globals(a.cutflow.globals)
    {
        // Do nothing
    };
};

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

class SelectLeptons : public VBSWHCut
{
public:
    SelectLeptons(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        // Lepton ID sf
        float lep_sf = 1.;
        // Percent errors (up/down) on sf
        float err_up = 0.;
        float err_dn = 0.;

        LorentzVectors good_lep_p4s;
        Integers good_lep_pdgIDs;
        Integers good_lep_idxs;
        Integers good_lep_jet_idxs;
        // Loop over electrons
        for (unsigned int i = 0; i < nt.nElectron(); ++i)
        {
            if (!ttH::electronID(i, ttH::IDveto, nt.year())) { continue; }
            LorentzVector el_p4 = nt.Electron_p4().at(i);
            // if (el_p4.pt() <= 40) { continue; } // DEBUG: add back in!!
            // Store basic info
            good_lep_p4s.push_back(el_p4);
            good_lep_pdgIDs.push_back(-nt.Electron_charge().at(i)*11);
            good_lep_idxs.push_back(i);
            good_lep_jet_idxs.push_back(nt.Electron_jetIdx().at(i));
            if (cli.is_data) { continue; }
            /* FIXME: add lepton sf code
            // Get scale factor (and up/down variations)
            float el_eta = std::max(std::min(el_p4.eta(), 2.4999f), -2.4999f);
            float el_pt = el_p4.pt();
            // event --> reco
            lep_sf *= ttH::getElecRecoEffSFUL(el_eta, el_pt, nt.year());
            err_up += std::pow(ttH::getElecRecoEffSFULErr(el_eta, el_pt, nt.year()), 2);
            err_dn += std::pow(ttH::getElecRecoEffSFULErr(el_eta, el_pt, nt.year()), 2);
            // reco --> loose POG ID
            lep_sf *= ttH::getElecPOGLooseSF(el_eta, el_pt, nt.year());
            err_up += std::pow(ttH::getElecPOGLooseSFErr(el_eta, el_pt, nt.year()), 2);
            err_dn += std::pow(ttH::getElecPOGLooseSFErr(el_eta, el_pt, nt.year()), 2);
            // loose POG ID --> loose ttH ID --> tight ttH ID
            lep_sf *= ttH::getElecLooseSF(el_eta, el_pt, nt.year());
            lep_sf *= ttH::getElecTightSF(el_eta, el_pt, nt.year());
            err_up += std::pow(ttH::getElecTTHSFErr(el_eta, el_pt, nt.year(), true), 2);
            err_dn += std::pow(ttH::getElecTTHSFErr(el_eta, el_pt, nt.year(), false), 2);
            */
        }
        // Loop over muons
        for (unsigned int i = 0; i < nt.nMuon(); ++i)
        {
            if (!ttH::muonID(i, ttH::IDveto, nt.year())) { continue; }
            LorentzVector mu_p4 = nt.Muon_p4().at(i);
            // if (mu_p4.pt() <= 40) { continue; } // DEBUG: add back in!!
            // Store basic info
            good_lep_p4s.push_back(mu_p4);
            good_lep_pdgIDs.push_back(-nt.Muon_charge().at(i)*13);
            good_lep_idxs.push_back(i);
            good_lep_jet_idxs.push_back(nt.Muon_jetIdx().at(i));
            if (cli.is_data) { continue; }
            /* FIXME: add lepton sf code
            // Get scale factor (and up/down variations)
            float mu_eta = mu_p4.eta();
            float mu_pt = mu_p4.pt();
            // medium POG ID --> loose ttH ID --> tight ttH ID (NOTE: POG sf is folded into ttH sf)
            lep_sf *= ttH::getMuonLooseSF(mu_eta, mu_pt, nt.year());
            lep_sf *= ttH::getMuonTightSF(mu_eta, mu_pt, nt.year());
            err_up += std::pow(ttH::getMuonTTHSFErr(mu_eta, mu_pt, nt.year(), true), 2);
            err_dn += std::pow(ttH::getMuonTTHSFErr(mu_eta, mu_pt, nt.year(), false), 2);
            */
        }
        // Store lepton sf and its up/down variations
        if (!cli.is_data)
        {
            // Finish error computation
            err_up = std::sqrt(err_up);
            err_dn = std::sqrt(err_dn);
            arbol.setLeaf<float>("lep_sf", lep_sf);
            arbol.setLeaf<float>("lep_sf_up", lep_sf + err_up*lep_sf);
            arbol.setLeaf<float>("lep_sf_dn", lep_sf - err_dn*lep_sf);
        }
        globals.setVal<LorentzVectors>("good_lep_p4s", good_lep_p4s);
        globals.setVal<Integers>("good_lep_pdgIDs", good_lep_pdgIDs);
        globals.setVal<Integers>("good_lep_idxs", good_lep_idxs);
        globals.setVal<Integers>("good_lep_jet_idxs", good_lep_jet_idxs);
        return true;
    };
};

class SelectJets : public VBSWHCut
{
public:
    LorentzVectors good_lep_p4s;
    Integers good_lep_jet_idxs;

    SelectJets(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    virtual void loadOverlapVars()
    {
        good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        good_lep_jet_idxs = globals.getVal<Integers>("good_lep_jet_idxs");
    };

    bool overlapsLepton(int jet_i, LorentzVector jet_p4)
    {
        for (unsigned int lep_i = 0; lep_i < good_lep_p4s.size(); ++lep_i)
        {
            int lep_jet_idx = good_lep_jet_idxs.at(lep_i);
            if (lep_jet_idx == -999)
            {
                LorentzVector lep_p4 = good_lep_p4s.at(lep_i);
                if (ROOT::Math::VectorUtil::DeltaR(lep_p4, jet_p4) < 0.4) 
                {
                    return true;
                }
            }
            else if (lep_jet_idx == int(jet_i))
            {
                return true;
            }
        }
        return false;
    };

    virtual bool isOverlap(int jet_i, LorentzVector jet_p4)
    {
        return overlapsLepton(jet_i, jet_p4);
    };

    bool evaluate()
    {
        loadOverlapVars();
        int n_loose_b_jets = 0;
        int n_medium_b_jets = 0;
        int n_tight_b_jets = 0;
        int n_jets_pt30 = 0;
        float ht = 0.;
        Floats good_jet_btags;
        LorentzVectors good_jet_p4s;
        Integers good_jet_idxs;
        for (unsigned int jet_i = 0; jet_i < nt.nJet(); ++jet_i)
        {
            // Read jet p4
            LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
            /* FIXME: add JEC variation code
            // Apply up/down JECs
            if (jec_var == 1 || jec_var == -1)
            {
                jec_unc->setJetEta(jet_p4.eta());
                jec_unc->setJetPt(jet_p4.pt());
                float jec_err = abs(jec_unc->getUncertainty(jec_var == 1))*jec_var;
                jet_p4 *= (1. + jec_err);
            }
            */
            if (jet_p4.pt() < 20) { continue; }
            // Apply jet ID
            int jet_id = nt.Jet_jetId().at(jet_i);
            if (nt.year() == 2016 && jet_id < 1) { continue; }
            if (nt.year() > 2016 && jet_id < 2) { continue; }
            // Perform overlap check against leptons
            if (isOverlap(jet_i, jet_p4)) { continue; }
            // Perform b-tagging (for b-veto); only possible for pt > 20 GeV and |eta| < 2.4
            bool is_btagged_loose = false;
            bool is_btagged_medium = false;
            bool is_btagged_tight = false;
            float deepflav_btag = nt.Jet_btagDeepFlavB().at(jet_i);
            if (std::abs(jet_p4.eta()) < 2.4) 
            {
                // Check DeepJet vs. working points in NanoCORE global config (gconf)
                is_btagged_loose = deepflav_btag > gconf.WP_DeepFlav_loose;
                is_btagged_medium = deepflav_btag > gconf.WP_DeepFlav_medium;
                is_btagged_tight = deepflav_btag > gconf.WP_DeepFlav_tight;
            }
            // Count b-tags and save jet info
            if (is_btagged_tight) 
            {
                good_jet_btags.push_back(deepflav_btag);
                n_tight_b_jets++;
            }
            else if (is_btagged_medium)
            {
                good_jet_btags.push_back(deepflav_btag);
                n_medium_b_jets++;
            }
            else if (is_btagged_loose)
            {
                good_jet_btags.push_back(deepflav_btag);
                n_loose_b_jets++;
            }
            else
            {
                good_jet_btags.push_back(-999.);
            }
            // Save additional jet info
            n_jets_pt30++;
            ht += jet_p4.pt();
            good_jet_p4s.push_back(jet_p4);
            good_jet_idxs.push_back(jet_i);
        }
        globals.setVal<Floats>("good_jet_btags", good_jet_btags);
        globals.setVal<LorentzVectors>("good_jet_p4s", good_jet_p4s);
        globals.setVal<Integers>("good_jet_idxs", good_jet_idxs);
        arbol.setLeaf<int>("n_loose_b_jets", n_loose_b_jets);
        arbol.setLeaf<int>("n_medium_b_jets", n_medium_b_jets);
        arbol.setLeaf<int>("n_tight_b_jets", n_tight_b_jets);
        arbol.setLeaf<int>("n_jets_pt30", n_jets_pt30);
        arbol.setLeaf<float>("HT", ht);
        return true;
    };
};

class SelectFatJets : public VBSWHCut
{
public:
    SelectFatJets(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_fatjet_p4s;
        Integers good_fatjet_idxs;
        Floats good_fatjet_hbbtags;
        Floats good_fatjet_masses;
        Floats good_fatjet_msoftdrops;
        LorentzVectors good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); ++fatjet_i)
        {
            LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);

            // Basic requirements
            if (fatjet_p4.pt() <= 250) { continue; }
            if (nt.FatJet_mass().at(fatjet_i) <= 50) { continue; }
            if (nt.FatJet_msoftdrop().at(fatjet_i) <= 40) { continue; }
            // Remove lepton overlap
            bool is_overlap = false;
            for (auto lep_p4 : good_lep_p4s)
            {
                if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8) 
                {
                    is_overlap = true;
                    break;
                }
            }
            if (is_overlap) { continue; }

            // Store good fat jets
            good_fatjet_p4s.push_back(fatjet_p4);
            good_fatjet_idxs.push_back(fatjet_i);
            good_fatjet_hbbtags.push_back(nt.FatJet_particleNet_HbbvsQCD().at(fatjet_i));
            good_fatjet_masses.push_back(nt.FatJet_mass().at(fatjet_i));
            good_fatjet_msoftdrops.push_back(nt.FatJet_msoftdrop().at(fatjet_i));
        }
        globals.setVal<LorentzVectors>("good_fatjet_p4s", good_fatjet_p4s);
        globals.setVal<Integers>("good_fatjet_idxs", good_fatjet_idxs);
        globals.setVal<Floats>("good_fatjet_hbbtags", good_fatjet_hbbtags);
        globals.setVal<Floats>("good_fatjet_masses", good_fatjet_masses);
        globals.setVal<Floats>("good_fatjet_msoftdrops", good_fatjet_msoftdrops);
        arbol.setLeaf<int>("n_fatjets", good_fatjet_p4s.size());
        return true;
    };
};

class SelectVBSJetsMaxE : public VBSWHCut
{
public:
    SelectVBSJetsMaxE(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        // Select VBS jet candidates
        std::vector<int> vbs_jet_cand_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            if (good_jet_p4s.at(jet_i).pt() >= 30.) { vbs_jet_cand_idxs.push_back(jet_i); }
        }
        if (vbs_jet_cand_idxs.size() < 2) { return false; }
        // Sort candidates by pt
        std::sort(
            vbs_jet_cand_idxs.begin(), vbs_jet_cand_idxs.end(),
            [&](int i, int j) -> bool { return good_jet_p4s.at(i).pt() > good_jet_p4s.at(j).pt(); }
        );
        // Process candidates
        std::pair<int, int> vbs_jet_idxs;
        if (vbs_jet_cand_idxs.size() == 2)
        {
            vbs_jet_idxs = std::make_pair(vbs_jet_cand_idxs.at(0), vbs_jet_cand_idxs.at(1));
        }
        else
        {
            // Collect jets in pos/neg eta hemispheres
            std::vector<int> vbs_pos_eta_jet_idxs;
            std::vector<int> vbs_neg_eta_jet_idxs;
            for (auto& jet_i : vbs_jet_cand_idxs)
            {
                const LorentzVector& jet_p4 = good_jet_p4s.at(jet_i);
                if (jet_p4.eta() >= 0)
                {
                    vbs_pos_eta_jet_idxs.push_back(jet_i);
                }
                else
                {
                    vbs_neg_eta_jet_idxs.push_back(jet_i);
                }
            }
            // Sort the jets in each hemisphere by P
            std::sort(
                vbs_pos_eta_jet_idxs.begin(), vbs_pos_eta_jet_idxs.end(),
                [&](int i, int j) -> bool { return good_jet_p4s.at(i).P() > good_jet_p4s.at(j).P(); }
            );
            std::sort(
                vbs_neg_eta_jet_idxs.begin(), vbs_neg_eta_jet_idxs.end(),
                [&](int i, int j) -> bool { return good_jet_p4s.at(i).P() > good_jet_p4s.at(j).P(); }
            );
            // Select VBS jets
            if (vbs_pos_eta_jet_idxs.size() == 0)
            {
                // All candidates are in the -eta hemisphere
                vbs_jet_idxs = std::make_pair(vbs_neg_eta_jet_idxs.at(0), vbs_neg_eta_jet_idxs.at(1));
            }
            else if (vbs_neg_eta_jet_idxs.size() == 0)
            {
                // All candidates are in the +eta hemisphere
                vbs_jet_idxs = std::make_pair(vbs_pos_eta_jet_idxs.at(0), vbs_pos_eta_jet_idxs.at(1));
            }
            else
            {
                // Take the leading candidate (in P) from each hemisphere
                vbs_jet_idxs = std::make_pair(vbs_pos_eta_jet_idxs.at(0), vbs_neg_eta_jet_idxs.at(0));
            }
        }
        // Separate the two VBS jets into leading/trailing
        int ld_vbs_jet_idx;
        int tr_vbs_jet_idx;
        if (good_jet_p4s.at(vbs_jet_idxs.first).pt() > good_jet_p4s.at(vbs_jet_idxs.first).pt())
        {
            ld_vbs_jet_idx = vbs_jet_idxs.first;
            tr_vbs_jet_idx = vbs_jet_idxs.second;
        }
        else
        {
            ld_vbs_jet_idx = vbs_jet_idxs.second;
            tr_vbs_jet_idx = vbs_jet_idxs.first;
        }
        LorentzVector ld_vbs_jet_p4 = good_jet_p4s.at(ld_vbs_jet_idx);
        LorentzVector tr_vbs_jet_p4 = good_jet_p4s.at(tr_vbs_jet_idx);
        // Save VBS jet info
        globals.setVal<LorentzVector>("ld_vbs_jet_p4", ld_vbs_jet_p4);
        globals.setVal<int>("ld_vbs_jet_idx", ld_vbs_jet_idx);
        globals.setVal<LorentzVector>("tr_vbs_jet_p4", tr_vbs_jet_p4);
        globals.setVal<int>("tr_vbs_jet_idx", tr_vbs_jet_idx);
        arbol.setLeaf<float>("ld_vbs_jet_pt", ld_vbs_jet_p4.pt());
        arbol.setLeaf<float>("tr_vbs_jet_pt", tr_vbs_jet_p4.pt());
        arbol.setLeaf<float>("M_jj", (ld_vbs_jet_p4 + tr_vbs_jet_p4).M());
        arbol.setLeaf<float>("deta_jj", ld_vbs_jet_p4.eta() - tr_vbs_jet_p4.eta());
        arbol.setLeaf<float>("dR_jj", ROOT::Math::VectorUtil::DeltaR(ld_vbs_jet_p4, tr_vbs_jet_p4));
        return true;
    };
};

class VBSPresel : public VBSWHCut
{
public:
    VBSPresel(std::string name, VBSWHAnalysis& analysis) : VBSWHCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        return (arbol.getLeaf<float>("M_jj") > 500) && (arbol.getLeaf<float>("deta_jj") > 3);
    };
};

#endif
