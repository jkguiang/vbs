#include "../core.h"
#include "cuts.h"
// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"

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
    arbol.newBranch<float>("xsec_sf", -999);
    arbol.newBranch<int>("event", -999);
    arbol.newBranch<float>("MET", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");

    // Pack above into struct for convenience
    VBSWHAnalysis analysis = VBSWHAnalysis(arbol, nt, cli, cutflow);

    // Bookkeeping
    Cut* bookkeeping = new Bookkeeping("Bookkeeping", analysis);
    cutflow.setRoot(bookkeeping);
    // 1 Lep preselection
    Cut* has1lep_presel = new Has1LepPresel("Has1LepPresel", analysis);
    cutflow.insert("Bookkeeping", has1lep_presel, Right);

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
            bool passed = cutflow.runUntil("Has1LepPresel");
            if (passed) { arbol.fillTTree(); }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.write(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
