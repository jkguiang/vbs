#include "vbsvvhjets/collections.h"
#include "corrections/qcd.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "histflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
#include "TH2.h"
#include "TH3.h"
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
    Histflow cutflow = Histflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCorrections();
    analysis.initCutflow();
    
    // List of cut names to book histograms for
    std::vector<std::string> cuts_to_book = {"SemiMerged_SelectVBSJets", "AllMerged_SelectVBSJets"};

    QCDPNetXbbSFs* xbb_rwgt = new QCDPNetXbbSFs();
    if (cli.variation == "apply_xbb_rwgt")
    {
        // Create a cut that either applies the Hbb reweighting
        Cut* apply_xbb_rwgt = new LambdaCut(
            "SemiMerged_ApplyXbbReweighting",
            [&]() { return true; },
            [&, xbb_rwgt]() 
            { 
                double pt = arbol.getLeaf<double>("hbbfatjet_pt");
                double eta = arbol.getLeaf<double>("hbbfatjet_eta");
                double score = arbol.getLeaf<double>("hbbfatjet_score");
                return xbb_rwgt->getSF(pt, eta, score);
            }
        );
        cutflow.insert("SemiMerged_SelectVBSJets", apply_xbb_rwgt, Right);
        cuts_to_book.push_back("SemiMerged_ApplyXbbReweighting");
    }

    // Define histogram binning
    const int n_ptbins = 12;
    double ptbin_edges[n_ptbins+1] = {300., 310., 320., 340., 360., 400., 450., 500., 600., 700., 800., 900., 1000.};
    const int n_etabins = 5;
    double etabin_edges[n_etabins+1] = {0., 0.5, 1.0, 1.5, 2.0, 2.5};
    const int n_scorebins = 1000;
    double scorebin_edges[n_scorebins+1] = {0.};
    for (int scorebin_i = 0; scorebin_i < n_scorebins; ++scorebin_i)
    {
        scorebin_edges[scorebin_i+1] = 0.001*double(scorebin_i+1);
    }

    // Initialize 2D histograms
    std::vector<TH2D*> xbb_hists2D;
    xbb_hists2D.push_back(new TH2D("hbbfatjet_xbbscore2D", "hbbfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    xbb_hists2D.push_back(new TH2D("ld_vqqfatjet_xbbscore2D", "ld_vqqfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    xbb_hists2D.push_back(new TH2D("tr_vqqfatjet_xbbscore2D", "tr_vqqfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    for (auto hist : xbb_hists2D) { hist->Sumw2(); }
    std::vector<TH2D*> xvqq_hists2D;
    xvqq_hists2D.push_back(new TH2D("hbbfatjet_xvqqscore2D", "hbbfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    xvqq_hists2D.push_back(new TH2D("ld_vqqfatjet_xvqqscore2D", "ld_vqqfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    xvqq_hists2D.push_back(new TH2D("tr_vqqfatjet_xvqqscore2D", "tr_vqqfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    for (auto hist : xvqq_hists2D) { hist->Sumw2(); }
    std::vector<TH2D*> xwqq_hists2D;
    xwqq_hists2D.push_back(new TH2D("hbbfatjet_xwqqscore2D", "hbbfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    xwqq_hists2D.push_back(new TH2D("ld_vqqfatjet_xwqqscore2D", "ld_vqqfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    xwqq_hists2D.push_back(new TH2D("tr_vqqfatjet_xwqqscore2D", "tr_vqqfatjet", n_ptbins, ptbin_edges, n_scorebins, scorebin_edges));
    for (auto hist : xwqq_hists2D) { hist->Sumw2(); }

    // Initialize 3D histograms
    std::vector<TH3D*> xbb_hists3D;
    xbb_hists3D.push_back(new TH3D("hbbfatjet_xbbscore3D", "hbbfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    xbb_hists3D.push_back(new TH3D("ld_vqqfatjet_xbbscore3D", "ld_vqqfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    xbb_hists3D.push_back(new TH3D("tr_vqqfatjet_xbbscore3D", "tr_vqqfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    for (auto hist : xbb_hists3D) { hist->Sumw2(); }
    std::vector<TH3D*> xvqq_hists3D;
    xvqq_hists3D.push_back(new TH3D("hbbfatjet_xvqqscore3D", "hbbfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    xvqq_hists3D.push_back(new TH3D("ld_vqqfatjet_xvqqscore3D", "ld_vqqfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    xvqq_hists3D.push_back(new TH3D("tr_vqqfatjet_xvqqscore3D", "tr_vqqfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    for (auto hist : xvqq_hists3D) { hist->Sumw2(); }
    std::vector<TH3D*> xwqq_hists3D;
    xwqq_hists3D.push_back(new TH3D("hbbfatjet_xwqqscore3D", "hbbfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    xwqq_hists3D.push_back(new TH3D("ld_vqqfatjet_xwqqscore3D", "ld_vqqfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    xwqq_hists3D.push_back(new TH3D("tr_vqqfatjet_xwqqscore3D", "tr_vqqfatjet", n_ptbins, ptbin_edges, n_etabins, etabin_edges, n_scorebins, scorebin_edges));
    for (auto hist : xwqq_hists3D) { hist->Sumw2(); }

    // Book histograms
    for (auto cut_name : cuts_to_book)
    {
        for (unsigned int hist_i = 0; hist_i < xbb_hists3D.size(); ++hist_i)
        {
            TH3D* xbb_hist3D = xbb_hists3D.at(hist_i);
            TH3D* xvqq_hist3D = xvqq_hists3D.at(hist_i);
            TH3D* xwqq_hist3D = xwqq_hists3D.at(hist_i);
            TH2D* xbb_hist2D = xbb_hists2D.at(hist_i);
            TH2D* xvqq_hist2D = xvqq_hists2D.at(hist_i);
            TH2D* xwqq_hist2D = xwqq_hists2D.at(hist_i);
            std::string obj_name = std::string(xbb_hist3D->GetTitle());
            if (TString(cut_name).Contains("SemiMerged_") && obj_name == "tr_vqqfatjet") { continue; }
            cutflow.bookHist3D<TH3D>(
                cut_name, xbb_hist3D, 
                [&, obj_name]() 
                {
                    unsigned int gidx = cutflow.globals.getVal<unsigned int>(obj_name+"_gidx");
                    double pt = arbol.getLeaf<double>(obj_name+"_pt");
                    double eta = fabs(arbol.getLeaf<double>(obj_name+"_eta"));
                    double score = cutflow.globals.getVal<Doubles>("good_fatjet_xbbtags").at(gidx);
                    return std::make_tuple(pt, eta, score);
                }
            );
            cutflow.bookHist3D<TH3D>(
                cut_name, xvqq_hist3D, 
                [&, obj_name]() 
                {
                    unsigned int gidx = cutflow.globals.getVal<unsigned int>(obj_name+"_gidx");
                    double pt = arbol.getLeaf<double>(obj_name+"_pt");
                    double eta = fabs(arbol.getLeaf<double>(obj_name+"_eta"));
                    double score = cutflow.globals.getVal<Doubles>("good_fatjet_xvqqtags").at(gidx);
                    return std::make_tuple(pt, eta, score);
                }
            );
            cutflow.bookHist3D<TH3D>(
                cut_name, xwqq_hist3D, 
                [&, obj_name]() 
                {
                    unsigned int gidx = cutflow.globals.getVal<unsigned int>(obj_name+"_gidx");
                    double pt = arbol.getLeaf<double>(obj_name+"_pt");
                    double eta = fabs(arbol.getLeaf<double>(obj_name+"_eta"));
                    double score = cutflow.globals.getVal<Doubles>("good_fatjet_xwqqtags").at(gidx);
                    return std::make_tuple(pt, eta, score);
                }
            );
            cutflow.bookHist2D<TH2D>(
                cut_name, xbb_hist2D, 
                [&, obj_name]() 
                {
                    unsigned int gidx = cutflow.globals.getVal<unsigned int>(obj_name+"_gidx");
                    double pt = arbol.getLeaf<double>(obj_name+"_pt");
                    double score = cutflow.globals.getVal<Doubles>("good_fatjet_xbbtags").at(gidx);
                    return std::make_pair(pt, score);
                }
            );
            cutflow.bookHist2D<TH2D>(
                cut_name, xvqq_hist2D, 
                [&, obj_name]() 
                {
                    unsigned int gidx = cutflow.globals.getVal<unsigned int>(obj_name+"_gidx");
                    double pt = arbol.getLeaf<double>(obj_name+"_pt");
                    double score = cutflow.globals.getVal<Doubles>("good_fatjet_xvqqtags").at(gidx);
                    return std::make_pair(pt, score);
                }
            );
            cutflow.bookHist2D<TH2D>(
                cut_name, xwqq_hist2D, 
                [&, obj_name]() 
                {
                    unsigned int gidx = cutflow.globals.getVal<unsigned int>(obj_name+"_gidx");
                    double pt = arbol.getLeaf<double>(obj_name+"_pt");
                    double score = cutflow.globals.getVal<Doubles>("good_fatjet_xwqqtags").at(gidx);
                    return std::make_pair(pt, score);
                }
            );
        }
    }

    // Run looper
    tqdm bar;
    looper.run(
        [&, xbb_rwgt](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
            if (cli.variation == "apply_xbb_rwgt")
            {
                TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
                xbb_rwgt->init(file_name);
            }
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
                cutflow.run();

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
    arbol.tfile->cd();
    cutflow.writeHists(arbol.tfile);
    arbol.tfile->Close();
    return 0;
}
