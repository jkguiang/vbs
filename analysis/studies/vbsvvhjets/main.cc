#include "vbsvvhjets/collections.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
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
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCutflow();

    Cut* reweight_c2v = new LambdaCut(
        "ReweightC2Vto3",
        [&]() 
        { 
            if (cli.is_signal && nt.nLHEReweightingWeight() > 0)
            {
                arbol.setLeaf<double>("reweight_c2v_eq_3", nt.LHEReweightingWeight().at(31));
            }
            return true;
        },
        [&]() 
        { 
            double weight = 1.;
            if (cli.is_signal && nt.nLHEReweightingWeight() > 0)
            {
                weight = arbol.setLeaf<double>("reweight_c2v_eq_3");
            }
            return weight;
        }
    );
    cutflow.insert("Bookkeeping", reweight_c2v, Right);

    // Cut* debug_gen = new LambdaCut(
    //     "DEBUG_GEN",
    //     [&]()
    //     {}
    // );
    // cutflow.insert("Bookkeeping", debug_gen, Right);

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

                nt.GetEntry(entry);
                // Run cutflow
                bool passed = cutflow.run("AllMerged_STGt800");
                if (passed) { arbol.fill(); }

                // Update progress bar
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
    arbol.write();
    return 0;
}
