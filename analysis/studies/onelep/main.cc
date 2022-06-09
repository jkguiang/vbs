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
#include "tqdm.h"

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
    arbol.newBranch<int>("lep_pdgID", -999);
    arbol.newBranch<float>("lep_pt", -999);
    arbol.newBranch<float>("lep_eta", -999);
    arbol.newBranch<float>("lep_phi", -999);
    arbol.newBranch<float>("LT", -999);
    arbol.newBranch<float>("hbbjet_score", -999);
    arbol.newBranch<float>("hbbjet_pt", -999);
    arbol.newBranch<float>("hbbjet_eta", -999);
    arbol.newBranch<float>("hbbjet_phi", -999);
    arbol.newBranch<float>("hbbjet_mass", -999);
    arbol.newBranch<float>("hbbjet_msoftdrop", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVector>("lep_p4");
    cutflow.globals.newVar<LorentzVector>("hbbjet_p4");

    // Pack above into VBSWH struct (also adds branches)
    VBSWHAnalysis analysis = VBSWHAnalysis(arbol, nt, cli, cutflow);

    // Bookkeeping
    Cut* bookkeeping = new Bookkeeping("Bookkeeping", analysis);
    cutflow.setRoot(bookkeeping);

    // Lepton selection
    Cut* select_leps = new SelectLeptons("SelectLeptons", analysis);
    cutflow.insert(bookkeeping->name, select_leps, Right);

    // == 1 lepton selection
    Cut* has_1lep = new Has1Lep("Has1TightLep", analysis);
    cutflow.insert(select_leps->name, has_1lep, Right);

    // Lepton has pT > 40
    Cut* lep_pt_gt40 = new LambdaCut(
        "LepPtGt40", [&]() { return arbol.getLeaf<float>("lep_pt") >= 40; }
    );
    cutflow.insert(has_1lep->name, lep_pt_gt40, Right);

    // Fat jet selection
    Cut* select_fatjets = new SelectFatJets("SelectFatJets", analysis);
    cutflow.insert(lep_pt_gt40->name, select_fatjets, Right);

    // Geq1FatJet
    Cut* geq1fatjet = new LambdaCut(
        "Geq1FatJet", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 1; }
    );
    cutflow.insert(select_fatjets->name, geq1fatjet, Right);

    // Hbb selection
    Cut* select_hbbjet = new SelectHbbFatJet("SelectHbbFatJet", analysis);
    cutflow.insert(geq1fatjet->name, select_hbbjet, Right);

    // Hbb score > 0.9
    Cut* hbbjet_score_gt0p9 = new LambdaCut(
        "PNetHbbScoreGt0p9", [&]() { return arbol.getLeaf<float>("hbbjet_score") > 0.9; }
    );
    cutflow.insert(select_hbbjet->name, hbbjet_score_gt0p9, Right);

    // Jet selection
    Cut* select_jets = new SelectJetsNoHbbOverlap("SelectJetsNoHbbOverlap", analysis);
    cutflow.insert(hbbjet_score_gt0p9->name, select_jets, Right);

    // VBS jet selection
    Cut* select_vbsjets_maxE = new SelectVBSJetsMaxE("SelectVBSJetsMaxE", analysis);
    cutflow.insert(select_jets->name, select_vbsjets_maxE, Right);

    // Basic VBS jet requirements
    Cut* vbsjets_presel = new VBSPresel("MjjGt500detajjGt3", analysis);
    cutflow.insert(select_vbsjets_maxE->name, vbsjets_presel, Right);

    // Run looper
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            gconf.GetConfigs(nt.year());
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // Reset branches and globals
                arbol.resetBranches();
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.runUntil(vbsjets_presel->name);
                if (passed) { arbol.fillTTree(); }
            }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.write(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
