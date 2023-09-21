#ifndef CORE_COLLECTIONS_H
#define CORE_COLLECTIONS_H

// RAPIDO
#include "arbol.h"
#include "arbusto.h"
#include "cutflow.h"
#include "hepcli.h"
// ROOT
#include "TString.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"             // gconf
#include "Tools/goodrun.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<double> Doubles;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

namespace Core
{

struct Skimmer
{
    Arbusto& arbusto;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;
    TList* runs;
    TList* lumis;

    Skimmer(Arbusto& arbusto_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbusto(arbusto_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;
        runs = new TList();
        lumis = new TList();
    };

    virtual void init(TTree* ttree)
    {
        arbusto.init(ttree);
        gconf.GetConfigs(nt.year());

        TString file_name = ttree->GetCurrentFile()->GetName();
        gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("NanoAODAPV") || file_name.Contains("UL16APV"));

        // Store metadata ttrees
        TTree* runtree = ((TTree*)ttree->GetCurrentFile()->Get("Runs"))->CloneTree();
        runtree->SetDirectory(0);
        runs->Add(runtree);
        TTree* lumitree = ((TTree*)ttree->GetCurrentFile()->Get("LuminosityBlocks"))->CloneTree();
        lumitree->SetDirectory(0);
        lumis->Add(lumitree);
    };

    virtual void write()
    {
        TTree* merged_runs = TTree::MergeTrees(runs);
        merged_runs->SetName("Runs");
        TTree* merged_lumis = TTree::MergeTrees(lumis);
        merged_lumis->SetName("LuminosityBlocks");

        arbusto.tfile->cd();
        merged_runs->Write();
        merged_lumis->Write();
        arbusto.write();
    };
};

struct Analysis
{
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbol(arbol_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        // Lepton globals
        cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
        cutflow.globals.newVar<Integers>("veto_lep_pdgIDs", {});
        cutflow.globals.newVar<Integers>("veto_lep_idxs", {});
        cutflow.globals.newVar<Integers>("veto_lep_jet_idxs", {});
        // Jet globals
        cutflow.globals.newVar<LorentzVectors>("good_jet_p4s", {});
        cutflow.globals.newVar<Integers>("good_jet_idxs", {});
        // Fat jet (AK8) globals
        cutflow.globals.newVar<LorentzVectors>("good_fatjet_p4s", {});
        cutflow.globals.newVar<Integers>("good_fatjet_idxs", {});
        cutflow.globals.newVar<Doubles>("good_fatjet_wqqtags", {});    // ParticleNet tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_zqqtags", {});    // ParticleNet tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_hbbtags", {});    // ParticleNet tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_xbbtags", {});    // ParticleNet mass-decorrelated tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_xqqtags", {});    // ParticleNet mass-decorrelated tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_xcctags", {});    // ParticleNet mass-decorrelated tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_xwqqtags", {});   // ParticleNet mass-decorrelated W-like tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_xvqqtags", {});   // ParticleNet mass-decorrelated W/Z-like tagger
        cutflow.globals.newVar<Doubles>("good_fatjet_masses", {});     // ParticleNet regressed mass
        cutflow.globals.newVar<Doubles>("good_fatjet_msoftdrops", {});
        // VBS jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vbsjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vbsjet_p4");
        cutflow.globals.newVar<int>("ld_vbsjet_idx");
        cutflow.globals.newVar<int>("tr_vbsjet_idx");
    };

