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
    arbol.newBranch<int>("ld_lep_pdgID", -999);
    arbol.newBranch<int>("md_lep_pdgID", -999);
    arbol.newBranch<int>("tr_lep_pdgID", -999);
    arbol.newBranch<float>("ld_lep_pt", -999);
    arbol.newBranch<float>("md_lep_pt", -999);
    arbol.newBranch<float>("tr_lep_pt", -999);
    arbol.newBranch<float>("ld_lep_eta", -999);
    arbol.newBranch<float>("md_lep_eta", -999);
    arbol.newBranch<float>("tr_lep_eta", -999);
    arbol.newBranch<float>("ld_lep_phi", -999);
    arbol.newBranch<float>("md_lep_phi", -999);
    arbol.newBranch<float>("tr_lep_phi", -999);
    arbol.newBranch<float>("LT", -999);
    arbol.newBranch<int>("n_SFOS_lep_pairs", -999);
    arbol.newBranch<bool>("passed_Z_veto", false);
    arbol.newBranch<float>("non_Z_dilep_M_T", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVector>("ld_lep_p4");
    cutflow.globals.newVar<LorentzVector>("md_lep_p4");
    cutflow.globals.newVar<LorentzVector>("tr_lep_p4");
    cutflow.globals.newVar<LorentzVectors>("Z_hyp_p4s");

    // Pack above into struct for convenience
    VBSWHAnalysis analysis = VBSWHAnalysis(arbol, nt, cli, cutflow);

    // Bookkeeping
    Cut* bookkeeping = new Bookkeeping("Bookkeeping", analysis);
    cutflow.setRoot(bookkeeping);
    // 3 Lep preselection
    Cut* has_3leps_presel = new Has3LepsPresel("Has3LepsPresel", analysis);
    cutflow.insert(bookkeeping->name, has_3leps_presel, Right);
    // Lepton selection
    Cut* select_leps = new SelectLeptons("SelectLeptons", analysis);
    cutflow.insert(has_3leps_presel->name, select_leps, Right);
    // Jet selection
    Cut* select_jets = new SelectJets("SelectJets", analysis);
    cutflow.insert(select_leps->name, select_jets, Right);
    // >= 2 jets
    Cut* geq2jets_nobtags = new VBSNoBTagPresel("Geq2JetsNoBTags", analysis);
    cutflow.insert(select_jets->name, geq2jets_nobtags, Right);
    // VBS jet selection
    Cut* select_vbsjets_maxE = new SelectVBSJetsMaxE("SelectVBSJetsMaxE", analysis);
    cutflow.insert(geq2jets_nobtags->name, select_vbsjets_maxE, Right);
    // == 3 lepton selection
    Cut* has_3leps = new Has3Leps("Has3Leps", analysis);
    cutflow.insert(select_vbsjets_maxE->name, has_3leps, Right);
    // 3-lepton SM selection
    Cut* smlike_3leps = new SMLike3Leps("SMLike3Leps", analysis);
    cutflow.insert(has_3leps->name, smlike_3leps, Right);
    // VBS jet preselection (Mjj > 500, etajj > 3)
    Cut* vbs_presel = new VBSPresel("MjjGt500detajjGt3", analysis);
    cutflow.insert(smlike_3leps->name, vbs_presel, Right);
    // 0SFOS
    Cut* has_3leps_0SFOS = new LambdaCut(
        "0SFOS",
        [&]()
        {
            return arbol.getLeaf<int>("n_SFOS_lep_pairs") == 0;
        }
    );
    cutflow.insert(vbs_presel->name, has_3leps_0SFOS, Right);
    // 1SFOS
    Cut* has_3leps_1SFOS = new LambdaCut(
        "1SFOS",
        [&]()
        {
            return arbol.getLeaf<int>("n_SFOS_lep_pairs") == 1;
        }
    );
    cutflow.insert(has_3leps_0SFOS->name, has_3leps_1SFOS, Left);
    // 1SFOS w/ Z veto
    Cut* Z_veto_1SFOS = new ZVeto3Leps("ZVeto_1SFOS", analysis);
    cutflow.insert(has_3leps_1SFOS->name, Z_veto_1SFOS, Right);
    // 2SFOS
    Cut* has_3leps_2SFOS = new LambdaCut(
        "2SFOS",
        [&]()
        {
            return arbol.getLeaf<int>("n_SFOS_lep_pairs") == 2;
        }
    );
    cutflow.insert(has_3leps_1SFOS->name, has_3leps_2SFOS, Left);
    // 2SFOS w/ Z veto
    Cut* Z_veto_2SFOS = new ZVeto3Leps("ZVeto_2SFOS", analysis);
    cutflow.insert(has_3leps_2SFOS->name, Z_veto_2SFOS, Right);

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
            bool passed = cutflow.run("Has3LepsPresel");
            if (passed) { arbol.fillTTree(); }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.write(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
