#include "../core.h"
#include "../scalefactors.h"
#include "cuts.h"
// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "tqdm.h"

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli.input_tchain);

    // Initialize Arbol
    TFile* output_tfile = new TFile(
        TString(cli.output_dir+"/"+cli.output_name+".root"),
        "RECREATE"
    );
    Arbol arbol = Arbol(output_tfile);
    arbol.newBranch<int>("lep_pdgID", -999);
    arbol.newBranch<double>("lep_pt", -999);
    arbol.newBranch<double>("lep_eta", -999);
    arbol.newBranch<double>("lep_phi", -999);
    arbol.newBranch<double>("LT", -999);
    arbol.newBranch<double>("hbbjet_score", -999);
    arbol.newBranch<double>("hbbjet_pt", -999);
    arbol.newBranch<double>("hbbjet_eta", -999);
    arbol.newBranch<double>("hbbjet_phi", -999);
    arbol.newBranch<double>("hbbjet_mass", -999);
    arbol.newBranch<double>("hbbjet_msoftdrop", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVector>("lep_p4");
    cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
    cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s");
    cutflow.globals.newVar<LorentzVectors>("loose_lep_p4s");
    cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s");

    // Pack above into VBSWH struct (also adds branches)
    VBSWHAnalysis analysis = VBSWHAnalysis(arbol, nt, cli, cutflow);

    // Bookkeeping
    Cut* bookkeeping = new Bookkeeping("Bookkeeping", analysis);
    cutflow.setRoot(bookkeeping);

    // Exactly 1 lepton
    Cut* findleps_skim = new LambdaCut(
        "SKIM_FindLeptons", 
        [&]() 
        { 
            LorentzVectors veto_lep_p4s;
            LorentzVectors loose_lep_p4s;
            LorentzVectors tight_lep_p4s;
            for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
            {
                LorentzVector lep_p4 = nt.Electron_p4().at(elec_i);
                if (ttH::electronID(elec_i, ttH::IDveto, nt.year())) { veto_lep_p4s.push_back(lep_p4); }
                if (ttH::electronID(elec_i, ttH::IDfakable, nt.year())) { loose_lep_p4s.push_back(lep_p4); }
                if (ttH::electronID(elec_i, ttH::IDtight, nt.year())) { tight_lep_p4s.push_back(lep_p4); }
            }
            for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
            {
                LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
                if (ttH::muonID(muon_i, ttH::IDveto, nt.year())) { veto_lep_p4s.push_back(lep_p4); }
                if (ttH::muonID(muon_i, ttH::IDfakable, nt.year())) { loose_lep_p4s.push_back(lep_p4); }
                if (ttH::muonID(muon_i, ttH::IDtight, nt.year())) { tight_lep_p4s.push_back(lep_p4); }
            }
            cutflow.globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("loose_lep_p4s", loose_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("tight_lep_p4s", tight_lep_p4s);
            return true;
        }
    );
    cutflow.insert(bookkeeping->name, findleps_skim, Right);

    // Geq1VetoLep
    Cut* geq1vetolep_skim = new LambdaCut(
        "SKIM_Geq1VetoLep", [&]() { return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1; }
    );
    cutflow.insert(findleps_skim->name, geq1vetolep_skim, Right);

    // Geq2Jets
    Cut* geq2jets_skim = new LambdaCut(
        "SKIM_Geq2Jets", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s");
            int n_jets = 0;
            for (unsigned int jet_i = 0; jet_i < nt.nJet(); jet_i++)
            {
                LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, jet_p4) < 0.4)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap && nt.Jet_pt().at(jet_i) > 20)
                {
                    n_jets++;
                }
            }
            return (n_jets >= 2);
        }
    );
    cutflow.insert(geq1vetolep_skim->name, geq2jets_skim, Right);

    // Geq1FatJet
    Cut* geq1fatjet_skim = new LambdaCut(
        "SKIM_Geq1FatJetNoVetoLepOverlap", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s");
            int n_fatjets = 0;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap 
                    && nt.FatJet_mass().at(fatjet_i) > 10 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 10 
                    && nt.FatJet_pt().at(fatjet_i) > 200)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(geq2jets_skim->name, geq1fatjet_skim, Right);

    // Exactly1Lep
    Cut* exactly1tightlep_postskim = new LambdaCut(
        "POSTSKIM_Exactly1TightLep", 
        [&]() 
        { 
            int n_loose_leps = cutflow.globals.getVal<LorentzVectors>("loose_lep_p4s").size();
            int n_tight_leps = cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").size();
            return (n_loose_leps == 1 && n_tight_leps == 1);
        }
    );
    cutflow.insert(geq1fatjet_skim->name, exactly1tightlep_postskim, Right);

    // Geq1FatJet
    Cut* geq1fatjet_postskim = new LambdaCut(
        "POSTSKIM_Geq1FatJetNoTightLepOverlap", 
        [&]() 
        { 
            LorentzVector lep_p4 = cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").at(0);
            int n_fatjets = 0;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                bool is_overlap = (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8);
                if (!is_overlap 
                    && nt.FatJet_mass().at(fatjet_i) > 25 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 25 
                    && nt.FatJet_pt().at(fatjet_i) > 250 
                    && nt.FatJet_particleNet_HbbvsQCD().at(fatjet_i) > 0.5)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(exactly1tightlep_postskim->name, geq1fatjet_postskim, Right);

    // Lepton selection
    Cut* select_leps = new SelectLeptons("SelectLeptons", analysis);
    cutflow.insert(geq1fatjet_postskim->name, select_leps, Right);

    // == 1 lepton selection
    Cut* has_1lep = new Has1Lep("Has1TightLep", analysis);
    cutflow.insert(select_leps->name, has_1lep, Right);

    // Lepton has pT > 40
    Cut* lep_pt_gt40 = new LambdaCut(
        "LepPtGt40", [&]() { return arbol.getLeaf<double>("lep_pt") >= 40; }
    );
    cutflow.insert(has_1lep->name, lep_pt_gt40, Right);

    // Single-lepton triggers
    Cut* lep_triggers = new Passes1LepTriggers("Passes1LepTriggers", analysis);
    cutflow.insert(lep_pt_gt40->name, lep_triggers, Right);

    // Fat jet selection
    Cut* select_fatjets = new SelectFatJets("SelectFatJets", analysis);
    cutflow.insert(lep_triggers->name, select_fatjets, Right);

    // Geq1FatJet
    Cut* geq1fatjet = new LambdaCut(
        "Geq1FatJet", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 1; }
    );
    cutflow.insert(select_fatjets->name, geq1fatjet, Right);

    // Hbb selection
    Cut* select_hbbjet = new SelectHbbFatJet("SelectHbbFatJet", analysis);
    cutflow.insert(geq1fatjet->name, select_hbbjet, Right);

    // Hbb score > 0.9
    Cut* hbbjet_score_gt0p9 = new LambdaCut(
        "PNetHbbScoreGt0p9", [&]() { return arbol.getLeaf<double>("hbbjet_score") > 0.9; }
    );
    cutflow.insert(select_hbbjet->name, hbbjet_score_gt0p9, Right);

    // Jet selection
    Cut* select_jets = new SelectJetsNoHbbOverlap("SelectJetsNoHbbOverlap", analysis);
    cutflow.insert(hbbjet_score_gt0p9->name, select_jets, Right);

    // Global AK4 b-veto
    Cut* ak4bveto = new LambdaCut(
        "Ak4GlobalBVeto", 
        [&]()
        {
            for (auto& btag : cutflow.globals.getVal<Doubles>("good_jet_btags"))
            {
                if (btag > gconf.WP_DeepFlav_medium) { return false; }
            }
            return true;
        }
    );
    cutflow.insert(select_jets->name, ak4bveto, Right);

    // VBS jet selection
    Cut* select_vbsjets_maxE = new SelectVBSJetsMaxE("SelectVBSJetsMaxE", analysis);
    cutflow.insert(ak4bveto->name, select_vbsjets_maxE, Right);

    // Basic VBS jet requirements
    Cut* vbsjets_presel = new VBSPresel("MjjGt500_detajjGt3", analysis);
    cutflow.insert(select_vbsjets_maxE->name, vbsjets_presel, Right);

    /* Splits trigger by lepton flavor to check eff
    // Single-electron channel
    Cut* is_elec = new LambdaCut(
        "IsElectron", [&]() { return abs(arbol.getLeaf<int>("lep_pdgID")) == 11; }
    );
    cutflow.insert(vbsjets_presel->name, is_elec, Right);

    // Single-electron triggers
    Cut* elec_triggers = new Passes1LepTriggers("Passes1ElecTriggers", analysis);
    cutflow.insert(is_elec->name, elec_triggers, Right);

    // Single-muon channel
    Cut* is_muon = new LambdaCut(
        "IsMuon", [&]() { return abs(arbol.getLeaf<int>("lep_pdgID")) == 13; }
    );
    cutflow.insert(is_elec->name, is_muon, Left);

    // Single-muon triggers
    Cut* muon_triggers = new Passes1LepTriggers("Passes1MuonTriggers", analysis);
    cutflow.insert(is_muon->name, muon_triggers, Right);
    */

    // BDT Preselection
    Cut* bdt_presel = new LambdaCut(
        "MjjGt1500_detajjGt5_STGt500", 
        [&]() 
        { 
            double M_jj = arbol.getLeaf<double>("M_jj");
            double deta_jj = arbol.getLeaf<double>("deta_jj");
            double ST = arbol.getLeaf<double>("ST");
            return (M_jj > 1500 && fabs(deta_jj) > 5 && ST > 500); 
        }
    );
    cutflow.insert(vbsjets_presel->name, bdt_presel, Right);

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // Reset branches and globals
                arbol.resetBranches();
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.runUntil(vbsjets_presel->name);
                if (passed) { arbol.fillTTree(); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data)
    {
        cutflow.print();
        cutflow.write(cli.output_dir);
    }
    arbol.writeTFile();
    return 0;
}
