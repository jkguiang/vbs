#include "../core.h"
#include "../sfs.h"
#include "../jes.h"
#include "../vbswh.h"
#include "cuts.h"
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
    Arbol pdf_arbol = Arbol(
        cli.output_dir+"/"+cli.output_name+"_pdf.root",
        "pdf_"+cli.output_ttree
    );
    for (int i = 1; i < 101; ++i)
    {
        pdf_arbol.newBranch<double>("lhe_pdf_"+std::to_string(i), -999);
    }

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");

    // Pack above into VBSWH struct
    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCorrections();
    analysis.initCutflow();

    Cut* fix_ewk_samples = new FixEWKSamples("FixEWKSamples", analysis);
    cutflow.insert("Bookkeeping", fix_ewk_samples, Right);

    Cut* save_pdfweights = new LambdaCut(
        "SavePDFWeights",
        [&]()
        {
            if (nt.isData()) { return true; }
            for (int i = 1; i < 101; ++i)
            {
                TString branch_name = "lhe_pdf_"+std::to_string(i);
                if (nt.nLHEPdfWeight() > 100)
                {
                    pdf_arbol.setLeaf<double>(branch_name, nt.LHEPdfWeight().at(i));
                }
                else
                {
                    pdf_arbol.setLeaf<double>(branch_name, 1.);
                }
            }
            return true;
        }
    );
    cutflow.insert("XbbGt0p9_MSDLt150", save_pdfweights, Right);

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
                pdf_arbol.resetBranches();
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                std::vector<bool> checkpoints = cutflow.run(
                    {
                        "SaveVariables",        // Object selection
                        "ApplyAk4GlobalBVeto",  // Preselection
                        "XbbGt0p9_MSDLt150"     // SR1
                    }
                );
                bool passed = (cli.variation == "nominal") ? checkpoints.at(0) : checkpoints.at(1);
                if (passed) 
                { 
                    arbol.fill(); 
                    if (checkpoints.at(2))
                    {
                        pdf_arbol.fill();
                    }
                }
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
    pdf_arbol.write();
    return 0;
}
