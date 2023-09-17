#ifndef VBSVVHJETS_COLLECTIONS_H
#define VBSVVHJETS_COLLECTIONS_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"          // Utilities::Variables
// VBS
#include "core/collections.h"   // Core::Core::Analysis, Core::Skimmer
#include "core/cuts.h"
#include "vbswh/collections.h"
#include "vbswh/cuts.h"
#include "vbsvvhjets/cuts.h"
#include "corrections/all.h"    // PileUpSFs, LeptonSFsTTH/PKU, BTagSFs, JetEnergyScales

namespace VBSVVHJets
{

struct Analysis : Core::Analysis
{
    JetEnergyScales* jes;
    LeptonSFsPKU* lep_sfs;
    HLT1LepSFs* hlt_sfs;
    BTagSFs* btag_sfs;
    PileUpSFs* pu_sfs;
    PileUpJetIDSFs* puid_sfs;
    bool all_corrections;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // W/Z fat jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vqqfatjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vqqfatjet_p4");
        cutflow.globals.newVar<unsigned int>("ld_vqqfatjet_gidx", 999); // idx in 'good' fatjets global vector
        cutflow.globals.newVar<unsigned int>("tr_vqqfatjet_gidx", 999); // idx in 'good' fatjets global vector
        // W/Z AK4 jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vqqjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vqqjet_p4");
        // Hbb jet globals
        cutflow.globals.newVar<LorentzVector>("hbbfatjet_p4");
        cutflow.globals.newVar<unsigned int>("hbbfatjet_gidx", 999); // idx in 'good' fatjets global vector

        // Scale factors
        jes = nullptr;
        lep_sfs = nullptr;
        hlt_sfs = nullptr;
        btag_sfs = nullptr;
        pu_sfs = nullptr;
        puid_sfs = nullptr;
        all_corrections = false;
    };

    virtual void initBranches()
    {
        Core::Analysis::initBranches();
        // W/Z fat jet branches
        arbol.newBranch<double>("ld_vqqfatjet_xwqq", -999);
        arbol.newBranch<double>("ld_vqqfatjet_xvqq", -999);
        arbol.newBranch<double>("ld_vqqfatjet_pt", -999);
        arbol.newBranch<double>("ld_vqqfatjet_eta", -999);
        arbol.newBranch<double>("ld_vqqfatjet_phi", -999);
        arbol.newBranch<double>("ld_vqqfatjet_mass", -999);
        arbol.newBranch<double>("ld_vqqfatjet_msoftdrop", -999);
        arbol.newBranch<double>("tr_vqqfatjet_xwqq", -999);
        arbol.newBranch<double>("tr_vqqfatjet_xvqq", -999);
        arbol.newBranch<double>("tr_vqqfatjet_pt", -999);
        arbol.newBranch<double>("tr_vqqfatjet_eta", -999);
        arbol.newBranch<double>("tr_vqqfatjet_phi", -999);
        arbol.newBranch<double>("tr_vqqfatjet_mass", -999);
        arbol.newBranch<double>("tr_vqqfatjet_msoftdrop", -999);
        // W/Z AK4 jet branches
        arbol.newBranch<double>("ld_vqqjet_qgl", -999);
        arbol.newBranch<double>("ld_vqqjet_pt", -999);
        arbol.newBranch<double>("ld_vqqjet_eta", -999);
        arbol.newBranch<double>("ld_vqqjet_phi", -999);
        arbol.newBranch<double>("ld_vqqjet_mass", -999);
        arbol.newBranch<double>("tr_vqqjet_qgl", -999);
        arbol.newBranch<double>("tr_vqqjet_pt", -999);
        arbol.newBranch<double>("tr_vqqjet_eta", -999);
        arbol.newBranch<double>("tr_vqqjet_phi", -999);
        arbol.newBranch<double>("tr_vqqjet_mass", -999);
        arbol.newBranch<double>("vqqjets_Mjj", -999);
        arbol.newBranch<double>("vqqjets_dR", -999);
        // Hbb fat jet branches
        arbol.newBranch<double>("hbbfatjet_xbb", -999);
        arbol.newBranch<double>("hbbfatjet_pt", -999);
        arbol.newBranch<double>("hbbfatjet_eta", -999);
        arbol.newBranch<double>("hbbfatjet_phi", -999);
        arbol.newBranch<double>("hbbfatjet_mass", -999);
        arbol.newBranch<double>("hbbfatjet_msoftdrop", -999);
        // Other branches
        arbol.newBranch<double>("ST", -999);
        arbol.newBranch<bool>("passes_bveto", false);
        arbol.newBranch<bool>("is_allmerged", false);
        arbol.newBranch<bool>("is_semimerged", false);
        arbol.newBranch<double>("M_VVH", -999);
        arbol.newBranch<double>("VVH_pt", -999);
        arbol.newBranch<double>("VVH_eta", -999);
        arbol.newBranch<double>("VVH_phi", -999);
        arbol.newBranch<double>("alphaS_up", -999);
        arbol.newBranch<double>("alphaS_dn", -999);
        arbol.newBranch<double>("xbb_sf", 1.);
        arbol.newBranch<double>("xbb_sf_up", 1.);
        arbol.newBranch<double>("xbb_sf_dn", 1.);
    };

