#ifndef VBSWH_COLLECTIONS_H
#define VBSWH_COLLECTIONS_H

// RAPIDO
#include "arbol.h"
#include "cutflow.h"
#include "utilities.h"
// VBS
#include "core/collections.h"
#include "core/cuts.h"
#include "core/pku.h"
#include "corrections/all.h"
#include "vbswh/cuts.h"

namespace VBSWH
{

struct Skimmer : Core::Skimmer
{
    Skimmer(Arbusto& arbusto_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Skimmer(arbusto_ref, nt_ref, cli_ref, cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;

        cutflow.globals.newVar<LorentzVector>("lep_p4");
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
        cutflow.globals.newVar<double>("ST", -999);
        cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
        cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s", {});
        cutflow.globals.newVar<Integers>("veto_lep_pdgIDs", {});
        cutflow.globals.newVar<Integers>("tight_lep_pdgIDs", {});
    };

    virtual void initCutflow()
    {
        Cut* base = new LambdaCut("Base", [&]() { return true; });
        cutflow.setRoot(base);

        Cut* find_leps = new FindLeptons("FindLeptons", *this);
        cutflow.insert(base, find_leps, Right);

        Cut* geq1_veto_lep = new LambdaCut(
            "Geq1VetoLep", 
            [&]()
            {
                return (cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1);
            }
        );
        cutflow.insert(find_leps, geq1_veto_lep, Right);

        Cut* geq2_jets = new Geq2Jets("Geq2Jets", *this);
        cutflow.insert(geq1_veto_lep, geq2_jets, Right);

        Cut* geq1_fatjet_loose = new Geq1FatJetLoose("Geq1FatJetLoose", *this);
        cutflow.insert(geq2_jets, geq1_fatjet_loose, Right);

        Cut* exactly1_lep = new Exactly1Lepton("Exactly1Lepton", *this);
        cutflow.insert(geq1_fatjet_loose, exactly1_lep, Right);

        Cut* geq1_fatjet_tight = new Geq1FatJetTight("Geq1FatJetTight", *this);
        cutflow.insert(exactly1_lep, geq1_fatjet_tight, Right);

        Cut* STgt800 = new LambdaCut(
            "STgt800", 
            [&]() { return (cutflow.globals.getVal<double>("ST") > 800); }
        );
        cutflow.insert(geq1_fatjet_tight, STgt800, Right);
    };
};

struct SkimmerPKU : Skimmer
{
    SkimmerPKU(Arbusto& arbusto_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Skimmer(arbusto_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Do nothing
    };

    void initCutflow()
    {
        Skimmer::initCutflow();
        
        Cut* find_leps = new FindLeptonsPKU("FindLeptonsPKU", *this);
        cutflow.replace("FindLeptons", find_leps);
    };
};

struct Analysis : Core::Analysis
{
    JetEnergyScales* jes;
    LeptonSFsPKU* lep_sfs;
    HLT1LepSFs* hlt_sfs;
    BTagSFs* btag_sfs;
    PileUpSFs* pu_sfs;
    PileUpJetIDSFs* puid_sfs;
    VBSWHXbbSFs* xbb_sfs;
    bool all_corrections;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Lepton globals
        cutflow.globals.newVar<LorentzVector>("lep_p4");
        // Hbb jet globals
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
        // Scale factors
        jes = nullptr;
        lep_sfs = nullptr;
        hlt_sfs = nullptr;
        btag_sfs = nullptr;
        pu_sfs = nullptr;
        puid_sfs = nullptr;
        xbb_sfs = nullptr;
        all_corrections = false;
    };