    virtual void initBranches()
    {
        // Jet (AK4) branches
        arbol.newBranch<double>("HT", -999);
        arbol.newBranch<int>("n_jets", -999);
        arbol.newBranch<int>("n_loose_b_jets", -999);
        arbol.newBranch<int>("n_medium_b_jets", -999);
        arbol.newBranch<int>("n_tight_b_jets", -999);
        arbol.newBranch<double>("light_btag_sf", -999);
        arbol.newBranch<double>("light_btag_sf_up", -999);
        arbol.newBranch<double>("light_btag_sf_dn", -999);
        arbol.newBranch<double>("bc_btag_sf", -999);
        arbol.newBranch<double>("bc_btag_sf_up", -999);
        arbol.newBranch<double>("bc_btag_sf_dn", -999);
        arbol.newBranch<double>("puid_sf", -999);
        arbol.newBranch<double>("puid_sf_up", -999);
        arbol.newBranch<double>("puid_sf_dn", -999);
        // Jet (AK8) branches
        arbol.newBranch<double>("HT_fat", -999);
        arbol.newBranch<int>("n_fatjets", -999);
        // VBS jet branches
        arbol.newBranch<double>("ld_vbsjet_pt", -999);
        arbol.newBranch<double>("ld_vbsjet_eta", -999);
        arbol.newBranch<double>("ld_vbsjet_phi", -999);
        arbol.newBranch<double>("tr_vbsjet_pt", -999);
        arbol.newBranch<double>("tr_vbsjet_eta", -999);
        arbol.newBranch<double>("tr_vbsjet_phi", -999);
        arbol.newBranch<double>("M_jj", -999);
        arbol.newBranch<double>("pt_jj", -999);
        arbol.newBranch<double>("eta_jj", -999);
        arbol.newBranch<double>("phi_jj", -999);
        arbol.newBranch<double>("deta_jj", -999);
        arbol.newBranch<double>("abs_deta_jj", 999);
        arbol.newBranch<double>("dR_jj", -999);
        // Other branches
        arbol.newBranch<double>("xsec_sf", -999);
        arbol.newBranch<double>("pu_sf", -999);
        arbol.newBranch<double>("pu_sf_up", -999);
        arbol.newBranch<double>("pu_sf_dn", -999);
        arbol.newBranch<double>("prefire_sf", -999);
        arbol.newBranch<double>("prefire_sf_up", -999);
        arbol.newBranch<double>("prefire_sf_dn", -999);
        arbol.newBranch<double>("trig_sf", -999);
        arbol.newBranch<double>("trig_sf_up", -999);
        arbol.newBranch<double>("trig_sf_dn", -999);
        arbol.newBranch<int>("event", -999);
        arbol.newBranch<int>("year", -999);
        arbol.newBranch<double>("MET", -999);
        arbol.newBranch<double>("MET_up", -999);
        arbol.newBranch<double>("MET_dn", -999);
        arbol.newBranch<float>("lhe_muF0p5_muR0p5", -999);
        arbol.newBranch<float>("lhe_muF1p0_muR0p5", -999);
        arbol.newBranch<float>("lhe_muF2p0_muR0p5", -999);
        arbol.newBranch<float>("lhe_muF0p5_muR1p0", -999);
        arbol.newBranch<float>("lhe_muF1p0_muR1p0", -999);
        arbol.newBranch<float>("lhe_muF2p0_muR1p0", -999);
        arbol.newBranch<float>("lhe_muF0p5_muR2p0", -999);
        arbol.newBranch<float>("lhe_muF1p0_muR2p0", -999);
        arbol.newBranch<float>("lhe_muF2p0_muR2p0", -999);
        arbol.newBranch<float>("ps_isr2p0_fsr1p0", -999);
        arbol.newBranch<float>("ps_isr1p0_fsr2p0", -999);
        arbol.newBranch<float>("ps_isr0p5_fsr1p0", -999);
        arbol.newBranch<float>("ps_isr1p0_fsr0p5", -999);
    };

    virtual void init()
    {
        // Global config
        gconf.nanoAOD_ver = 9;
        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        gconf.isAPV = (
            file_name.Contains("HIPM_UL2016") 
            || file_name.Contains("NanoAODAPV") 
            || file_name.Contains("UL16APV")
        );
        gconf.GetConfigs(nt.year());

        // Golden JSON
        if (nt.isData())
        {
            switch (nt.year())
            {
            case 2016:
                set_goodrun_file(
                    "data/golden_jsons/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_formatted.txt"
                );
                break;
            case 2017:
                set_goodrun_file(
                    "data/golden_jsons/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_formatted.txt"
                );
                break;
            case 2018:
                set_goodrun_file(
                    "data/golden_jsons/Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON_formatted.txt"
                );
                break;
            default:
                throw std::runtime_error("Core::Analysis - invalid year or none set");
                break;
            }
        }
    };
};

}; // End namespace Core

#endif
