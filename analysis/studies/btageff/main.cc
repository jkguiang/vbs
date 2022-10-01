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

    // Pack above into VBSWH struct
    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();

    Cut* base = new LambdaCut("Base", [&]() { return true; });
    cutflow.setRoot(base);

    // Event filters
    Cut* event_filters = new VBSWH::PassesEventFilters("PassesEventFilters", analysis);
    cutflow.insert(base, event_filters, Right);

    // Lepton selection
    Cut* select_leps = new Core::SelectLeptonsPKU("SelectLeptons", analysis);
    // Cut* select_leps = new Core::SelectLeptons("SelectLeptons", analayis);
    cutflow.insert(event_filters, select_leps, Right);

    // == 1 lepton selection
    Cut* has_1lep = new VBSWH::Has1LepPKU("Has1TightLep", analysis);
    // Cut* has_1lep = new Has1Lep("Has1TightLep", analysis);
    cutflow.insert(select_leps, has_1lep, Right);

    // Lepton has pT > 40
    Cut* lep_pt_gt40 = new LambdaCut(
        "LepPtGt40", [&]() { return arbol.getLeaf<double>("lep_pt") >= 40; }
    );
    cutflow.insert(has_1lep, lep_pt_gt40, Right);

    // Single-lepton triggers
    Cut* lep_triggers = new VBSWH::Passes1LepTriggers("Passes1LepTriggers", analysis);
    cutflow.insert(lep_pt_gt40, lep_triggers, Right);

    // Fat jet selection
    Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", analysis);
    cutflow.insert(lep_triggers, select_fatjets, Right);

    // Geq1FatJet
    Cut* geq1fatjet = new LambdaCut(
        "Geq1FatJet", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 1; }
    );
    cutflow.insert(select_fatjets, geq1fatjet, Right);

    // Hbb selection
    Cut* select_hbbjet = new VBSWH::SelectHbbFatJet("SelectHbbFatJet", analysis, true);
    cutflow.insert(geq1fatjet, select_hbbjet, Right);

    // Jet selection
    JetEnergyScales* jes = new JetEnergyScales(cli.variation);
    Cut* select_jets = new VBSWH::SelectJetsNoHbbOverlap("SelectJetsNoHbbOverlap", analysis, jes);
    cutflow.insert(select_hbbjet, select_jets, Right);

    FillHistograms* hists = new FillHistograms("FillHistograms", analysis);
    cutflow.insert(select_jets, hists, Right);

    // Run looper
    gconf.nanoAOD_ver = 9;
    tqdm bar;
    looper.run(
        [&](TTree* ttree) 
        { 
            nt.Init(ttree); 
            gconf.GetConfigs(nt.year());
            TString file_name = ttree->GetCurrentFile()->GetName();
            gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("16APV"));
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
                bool passed = cutflow.run(hists);
                if (passed) { arbol.fill(); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.writeCSV(cli.output_dir);

    arbol.tfile->cd();
    hists->writeHists();
    arbol.write();
    return 0;
}
