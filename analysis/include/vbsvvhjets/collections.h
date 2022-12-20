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
// #include "vbsvvhjets/enums.h"
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
    bool all_corrections;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // W/Z fat jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vqqjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vqqjet_p4");
        // W/Z AK4 jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vqqjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vqqjet_p4");
        // Hbb jet globals
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");

        // Scale factors
        jes = nullptr;
        lep_sfs = nullptr;
        hlt_sfs = nullptr;
        btag_sfs = nullptr;
        pu_sfs = nullptr;
        all_corrections = false;
    };

    virtual void initBranches()
    {
        Core::Analysis::initBranches();
        // W/Z fat jet branches
        arbol.newBranch<double>("ld_vqqfatjet_score", -999);
        arbol.newBranch<double>("ld_vqqfatjet_pt", -999);
        arbol.newBranch<double>("ld_vqqfatjet_eta", -999);
        arbol.newBranch<double>("ld_vqqfatjet_phi", -999);
        arbol.newBranch<double>("ld_vqqfatjet_mass", -999);
        arbol.newBranch<double>("ld_vqqfatjet_msoftdrop", -999);
        arbol.newBranch<double>("tr_vqqfatjet_score", -999);
        arbol.newBranch<double>("tr_vqqfatjet_pt", -999);
        arbol.newBranch<double>("tr_vqqfatjet_eta", -999);
        arbol.newBranch<double>("tr_vqqfatjet_phi", -999);
        arbol.newBranch<double>("tr_vqqfatjet_mass", -999);
        arbol.newBranch<double>("tr_vqqfatjet_msoftdrop", -999);
        // W or Z AK4 jet branches
        arbol.newBranch<double>("ld_vqqjet_score", -999);
        arbol.newBranch<double>("ld_vqqjet_pt", -999);
        arbol.newBranch<double>("ld_vqqjet_eta", -999);
        arbol.newBranch<double>("ld_vqqjet_phi", -999);
        arbol.newBranch<double>("ld_vqqjet_mass", -999);
        arbol.newBranch<double>("ld_vqqjet_msoftdrop", -999);
        arbol.newBranch<double>("tr_vqqjet_score", -999);
        arbol.newBranch<double>("tr_vqqjet_pt", -999);
        arbol.newBranch<double>("tr_vqqjet_eta", -999);
        arbol.newBranch<double>("tr_vqqjet_phi", -999);
        arbol.newBranch<double>("tr_vqqjet_mass", -999);
        arbol.newBranch<double>("tr_vqqjet_msoftdrop", -999);
        // Hbb fat jet branches
        arbol.newBranch<double>("hbbfatjet_score", -999);
        arbol.newBranch<double>("hbbfatjet_pt", -999);
        arbol.newBranch<double>("hbbfatjet_eta", -999);
        arbol.newBranch<double>("hbbfatjet_phi", -999);
        arbol.newBranch<double>("hbbfatjet_mass", -999);
        arbol.newBranch<double>("hbbfatjet_msoftdrop", -999);
        // Other branches
        arbol.newBranch<double>("ST", -999);
        arbol.newBranch<bool>("passes_bveto", false);
    };

    virtual void initCorrections()
    {
        jes = new JetEnergyScales(cli.variation);
        // lep_sfs = new LeptonSFsPKU(PKU::IDtight);     // TODO: figure out how to apply these for a lep veto
        // hlt_sfs = new HLT1LepSFs();                   // TODO: add HT HLT sfs
        // btag_sfs = new BTagSFs(cli.output_name, "M"); // TODO: design new btageff study for this analysis
        pu_sfs = new PileUpSFs();
        all_corrections = true;
    };

    virtual void initCutflow()
    {
        // Bookkeeping
        Cut* bookkeeping = new Core::Bookkeeping("Bookkeeping", *this, pu_sfs);
        cutflow.setRoot(bookkeeping);

        // Save LHE mu_R and mu_F scale weights
        Cut* save_lhe = new Core::SaveSystWeights("SaveSystWeights", *this);
        cutflow.insert(bookkeeping, save_lhe, Right);

        // Event filters
        Cut* event_filters = new VBSWH::PassesEventFilters("PassesEventFilters", *this);
        cutflow.insert(save_lhe, event_filters, Right);

        // HT triggers
        Cut* ht_triggers = new PassesTriggers("PassesTriggers", *this);
        cutflow.insert(event_filters, ht_triggers, Right);

        // Lepton selection
        Cut* select_leps = new Core::SelectLeptonsPKU("SelectLeptons", *this);
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

        /* ------------------ 3 fatjet channel ------------------ */
        Cut* exactly3_fatjets = new LambdaCut(
            "Exactly3FatJets", [&]() { return arbol.getLeaf<int>("n_fatjets") == 3; }
        );
        cutflow.insert(select_fatjets, exactly3_fatjets, Right);

        // VVH fat jet candidate selection
        Cut* allmerged_select_vvh = new SelectVVHFatJets("AllMerged_SelectVVHFatJets", *this, AllMerged);
        cutflow.insert(select_fatjets, allmerged_select_vvh, Right);

        // Jet selection
        Cut* allmerged_select_jets = new SelectJetsNoFatJetOverlap("AllMerged_SelectJets", *this, AllMerged, jes, btag_sfs);
        cutflow.insert(allmerged_select_vvh, allmerged_select_jets, Right);

        // VBS jet selection
        Cut* allmerged_select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("AllMerged_SelectVBSJetsMaxE", *this);
        cutflow.insert(allmerged_select_jets, allmerged_select_vbsjets_maxE, Right);
        /* ------------------------------------------------------ */

        /* ------------------ 2 fatjet channel ------------------ */
        Cut* exactly2_fatjets = new LambdaCut(
            "Exactly2FatJets", [&]() { return arbol.getLeaf<int>("n_fatjets") == 2; }
        );
        cutflow.insert(exactly3_fatjets, exactly2_fatjets, Left);

        // VVH fat jet candidate selection
        Cut* semimerged_select_vvh = new SelectVVHFatJets("SemiMerged_SelectVVHFatJets", *this, SemiMerged);
        cutflow.insert(exactly2_fatjets, semimerged_select_vvh, Right);

        // Jet selection
        Cut* semimerged_select_jets = new SelectJetsNoFatJetOverlap("SemiMerged_SelectJets", *this, SemiMerged, jes, btag_sfs);
        cutflow.insert(semimerged_select_vvh, semimerged_select_jets, Right);

        // V-->qq jet candidate selection
        Cut* semimerged_select_vjets = new SelectVJets("SemiMerged_SelectVJets", *this);
        cutflow.insert(semimerged_select_jets, semimerged_select_vjets, Right);

        // VBS jet selection
        Cut* semimerged_select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SemiMerged_SelectVBSJetsMaxE", *this);
        cutflow.insert(semimerged_select_vjets, semimerged_select_vbsjets_maxE, Right);
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
            // btag_sfs->init(file_name); // TODO: see Analysis::initCorrections
            pu_sfs->init(file_name);
        }
    };
};

} // End namespace VBSVVHJets;

#endif
