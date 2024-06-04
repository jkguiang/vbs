#include "core/collections.h"
#include "core/cuts.h"
#include "tools/mvaTTH.h"
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

    arbusto.newVecBranch<float>("Electron_mvaTTHUL", {});
    arbusto.newVecBranch<float>("Muon_mvaTTHUL", {});

    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");

    Core::Skimmer skimmer = Core::Skimmer(arbusto, nt, cli, cutflow);
    
    MVATTH::MVATTH mvatth_2018("../skimmer/data/leptonMVA/UL20_2018.xml");
    MVATTH::MVATTH mvatth_2017("../skimmer/data/leptonMVA/UL20_2017.xml");
    MVATTH::MVATTH mvatth_2016("../skimmer/data/leptonMVA/UL20_2016.xml");
    MVATTH::MVATTH mvatth_2016APV("../skimmer/data/leptonMVA/UL20_2016APV.xml");

    /* --- Assemble cutflow --- */

    Cut* base = new LambdaCut(
        "Base", 
        [&]() 
        { 
            float mvatth_score = -999;
            if (nt.year() == 2018)
            {
                mvatth_score = mvatth_2018.computeElecMVA(elec_i);
            }
            else if (nt.year() == 2017)
            {
                mvatth_score = mvatth_2017.computeElecMVA(elec_i);
            }
            else if (nt.year() == 2016)
            {
                if (gconf.isAPV)
                {
                    mvatth_score = mvatth_2016APV.computeElecMVA(elec_i);
                }
                else
                {
                    mvatth_score = mvatth_2016.computeElecMVA(elec_i);
                }
            }
            for (unsigned int elec_i = 0; elec_i < nt.nElectron(); ++elec_i)
            {
                arbusto.appendToVecLeaf<float>("Electron_mvaTTHUL", mvatth_score);
            }
            for (unsigned int muon_i = 0; muon_i < nt.nMuon(); ++muon_i)
            {
                arbusto.appendToVecLeaf<float>("Muon_mvaTTHUL", nt.Muon_mvaTTH().at(muon_i));
            }
            return true; 
        }
    );
    cutflow.setRoot(base);

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
                // Run cutflow
                nt.GetEntry(entry);
                cutflow.run();
                arbusto.fill(entry);
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data) { cutflow.print(); }

    skimmer.write();

    return 0;
}