    virtual void initCorrections()
    {
        jes = new JetEnergyScales(cli.variation);
        // lep_sfs = new LeptonSFsPKU(PKU::IDtight);     // TODO: figure out how to apply these for a lep veto
        // hlt_sfs = new HLT1LepSFs();                   // TODO: add HT HLT sfs? If we are on the trigger plateau, not needed
        pu_sfs = new PileUpSFs();
        puid_sfs = new PileUpJetIDSFs();
        all_corrections = true;
    };

    virtual void initCutflow()
    {
        // Bookkeeping
        Cut* bookkeeping = new Core::Bookkeeping("Bookkeeping", *this, pu_sfs);
        cutflow.setRoot(bookkeeping);

        // Cut to make jet selection consistent in cutflow tables...
        Cut* jet_pt = new LambdaCut(
            "Geq2JetsPtGt30_FatJetHLTPlataeuCut",
            [&]()
            {
                int n_jets = 0;
                for (auto jet_pt : nt.Jet_pt())
                {
                    if (jet_pt > 30) { n_jets++; }
                }

                double max_fatjet_pt = 0;
                for (auto fatjet_pt : nt.FatJet_pt())
                {
                    if (fatjet_pt > max_fatjet_pt) { max_fatjet_pt = fatjet_pt; }
                }

                return (n_jets >= 2 && max_fatjet_pt > 550);
            }
        );
        cutflow.insert(bookkeeping, jet_pt, Right);

        // Save LHE mu_R and mu_F scale weights
        Cut* save_lhe = new Core::SaveSystWeights("SaveSystWeights", *this);
        cutflow.insert(jet_pt, save_lhe, Right);

        // Event filters
        Cut* event_filters = new VBSWH::PassesEventFilters("PassesEventFilters", *this);
        cutflow.insert(save_lhe, event_filters, Right);

        // HT triggers
        Cut* ht_triggers = new PassesTriggers("PassesTriggers", *this);
        cutflow.insert(event_filters, ht_triggers, Right);

        // Lepton selection
        Cut* select_leps = new Core::SelectLeptons("SelectLeptons", *this);
        cutflow.insert(ht_triggers, select_leps, Right);

        // Lepton veto
        Cut* no_leps = new LambdaCut(
            "NoLeptons", 
            [&]() 
            { 
                return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() == 0; 
            }
        );
        cutflow.insert(select_leps, no_leps, Right);

        // Fat jet selection
        Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", *this, jes);
        cutflow.insert(no_leps, select_fatjets, Right);

        Cut* trigger_plateau = new LambdaCut( // Delete if jet_pt Cut works
            "TriggerPlateauCuts",
            [&]()
            {
                LorentzVectors fatjet_p4s = cutflow.globals.getVal<LorentzVectors>("good_fatjet_p4s");
                double max_fatjet_pt = -999;
                for (auto fatjet_p4 : fatjet_p4s)
                {
                    if (fatjet_p4.pt() > max_fatjet_pt) { max_fatjet_pt = fatjet_p4.pt(); }
                }
                return max_fatjet_pt > 550;
            }
        );
        cutflow.insert(select_fatjets, trigger_plateau, Right); // Delete if jet_pt Cut works

        /* ------------------ 3 fatjet channel ------------------ */
        Cut* geq3_fatjets = new LambdaCut(
            "Geq3FatJets", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 3; }
        );
        // cutflow.insert(select_fatjets, geq3_fatjets, Right);
        cutflow.insert(trigger_plateau, geq3_fatjets, Right); // Delete if jet_pt Cut works

