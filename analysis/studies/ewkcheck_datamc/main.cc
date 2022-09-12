#include "../core.h"
#include "../scalefactors.h"
#include "cuts.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "histflow.h"
// ROOT
#include "TString.h"
#include "TObject.h"
#include "Math/VectorUtil.h"
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

    // Initialize TLists for metadata TTrees
    TList* runs = new TList();
    TList* lumis = new TList();

    // Initialize Cutflow
    Histflow cutflow = Histflow(cli.output_name + "_Cutflow");

    // Pack above into Analysis struct
    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();

    // Bookkeeping
    Cut* bookkeeping = new LambdaCut(
        "Bookkeeping", 
        [&]()
        {
            TString filename = cli.input_tchain->GetCurrentFile()->GetName();
            bool is_data = filename.Contains("Run201");
            arbol.setLeaf<double>("xsec_sf", (is_data) ? 1. : nt.genWeight());
            arbol.setLeaf<int>("event", nt.event());
            arbol.setLeaf<double>("MET", nt.MET_pt());
            return (is_data) ? goodrun(nt.run(), nt.luminosityBlock()) : true;
        },
        [&]()
        {
            TString filename = cli.input_tchain->GetCurrentFile()->GetName();
            bool is_data = filename.Contains("Run201");
            return (is_data) ? 1. : nt.genWeight();
        }
    );
    cutflow.setRoot(bookkeeping);

    // Lepton selection
    Cut* select_leps = new SelectLeptonsNoUL("SelectLeptons", analysis);
    cutflow.insert(bookkeeping, select_leps, Right);

    // == 1 lepton selection
    Cut* has_1lep = new Has1LepNoUL("Has1TightLep", analysis);
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

    // == 0 fat jets
    Cut* no_fatjets = new LambdaCut(
        "NoFatJets", 
        [&]()
        {
            return cutflow.globals.getVal<Integers>("good_fatjet_idxs").size() == 0;
        }
    );
    cutflow.insert(select_fatjets, no_fatjets, Right);

    // Jet selection
    Cut* select_jets = new Core::SelectJets("SelectJets", analysis);
    cutflow.insert(no_fatjets, select_jets, Right);

    // VBS jet selection
    Cut* select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SelectVBSJetsMaxE", analysis);
    cutflow.insert(select_jets, select_vbsjets_maxE, Right);

    // Global AK4 b-veto
    Cut* ak4bveto = new LambdaCut(
        "SaveAk4GlobalBVeto", 
        [&]()
        {
            for (auto& btag : cutflow.globals.getVal<Doubles>("good_jet_btags"))
            {
                if (btag > gconf.WP_DeepFlav_medium) 
                { 
                    return false;
                    break;
                }
            }
            return true;
        }
    );
    cutflow.insert(select_vbsjets_maxE, ak4bveto, Right);

    // Initialize hists
    std::vector<TH1D*> hists;
    hists.push_back(new TH1D("M_jj_hist", "M_jj", 5000, 0, 5000));
    hists.push_back(new TH1D("abs_deta_jj_hist", "abs_deta_jj", 1000, 0, 10));
    hists.push_back(new TH1D("dR_jj", "dR_jj", 1000, 0, 10));
    hists.push_back(new TH1D("ld_vbsjet_pt", "ld_vbsjet_pt", 2000, 0, 2000));
    hists.push_back(new TH1D("tr_vbsjet_pt", "tr_vbsjet_pt", 2000, 0, 2000));
    hists.push_back(new TH1D("n_jets_pt30", "n_jets_pt30", 20, 0, 20));
    hists.push_back(new TH1D("HT", "HT", 5000, 0, 5000));
    hists.push_back(new TH1D("MET", "MET", 1000, 0, 1000));
    hists.push_back(new TH1D("lep_pt", "lep_pt", 2000, 0, 2000));
    hists.push_back(new TH1D("lep_eta", "lep_eta", 2000, -10, 10));
    hists.push_back(new TH1D("lep_phi", "lep_phi", 1000, 0, 10));
    hists.push_back(new TH1D("LT", "LT", 5000, 0, 5000));
    // Book hists
    for (auto cut : { select_vbsjets_maxE, ak4bveto })
    {
        for (auto hist : hists)
        {
           cutflow.bookHist1D<TH1D>(
                cut, hist, 
                [&]() { return true; },
                [&, hist]() { return arbol.getLeaf<double>(hist->GetTitle()); }
            );
        }
    }

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
            // Store metadata ttrees
            TTree* runtree = ((TTree*)ttree->GetCurrentFile()->Get("Runs"))->CloneTree();
            runtree->SetDirectory(0);
            runs->Add(runtree);
            TTree* lumitree = ((TTree*)ttree->GetCurrentFile()->Get("LuminosityBlocks"))->CloneTree();
            lumitree->SetDirectory(0);
            lumis->Add(lumitree);
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
    cutflow.writeCSV(cli.output_dir);
    cutflow.writeHists(arbol.tfile);

    TTree* merged_runs = TTree::MergeTrees(runs);
    merged_runs->SetName("Runs");
    TTree* merged_lumis = TTree::MergeTrees(lumis);
    merged_lumis->SetName("LuminosityBlocks");

    arbol.tfile->cd();
    merged_runs->Write();
    merged_lumis->Write();
    arbol.write();
    return 0;
}