    virtual void initBranches()
    {
        Core::Analysis::initBranches();
        // Lepton branches
        arbol.newBranch<double>("lep_id_sf", -999);
        arbol.newBranch<double>("lep_id_sf_up", -999);
        arbol.newBranch<double>("lep_id_sf_dn", -999);
        arbol.newBranch<double>("elec_reco_sf", -999);
        arbol.newBranch<double>("elec_reco_sf_up", -999);
        arbol.newBranch<double>("elec_reco_sf_dn", -999);
        arbol.newBranch<double>("muon_iso_sf", -999);
        arbol.newBranch<double>("muon_iso_sf_up", -999);
        arbol.newBranch<double>("muon_iso_sf_dn", -999);
        arbol.newBranch<double>("elec_id_sf", -999);
        arbol.newBranch<double>("elec_id_sf_up", -999);
        arbol.newBranch<double>("elec_id_sf_dn", -999);
        arbol.newBranch<double>("muon_id_sf", -999);
        arbol.newBranch<double>("muon_id_sf_up", -999);
        arbol.newBranch<double>("muon_id_sf_dn", -999);
        arbol.newBranch<double>("trig_elec_sf", -999);
        arbol.newBranch<double>("trig_elec_sf_up", -999);
        arbol.newBranch<double>("trig_elec_sf_dn", -999);
        arbol.newBranch<double>("trig_muon_sf", -999);
        arbol.newBranch<double>("trig_muon_sf_up", -999);
        arbol.newBranch<double>("trig_muon_sf_dn", -999);
        arbol.newBranch<int>("lep_pdgID", -999);
        arbol.newBranch<double>("lep_pt", -999);
        arbol.newBranch<double>("lep_eta", -999);
        arbol.newBranch<double>("lep_phi", -999);
        arbol.newBranch<double>("LT", -999);
        arbol.newBranch<double>("LT_up", -999);
        arbol.newBranch<double>("LT_dn", -999);
        // Hbb jet branches
        arbol.newBranch<int>("n_hbbjet_genbquarks", -999);
        arbol.newBranch<double>("hbbjet_score", -999);
        arbol.newBranch<double>("hbbjet_pt", -999);
        arbol.newBranch<double>("hbbjet_eta", -999);
        arbol.newBranch<double>("hbbjet_phi", -999);
        arbol.newBranch<double>("hbbjet_mass", -999);
        arbol.newBranch<double>("hbbjet_msoftdrop", -999);
        // Other branches
        arbol.newBranch<double>("ST", -999);
        arbol.newBranch<double>("ST_up", -999);
        arbol.newBranch<double>("ST_dn", -999);
        arbol.newBranch<bool>("passes_bveto", false);
        arbol.newBranch<double>("alphaS_up", -999);
        arbol.newBranch<double>("alphaS_dn", -999);
        arbol.newBranch<Doubles>("reweights", {});
        arbol.newBranch<double>("xbb_sf", 1.);
        arbol.newBranch<double>("xbb_sf_up", 1.);
        arbol.newBranch<double>("xbb_sf_dn", 1.);
        arbol.newBranch<double>("ewkfix_sf", -999);
    };

    virtual void initCorrections()
    {
        jes = new JetEnergyScales(cli.variation);
        lep_sfs = new LeptonSFsPKU(PKU::IDtight);
        hlt_sfs = new HLT1LepSFs();
        btag_sfs = new BTagSFs(cli.output_name, "M");
        pu_sfs = new PileUpSFs();
        puid_sfs = new PileUpJetIDSFs();
        xbb_sfs = new VBSWHXbbSFs();
        all_corrections = true;
    };

    virtual void initCutflow()
    {
        // Bookkeeping
        Cut* bookkeeping = new Core::Bookkeeping("Bookkeeping", *this, pu_sfs);
        cutflow.setRoot(bookkeeping);

        // Event filters
        Cut* event_filters = new PassesEventFilters("PassesEventFilters", *this);
        cutflow.insert(bookkeeping, event_filters, Right);

        // Lepton selection
        Cut* select_leps = new Core::SelectLeptonsPKU("SelectLeptons", *this);
        cutflow.insert(event_filters, select_leps, Right);

        // == 1 lepton selection
        Cut* has_1lep = new Has1LepPKU("Has1TightLep", *this, lep_sfs);
        cutflow.insert(select_leps, has_1lep, Right);

        // Lepton has pT > 40
        Cut* lep_pt_gt40 = new LambdaCut(
            "LepPtGt40", [&]() { return arbol.getLeaf<double>("lep_pt") >= 40.; }
        );
        cutflow.insert(has_1lep, lep_pt_gt40, Right);

        // Fat jet selection
        Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", *this, jes);
        cutflow.insert(lep_pt_gt40, select_fatjets, Right);

        // Geq1FatJet
        Cut* geq1fatjet = new LambdaCut(
            "Geq1FatJet", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 1; }
        );
        cutflow.insert(select_fatjets, geq1fatjet, Right);

