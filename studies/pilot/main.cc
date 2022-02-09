#include "main.h"

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper<Nano>(&nt, cli.input_tchain, cli.input_ttree);

    // Initialize Arbol
    TFile* output_tfile = new TFile(
        TString(cli.output_dir+"/"+cli.output_name+".root"),
        "RECREATE"
    );
    Arbol arbol = Arbol(output_tfile);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow();

    VBS3LepCuts cuts = VBS3LepCuts(arbol, nt, cli, cutflow);
    cutflow.setRoot(cuts.bookkeeping);
    cutflow.insert(cuts.bookkeeping->name, cuts.has_3leps_presel, Right);
    cutflow.insert(cuts.has_3leps_presel->name, cuts.select_leps, Right);
    cutflow.insert(cuts.select_leps->name, cuts.select_jets, Right);
    cutflow.insert(cuts.select_jets->name, cuts.geq_2_jets, Right);
    cutflow.insert(cuts.geq_2_jets->name, cuts.no_tight_b_jets, Right);
    cutflow.insert(cuts.no_tight_b_jets->name, cuts.select_vbs_jets_maxE, Right);

    // Run looper
    looper.run(
        [&]() 
        {
            // Reset branches and globals
            arbol.resetBranches();
            cutflow.globals.resetVars();
            // Run cutflow
            nt.GetEntry(looper.current_index);
            bool passed = cutflow.runUntil(cuts.select_vbs_jets_maxE->name);
            if (passed) { arbol.fillTTree(); }
            return;
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.writeCSV(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
