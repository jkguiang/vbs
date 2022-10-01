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
    Looper looper = Looper(cli.input_tchain);

    // Initialize Arbusto
    TFile* output_tfile = new TFile(
        TString(cli.output_dir+"/"+cli.output_name+".root"),
        "RECREATE"
    );
    Arbusto arbusto = Arbusto(
        output_tfile,
        cli.input_tchain,
        {
            "Electron*",
            "Muon*",
            "Jet*",
            "Tau*",
            "GenPart*",
            "Generator*",
            "FatJet*",
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

    // Exactly 1 lepton
    Cut* findleps = new LambdaCut(
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
    cutflow.insert(base, findleps, Right);

    // Geq1VetoLep
    Cut* geq1vetolep = new LambdaCut(
        "SKIM_Geq1VetoLep", [&]() { return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1; }
    );
    cutflow.insert(findleps, geq1vetolep, Right);

    // Geq2Jets
    Cut* geq2jets = new LambdaCut(
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
    cutflow.insert(geq1vetolep, geq2jets, Right);

    // Geq1FatJet
    Cut* geq1fatjet = new LambdaCut(
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
    cutflow.insert(geq2jets, geq1fatjet, Right);

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            arbusto.init(ttree);
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            gconf.GetConfigs(nt.year());
            gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("NanoAODAPV") || file_name.Contains("UL16APV"));
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // Reset branches and globals
                arbusto.resetBranches();
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.run(geq1fatjet);
                if (passed) { arbusto.fill(); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data)
    {
        cutflow.print();
    }
    arbusto.write();
    return 0;
}