        // Hbb selection
        Cut* select_hbbjet = new SelectHbbFatJet("SelectHbbFatJet", *this, true);
        cutflow.insert(geq1fatjet, select_hbbjet, Right);

        // Jet selection
        Cut* select_jets = new SelectJetsNoHbbOverlap("SelectJetsNoHbbOverlap", *this, jes, btag_sfs, puid_sfs);
        cutflow.insert(select_hbbjet, select_jets, Right);

        // VBS jet selection
        Cut* select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SelectVBSJetsMaxE", *this);
        cutflow.insert(select_jets, select_vbsjets_maxE, Right);
        // Cut* select_vbsjets = new Core::SelectVBSJets("SelectVBSJets", *this);
        // cutflow.insert(select_jets, select_vbsjets, Right);

        // Save LHE mu_R and mu_F scale weights
        Cut* save_lhe = new Core::SaveSystWeights("SaveSystWeights", *this);
        cutflow.insert(select_vbsjets_maxE, save_lhe, Right);
        // cutflow.insert(select_vbsjets, save_lhe, Right);

        // Save analysis variables
        Cut* save_vars = new SaveVariables("SaveVariables", *this, xbb_sfs);
        cutflow.insert(save_lhe, save_vars, Right);

        // Single-lepton triggers
        Cut* lep_triggers = new Passes1LepTriggers("Passes1LepTriggers", *this, hlt_sfs);
        cutflow.insert(save_vars, lep_triggers, Right);

        // Basic VBS jet requirements
        Cut* vbsjets_presel = new LambdaCut(
            "MjjGt500", [&]() { return arbol.getLeaf<double>("M_jj") > 500; }
        );
        cutflow.insert(lep_triggers, vbsjets_presel, Right);

        Cut* xbb_presel = new LambdaCut(
            "XbbGt0p3", [&]() { return arbol.getLeaf<double>("hbbjet_score") > 0.3; }
        );
        cutflow.insert(vbsjets_presel, xbb_presel, Right);

        Cut* apply_ak4bveto = new LambdaCut(
            "ApplyAk4GlobalBVeto", [&]() { return arbol.getLeaf<bool>("passes_bveto"); }
        );
        cutflow.insert(xbb_presel, apply_ak4bveto, Right);
        
        Cut* SR1_vbs_cuts = new LambdaCut(
            "MjjGt600_detajjGt4", 
            [&]() 
            { 
                return (
                    arbol.getLeaf<double>("M_jj") > 600 
                    && fabs(arbol.getLeaf<double>("deta_jj")) > 4
                );
            }
        );
        cutflow.insert(apply_ak4bveto, SR1_vbs_cuts, Right);

        Cut* SR1_ST_cut = new LambdaCut(
            "STGt900", [&]() { return arbol.getLeaf<double>("ST") > 900; }
        );
        cutflow.insert(SR1_vbs_cuts, SR1_ST_cut, Right);

        Cut* SR1_hbb_cut = new LambdaCut(
            "XbbGt0p9_MSDLt150", 
            [&]() 
            { 
                return (
                    arbol.getLeaf<double>("hbbjet_score") > 0.9
                    && arbol.getLeaf<double>("hbbjet_msoftdrop") < 150
                );
            },
            [&]()
            {
                return arbol.getLeaf<double>("xbb_sf");
            }
        );
        cutflow.insert(SR1_ST_cut, SR1_hbb_cut, Right);

        Cut* SR2 = new LambdaCut(
            "STGt1500", [&]() { return arbol.getLeaf<double>("ST") > 1500; }
        );
        cutflow.insert(SR1_hbb_cut, SR2, Right);
    };

    virtual void init()
    {
        Core::Analysis::init();
        if (all_corrections)
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            jes->init(file_name);
            lep_sfs->init(file_name);
            hlt_sfs->init(file_name);
            btag_sfs->init(file_name);
            pu_sfs->init(file_name);
            puid_sfs->init(file_name);
            xbb_sfs->init(file_name);
        }
    };
};

} // End namespace VBSWH;

#endif
