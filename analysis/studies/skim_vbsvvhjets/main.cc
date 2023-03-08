#include "core/collections.h"
#include "core/cuts.h"
#include "vbswh/cuts.h"
#include "vbsvvhjets/cuts.h"
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
            "Jet*", "nJet", 
            "FatJet*", "nFatJet", 
            "GenPart*", "nGenPart",
            "GenJet*", "nGenJet", 
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
            "Pileup*",
            "fixedGridRhoFastjetAll"
        }
    );

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s", {});
    cutflow.globals.newVar<Integers>("veto_lep_pdgIDs", {});
    cutflow.globals.newVar<Integers>("tight_lep_pdgIDs", {});
    cutflow.globals.newVar<LorentzVectors>("jet_p4s", {});

    Core::Skimmer skimmer = Core::Skimmer(arbusto, nt, cli, cutflow);

    /* --- Assemble cutflow --- */

    Cut* base = new LambdaCut("Base", [&]() { return true; });
    cutflow.setRoot(base);

    Cut* find_leps = new VBSVVHJets::FindLeptonsTTHNoUL("FindLeptonsTTH", skimmer);
    cutflow.insert(base, find_leps, Right);

    Cut* no_leps = new LambdaCut(
        "NoVetoLeptons", 
        [&]() 
        { 
            return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() == 0; 
        }
    );
    cutflow.insert(find_leps, no_leps, Right);

    Cut* geq2_fatjets = new LambdaCut(
        "Geq2FatJets", 
        [&]() 
        { 
            LorentzVectors fatjet_p4s = {};
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                if (fatjet_p4.pt() > 300
                    && fabs(fatjet_p4.eta()) < 2.5
                    && fatjet_p4.mass() > 50
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 40
                    && nt.FatJet_jetId().at(fatjet_i) > 0)
                {
                    fatjet_p4s.push_back(fatjet_p4);
                }
            }
            if (fatjet_p4s.size() >= 2)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    );
    cutflow.insert(no_leps, geq2_fatjets, Right);

    Cut* geq2_jets = new LambdaCut(
        "Geq2Jets",
        [&]() 
        {
            LorentzVectors jet_p4s = {};
            for (unsigned int jet_i = 0; jet_i < nt.nJet(); jet_i++)
            {
                LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
                bool passes_jet_id = (
                    (nt.year() == 2016 && nt.Jet_jetId().at(jet_i) >= 1)
                    || (nt.year() > 2016 && nt.Jet_jetId().at(jet_i) >= 2)
                );
                if (jet_p4.pt() > 20 && passes_jet_id)
                {
                    jet_p4s.push_back(jet_p4);
                }
            }
            cutflow.globals.setVal<LorentzVectors>("jet_p4s", jet_p4s);
            return (jet_p4s.size() >= 2);
        }
    );
    cutflow.insert(geq2_fatjets, geq2_jets, Right);

    Cut* find_vbsjets = new LambdaCut(
        "FindVBSJetPairs", 
        [&]() 
        { 
            LorentzVectors jet_p4s = cutflow.globals.getVal<LorentzVectors>("jet_p4s");
            int n_vbsjet_pairs = 0;
            for (unsigned int jet_i = 0; jet_i < jet_p4s.size(); ++jet_i)
            {
                for (unsigned int jet_j = jet_i + 1; jet_j < jet_p4s.size(); ++jet_j)
                {
                    LorentzVector jet1_p4 = jet_p4s.at(jet_i);
                    LorentzVector jet2_p4 = jet_p4s.at(jet_j);
                    double M_jj = (jet1_p4 + jet2_p4).M();
                    double abs_deta_jj = fabs(jet1_p4.eta() - jet2_p4.eta());
                    if (M_jj > 250 && abs_deta_jj > 2.5)
                    {
                        n_vbsjet_pairs++;
                    }
                }
            }
            return (n_vbsjet_pairs > 0);
        }
    );
    cutflow.insert(geq2_jets, find_vbsjets, Right);

    /* ------------------------ */

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            skimmer.init(ttree);
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
                bool passed = cutflow.run("FindVBSJetPairs");
                if (passed) { arbusto.fill(entry); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data) { cutflow.print(); }

    skimmer.write();

    return 0;
}
