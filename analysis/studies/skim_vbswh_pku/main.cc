#include "vbswh/collections.h"
#include "vbswh/cuts.h"
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
    TList* runs = new TList();
    TList* lumis = new TList();

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");

    VBSWH::SkimmerPKU skimmer = VBSWH::SkimmerPKU(arbusto, nt, cli, cutflow);
    skimmer.initCutflow();

    Cut* base = new LambdaCut(
        "Base", 
        [&]() { return true; },
        [&]() { return cli.scale_factor; }
    );
    cutflow.replace("Base", base);

    unsigned int lumi_nevents = int(40000*(1./cli.scale_factor));

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            skimmer.init(ttree);
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
            if (cli.debug && looper.n_events_processed == lumi_nevents) { looper.stop(); }
            else
            {
                // reset branches and globals
                arbusto.resetBranches();
                cutflow.globals.resetVars();
                // run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.run("STgt800");
                if (passed) { arbusto.fill(entry); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.write(cli.output_dir);

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
