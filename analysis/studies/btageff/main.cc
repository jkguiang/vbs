// VBS
#include "../core.h"
#include "../jes.h"
#include "../sfs.h"
#include "../vbswh.h"
#include "cuts.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "TObject.h"
#include "Math/VectorUtil.h"
#include "TH2.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "ElectronSelections.h"
#include "MuonSelections.h"
#include "tqdm.h"

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");

    // Initialize JECs/JERs
    JetEnergyScales* jes = new JetEnergyScales(cli.variation);

    // Pack above into VBSWH struct
    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.jes = jes;
    analysis.initBranches();
    analysis.initCutflow();

    FillHistograms* hists = new FillHistograms("FillHistograms", analysis);
    cutflow.insert("SelectJetsNoHbbOverlap", hists, Right);

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree) 
        { 
            nt.Init(ttree); 
            analysis.init();
            jes->init();
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // Reset branches and globals
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                cutflow.run();
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    cutflow.print();

    arbol.tfile->cd();
    hists->writeHists();
    arbol.write();
    return 0;
}
