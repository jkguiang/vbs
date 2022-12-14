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
    bool all_corrections;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // W jet(s) globals
        cutflow.globals.newVar<LorentzVector>("wqqjet_p4");
        cutflow.globals.newVar<LorentzVector>("ld_wjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_wjet_p4");
        // Z jet(s) globals
        cutflow.globals.newVar<LorentzVector>("zqqjet_p4");
        cutflow.globals.newVar<LorentzVector>("ld_zjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_zjet_p4");
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
        // W jet(s) branches
        arbol.newBranch<double>("wqqjet_score", -999);
        arbol.newBranch<double>("wqqjet_pt", -999);
        arbol.newBranch<double>("wqqjet_eta", -999);
        arbol.newBranch<double>("wqqjet_phi", -999);
        arbol.newBranch<double>("wqqjet_mass", -999);
        arbol.newBranch<double>("wqqjet_msoftdrop", -999);
        arbol.newBranch<double>("ld_wjet_pt", -999);
        arbol.newBranch<double>("ld_wjet_eta", -999);
        arbol.newBranch<double>("ld_wjet_phi", -999);
        arbol.newBranch<double>("tr_wjet_pt", -999);
        arbol.newBranch<double>("tr_wjet_eta", -999);
        arbol.newBranch<double>("tr_wjet_phi", -999);
        // Z jet(s) branches
        arbol.newBranch<double>("zqqjet_score", -999);
        arbol.newBranch<double>("zqqjet_pt", -999);
        arbol.newBranch<double>("zqqjet_eta", -999);
        arbol.newBranch<double>("zqqjet_phi", -999);
        arbol.newBranch<double>("zqqjet_mass", -999);
        arbol.newBranch<double>("zqqjet_msoftdrop", -999);
        arbol.newBranch<double>("ld_zjet_pt", -999);
        arbol.newBranch<double>("ld_zjet_eta", -999);
        arbol.newBranch<double>("ld_zjet_phi", -999);
        arbol.newBranch<double>("tr_zjet_pt", -999);
        arbol.newBranch<double>("tr_zjet_eta", -999);
        arbol.newBranch<double>("tr_zjet_phi", -999);
        // Hbb jet branches
        arbol.newBranch<double>("hbbjet_score", -999);
        arbol.newBranch<double>("hbbjet_pt", -999);
        arbol.newBranch<double>("hbbjet_eta", -999);
        arbol.newBranch<double>("hbbjet_phi", -999);
        arbol.newBranch<double>("hbbjet_mass", -999);
        arbol.newBranch<double>("hbbjet_msoftdrop", -999);
        // Other branches
        arbol.newBranch<double>("ST", -999);
        arbol.newBranch<bool>("passes_bveto", false);
    };

    virtual void initCorrections()
    {
        jes = new JetEnergyScales(cli.variation);
        lep_sfs = new LeptonSFsPKU(PKU::IDtight);
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

        // Event filters
        Cut* event_filters = new VBSWH::PassesEventFilters("PassesEventFilters", *this);
        cutflow.insert(bookkeeping, event_filters, Right);

        // HT triggers
        Cut* ht_triggers = new PassesTriggers("PassesTriggers", *this, hlt_sfs);
        cutflow.insert(event_filters, ht_triggers, Right);

        // Lepton selection
        Cut* select_leps = new Core::SelectLeptonsPKU("SelectLeptons", *this);
        cutflow.insert(ht_triggers, select_leps, Right);

        // Lepton veto
        Cut* no_leps = new LambdaCut(
            "NoLeptons", 
            [&]() { cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() == 0; }
        );
        cutflow.insert(select_leps, no_leps, Right);

        // Fat jet selection
        Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", *this, jes);
        cutflow.insert(no_leps, select_fatjets, Right);

        // VVH fat jet candidate selection
        Cut* select_vvh = new SelectVVHFatJets("SelectVVHFatJets", *this);
        cutflow.insert(select_fatjets, select_vvh, Right);

        // Jet selection
        Cut* select_jets = new SelectJetsNoFatJetOverlap("SelectJetsNoFatJetOverlap", *this, jes, btag_sfs);
        cutflow.insert(select_vvh, select_jets, Right);

        // VBS jet selection
        Cut* select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SelectVBSJetsMaxE", *this);
        cutflow.insert(select_jets, select_vbsjets_maxE, Right);

        // Save LHE mu_R and mu_F scale weights
        Cut* save_lhe = new Core::SaveSystWeights("SaveSystWeights", *this);
        cutflow.insert(select_vbsjets_maxE, save_lhe, Right);
    };

    virtual void init()
    {
        Core::Analysis::init();
        if (all_corrections)
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            jes->init();
            lep_sfs->init(file_name);
            // hlt_sfs->init(file_name);  // TODO: see Analysis::initCorrections
            // btag_sfs->init(file_name); // TODO: see Analysis::initCorrections
            pu_sfs->init(file_name);
        }
    };
};

} // End namespace VBSVVHJets;

#endif
