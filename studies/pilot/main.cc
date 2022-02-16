#include "main.h"

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

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");

    VBS3LepCuts cuts = VBS3LepCuts(arbol, nt, cli, cutflow);
    cutflow.setRoot(cuts.bookkeeping);
    cutflow.insert(cuts.bookkeeping->name, cuts.has_3leps_presel, Right);
    cutflow.insert(cuts.has_3leps_presel->name, cuts.select_leps, Right);
    cutflow.insert(cuts.select_leps->name, cuts.select_jets, Right);
    cutflow.insert(cuts.select_jets->name, cuts.geq_2_jets, Right);
    cutflow.insert(cuts.geq_2_jets->name, cuts.no_tight_b_jets, Right);
    cutflow.insert(cuts.no_tight_b_jets->name, cuts.select_vbs_jets_maxE, Right);
    cutflow.insert(cuts.select_vbs_jets_maxE->name, cuts.has_3leps, Right);
    cutflow.insert(cuts.has_3leps->name, cuts.has_3leps_0SFOS, Right);
    cutflow.insert(cuts.has_3leps_0SFOS->name, cuts.has_3leps_1SFOS, Left);
    Cut* Z_veto_1SFOS = cuts.Z_veto->clone(cuts.Z_veto->name+"_1SFOS");
    cutflow.insert(cuts.has_3leps_1SFOS->name, Z_veto_1SFOS, Right);
    cutflow.insert(cuts.has_3leps_1SFOS->name, cuts.has_3leps_2SFOS, Left);
    Cut* Z_veto_2SFOS = cuts.Z_veto->clone(cuts.Z_veto->name+"_2SFOS");
    cutflow.insert(cuts.has_3leps_2SFOS->name, Z_veto_2SFOS, Right);

    // Run looper
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            gconf.GetConfigs(nt.year());
        },
        [&](int entry) 
        {
            // Reset branches and globals
            arbol.resetBranches();
            cutflow.globals.resetVars();
            // Run cutflow
            nt.GetEntry(entry);
            bool passed = cutflow.runUntil(cuts.has_3leps->name);
            if (passed) { arbol.fillTTree(); }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.writeCSV(cli.output_dir);
    cutflow.writeMermaid(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