        // VVH fat jet candidate selection
        Cut* allmerged_select_vvh = new SelectVVHFatJets("AllMerged_SelectVVHFatJets", *this, AllMerged);
        cutflow.insert(geq3_fatjets, allmerged_select_vvh, Right);

        // Jet selection
        Cut* allmerged_select_jets = new SelectJetsNoFatJetOverlap("AllMerged_SelectJets", *this, AllMerged, jes, btag_sfs, puid_sfs);
        cutflow.insert(allmerged_select_vvh, allmerged_select_jets, Right);

        // VBS jet selection
        Cut* allmerged_select_vbsjets = new Core::SelectVBSJets("AllMerged_SelectVBSJets", *this);
        cutflow.insert(allmerged_select_jets, allmerged_select_vbsjets, Right);

        // Save analysis variables
        Cut* allmerged_save_vars = new SaveVariables("AllMerged_SaveVariables", *this, AllMerged);
        cutflow.insert(allmerged_select_vbsjets, allmerged_save_vars, Right);

        // Basic VBS jet requirements
        Cut* allmerged_Mjjgt500 = new LambdaCut(
            "AllMerged_MjjGt500", [&]() { return arbol.getLeaf<double>("M_jj") > 500; }
        );
        cutflow.insert(allmerged_save_vars, allmerged_Mjjgt500, Right);
        Cut* allmerged_detajjgt3 = new LambdaCut(
            "AllMerged_detajjGt3", [&]() { return fabs(arbol.getLeaf<double>("deta_jj")) > 3; }
        );
        cutflow.insert(allmerged_Mjjgt500, allmerged_detajjgt3, Right);
        
        // Preliminary cut tests
        Cut* allmerged_prelim_cut1 = new LambdaCut(
            "AllMerged_XbbGt0p9", [&]() { return arbol.getLeaf<double>("hbbfatjet_xbb") > 0.9; }
        );
        cutflow.insert(allmerged_detajjgt3, allmerged_prelim_cut1, Right);
        Cut* allmerged_prelim_cut2 = new LambdaCut(
            "AllMerged_XVqqGt0p9", 
            [&]() 
            { 
                return (
                    arbol.getLeaf<double>("ld_vqqfatjet_xwqq") > 0.9
                    && arbol.getLeaf<double>("tr_vqqfatjet_xwqq") > 0.9
                );
            }
        );
        cutflow.insert(allmerged_prelim_cut1, allmerged_prelim_cut2, Right);
        Cut* allmerged_prelim_cut3 = new LambdaCut(
            "AllMerged_STGt1300", [&]() { return arbol.getLeaf<double>("ST") > 1300; }
        );
        cutflow.insert(allmerged_prelim_cut2, allmerged_prelim_cut3, Right);
        Cut* allmerged_prelim_cut4 = new LambdaCut(
            "AllMerged_HbbMSDLt150", [&]() { return arbol.getLeaf<double>("hbbfatjet_msoftdrop") < 150; }
        );
        cutflow.insert(allmerged_prelim_cut3, allmerged_prelim_cut4, Right);
        Cut* allmerged_prelim_cut5 = new LambdaCut(
            "AllMerged_VqqMSDLt120", 
            [&]() 
            { 
                return (
                    arbol.getLeaf<double>("ld_vqqfatjet_msoftdrop") < 120
                    && arbol.getLeaf<double>("tr_vqqfatjet_msoftdrop") < 120
                );
            }
        );
        cutflow.insert(allmerged_prelim_cut4, allmerged_prelim_cut5, Right);
        /* ------------------------------------------------------ */

        /* ------------------ 2 fatjet channel ------------------ */
        Cut* exactly2_fatjets = new LambdaCut(
            "Exactly2FatJets", [&]() { return arbol.getLeaf<int>("n_fatjets") == 2; }
        );
        cutflow.insert(geq3_fatjets, exactly2_fatjets, Left);

        // VVH fat jet candidate selection
        Cut* semimerged_select_vvh = new SelectVVHFatJets("SemiMerged_SelectVVHFatJets", *this, SemiMerged);
        cutflow.insert(exactly2_fatjets, semimerged_select_vvh, Right);

