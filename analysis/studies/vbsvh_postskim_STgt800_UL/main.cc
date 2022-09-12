// RAPIDO
#include "arbusto.h"
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
#include "tqdm.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<double> Doubles;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

int main(int argc, char** argv) 
{
    gconf.nanoAOD_ver = 9;

    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);
    // Initialize Arbusto
    Arbusto arbusto = Arbusto(
        cli,
        {
            "Electron*", "nElectron",
            "Muon*", "nMuon",
            "Tau*", "nTau", 
            "Jet*", "nJet", 
            "FatJet*", "nFatJet", 
            "GenPart*", "nGenPart",
            "Generator*",
            "MET*",
            "event*",
            "run*",
            "luminosityBlock*",
            "genWeight*",
            "btagWeight*",
            "LHE*",
            "*Weight*",
            "Flag*",
            "SubJet*",
            "HLT_*",
            "Pileup*"
        }
    );
    TList* runs = new TList();
    TList* lumis = new TList();

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVector>("lep_p4");
    cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
    cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("loose_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s", {});

    // Bookkeeping
    Cut* base = new LambdaCut("Base", [&]() { return true; });
    cutflow.setRoot(base);

    // Find leptons
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
                if (ttH_UL::electronID(elec_i, ttH::IDveto, nt.year())) { veto_lep_p4s.push_back(lep_p4); }
                if (ttH_UL::electronID(elec_i, ttH::IDfakable, nt.year())) { loose_lep_p4s.push_back(lep_p4); }
                if (ttH_UL::electronID(elec_i, ttH::IDtight, nt.year())) { tight_lep_p4s.push_back(lep_p4); }
            }
            for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
            {
                LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
                if (ttH_UL::muonID(muon_i, ttH::IDveto, nt.year())) { veto_lep_p4s.push_back(lep_p4); }
                if (ttH_UL::muonID(muon_i, ttH::IDfakable, nt.year())) { loose_lep_p4s.push_back(lep_p4); }
                if (ttH_UL::muonID(muon_i, ttH::IDtight, nt.year())) { tight_lep_p4s.push_back(lep_p4); }
            }
            cutflow.globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("loose_lep_p4s", loose_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("tight_lep_p4s", tight_lep_p4s);
            return true;
        }
    );
    cutflow.insert(base, findleps_skim, Right);

    // Geq1VetoLep
    Cut* geq1vetolep_skim = new LambdaCut(
        "SKIM_Geq1VetoLep", [&]() { return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1; }
    );
    cutflow.insert(findleps_skim, geq1vetolep_skim, Right);

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
    cutflow.insert(geq1vetolep_skim, geq2jets_skim, Right);

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
    cutflow.insert(geq2jets_skim, geq1fatjet_skim, Right);

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
    cutflow.insert(geq1fatjet_skim, exactly1tightlep_postskim, Right);

    // Geq1FatJet
    Cut* geq1fatjet_postskim = new LambdaCut(
        "POSTSKIM_Geq1FatJetNoTightLepOverlap", 
        [&]() 
        { 
            LorentzVector lep_p4 = cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").at(0);
            int n_fatjets = 0;
            double hbbjet_score = -999.;
            LorentzVector hbbjet_p4;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                // Basic requirements
                if (nt.FatJet_pt().at(fatjet_i) <= 250) { continue; }
                if (nt.FatJet_mass().at(fatjet_i) <= 50) { continue; }
                if (nt.FatJet_msoftdrop().at(fatjet_i) <= 40) { continue; }
                // Remove lepton overlap
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8) { continue; }

                // Count good fat jets
                n_fatjets++;
                double pnet_xbb = nt.FatJet_particleNetMD_Xbb().at(fatjet_i);
                double pnet_qcd = nt.FatJet_particleNetMD_QCD().at(fatjet_i);
                double xbb_score = pnet_xbb/(pnet_xbb + pnet_qcd);
                if (xbb_score > hbbjet_score)
                {
                    hbbjet_p4 = fatjet_p4;
                    hbbjet_score = xbb_score;
                }
            }
            if (n_fatjets >= 1)
            {
                cutflow.globals.setVal<LorentzVector>("hbbjet_p4", hbbjet_p4);
                return true;
            }
            else
            {
                return false;
            }
        }
    );
    cutflow.insert(exactly1tightlep_postskim, geq1fatjet_postskim, Right);

    Cut* STgt800_postskim = new LambdaCut(
        "POSTSKIM_STgt800", 
        [&]() 
        { 
            double ST = (
                cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").at(0).pt()
                + cutflow.globals.getVal<LorentzVector>("hbbjet_p4").pt()
                + nt.MET_pt()
            );
            return (ST > 800);
        }
    );
    cutflow.insert(geq1fatjet_postskim, STgt800_postskim, Right);

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            arbusto.init(ttree);
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            gconf.GetConfigs(nt.year());
            gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("16APV"));
            // Store metadata ttrees
            TTree* runtree = ((TTree*)ttree->GetCurrentFile()->Get("Runs"))->CloneTree();
            runtree->SetDirectory(0);
            runs->Add(runtree);
            TTree* lumitree = ((TTree*)ttree->GetCurrentFile()->Get("LuminosityBlocks"))->CloneTree();
            lumitree->SetDirectory(0);
            lumis->Add(lumitree);
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // reset branches and globals
                arbusto.resetBranches();
                cutflow.globals.resetVars();
                // run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.run(STgt800_postskim);
                if (passed) { arbusto.fill(entry); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data) { cutflow.print(); }

    TTree* merged_runs = TTree::MergeTrees(runs);
    merged_runs->SetName("Runs");
    TTree* merged_lumis = TTree::MergeTrees(lumis);
    merged_lumis->SetName("LuminosityBlocks");

    arbusto.tfile->cd();
    merged_runs->Write();
    merged_lumis->Write();
    arbusto.write();
    return 0;
}