        // Jet selection
        Cut* semimerged_select_jets = new SelectJetsNoFatJetOverlap("SemiMerged_SelectJets", *this, SemiMerged, jes, btag_sfs, puid_sfs);
        cutflow.insert(semimerged_select_vvh, semimerged_select_jets, Right);

        // N jets >= 4 (2 VBS + V --> qq)
        Cut* semimerged_geq4_jets = new LambdaCut(
            "SemiMerged_Geq4Jets", [&]() { return arbol.getLeaf<int>("n_jets") >= 4; }
        );
        cutflow.insert(semimerged_select_jets, semimerged_geq4_jets, Right);

        // V --> qq jet candidate selection
        Cut* semimerged_select_vjets = new SelectVJets("SemiMerged_SelectVJets", *this);
        cutflow.insert(semimerged_geq4_jets, semimerged_select_vjets, Right);

        // VBS jet selection
        Cut* semimerged_select_vbsjets = new Core::SelectVBSJets("SemiMerged_SelectVBSJets", *this);
        cutflow.insert(semimerged_select_vjets, semimerged_select_vbsjets, Right);

        // Save analysis variables
        Cut* semimerged_save_vars = new SaveVariables("SemiMerged_SaveVariables", *this, SemiMerged);
        cutflow.insert(semimerged_select_vbsjets, semimerged_save_vars, Right);

        // Basic VBS jet requirements
        Cut* semimerged_Mjjgt500 = new LambdaCut(
            "SemiMerged_MjjGt500", [&]() { return arbol.getLeaf<double>("M_jj") > 500; }
        );
        cutflow.insert(semimerged_save_vars, semimerged_Mjjgt500, Right);
        Cut* semimerged_detajjgt3 = new LambdaCut(
            "SemiMerged_detajjGt3", [&]() { return fabs(arbol.getLeaf<double>("deta_jj")) > 3; }
        );
        cutflow.insert(semimerged_Mjjgt500, semimerged_detajjgt3, Right);
        
        // Preliminary cut tests
        Cut* semimerged_prelim_cut1 = new LambdaCut(
            "SemiMerged_XbbGt0p9", [&]() { return arbol.getLeaf<double>("hbbfatjet_xbb") > 0.9; }
        );
        cutflow.insert(semimerged_detajjgt3, semimerged_prelim_cut1, Right);
        Cut* semimerged_prelim_cut2 = new LambdaCut(
            "SemiMerged_XVqqGt0p9", [&]() { return arbol.getLeaf<double>("ld_vqqfatjet_xwqq") > 0.9; }
        );
        cutflow.insert(semimerged_prelim_cut1, semimerged_prelim_cut2, Right);
        Cut* semimerged_prelim_cut3 = new LambdaCut(
            "SemiMerged_STGt1300", [&]() { return arbol.getLeaf<double>("ST") > 1300; }
        );
        cutflow.insert(semimerged_prelim_cut2, semimerged_prelim_cut3, Right);
        Cut* semimerged_prelim_cut4 = new LambdaCut(
            "SemiMerged_HbbMSDLt150", [&]() { return arbol.getLeaf<double>("hbbfatjet_msoftdrop") < 150; }
        );
        cutflow.insert(semimerged_prelim_cut3, semimerged_prelim_cut4, Right);
        Cut* semimerged_prelim_cut5 = new LambdaCut(
            "SemiMerged_VqqMSDLt120", [&]() { return arbol.getLeaf<double>("ld_vqqfatjet_msoftdrop") < 120; }
        );
        cutflow.insert(semimerged_prelim_cut4, semimerged_prelim_cut5, Right);
        Cut* semimerged_prelim_cut6 = new LambdaCut(
            "SemiMerged_VqqMjjLt120", [&]() { return arbol.getLeaf<double>("vqqjets_Mjj") < 120; }
        );
        cutflow.insert(semimerged_prelim_cut5, semimerged_prelim_cut6, Right);
        /* ------------------------------------------------------ */
    };

    virtual void init()
    {
        Core::Analysis::init();
        if (all_corrections)
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            jes->init();
            // lep_sfs->init(file_name);  // TODO: see Analysis::initCorrections
            // hlt_sfs->init(file_name);  // TODO: see Analysis::initCorrections
            pu_sfs->init(file_name);
            puid_sfs->init(file_name);
        }
    };
};

} // End namespace VBSVVHJets;

#endif
