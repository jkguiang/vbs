#include "vbsvvhjets/collections.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "histflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
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
    arbol.newBranch<double>("reweight_c2v_eq_3", -999);

    // Initialize Cutflow
    Histflow cutflow = Histflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCutflow();

    TH3D* hbb_hist = new TH3D("hbbfatjet_score", "hbbfatjet_score", 7, 300, 1000, 5, 0, 2.5, 1000, 0, 1);
    TH3D* vqq_hist = new TH3D("vqqfatjet_score", "vqqfatjet_score", 7, 300, 1000, 5, 0, 2.5, 1000, 0, 1);
    hbb_hist->Sumw2();
    vqq_hist->Sumw2();

    cutflow.bookHist3D<TH3D>(
        "SemiMerged_Geq4Jets", hbb_hist, 
        [&]() 
        {
            double pt = arbol.getLeaf<double>("hbbfatjet_pt");
            double eta = fabs(arbol.getLeaf<double>("hbbfatjet_eta"));
            double score = fabs(arbol.getLeaf<double>("hbbfatjet_score"));
            return std::make_tuple(pt, eta, score);
        }
    );

    cutflow.bookHist3D<TH3D>(
        "SemiMerged_Geq4Jets", vqq_hist, 
        [&]() 
        {
            double pt = arbol.getLeaf<double>("hbbfatjet_pt");
            double eta = fabs(arbol.getLeaf<double>("hbbfatjet_eta"));
            double score = fabs(arbol.getLeaf<double>("hbbfatjet_score"));
            return std::make_tuple(pt, eta, score);
        }
    );

    /* Trying 3D hists instead
    std::vector<float> eta_edges = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5};
    std::vector<float> pt_edges = {300, 350, 450, 600, 800, 1000};

    // Initialize histograms
    std::vector<TH1D*> hbb_hists;
    std::vector<TH1D*> vqq_hists;
    for (unsigned int edge_i = 0; edge_i < pt_edges.size() - 1; ++edge_i)
    {
        for (unsigned int edge_j = 0; edge_j < eta_edges.size() - 1; ++edge_j)
        {
            // Dumb hack to get edge_i and edge_j in plot title
            TString tag = "_ptbin";
            tag += edge_i;
            tag += "_etabin";
            tag += edge_j;

            TH1D* hbb_hist = new TH1D("hbbfatjet_score"+tag, "hbbfatjet_score"+tag, 1000, 0, 1);
            TH1D* vqq_hist = new TH1D("vqqfatjet_score"+tag, "vqqfatjet_score"+tag, 1000, 0, 1);
            hbb_hist->Sumw2();
            vqq_hist->Sumw2();
            hbb_hists.push_back(hbb_hist);
            vqq_hists.push_back(vqq_hist);
        }
    }
    for (unsigned int edge_i = 0; edge_i < pt_edges.size() - 1; ++edge_i)
    {
        // Dumb hack to get edge_i in plot title
        TString tag = "_ptbin";
        tag += edge_i;
        hbb_hists.push_back(new TH1D("hbbfatjet_score"+tag+"_etabinOF", "hbbfatjet_score"+tag+"_etabinOF", 1000, 0, 1));
        vqq_hists.push_back(new TH1D("vqqfatjet_score"+tag+"_etabinOF", "vqqfatjet_score"+tag+"_etabinOF", 1000, 0, 1));
    }
    for (unsigned int edge_j = 0; edge_j < eta_edges.size() - 1; ++edge_j)
    {
        // Dumb hack to get edge_j in plot title
        TString tag = "_etabin";
        tag += edge_j;
        hbb_hists.push_back(new TH1D("hbbfatjet_score_ptbinOF"+tag, "hbbfatjet_score_ptbinOF"+tag, 1000, 0, 1));
        vqq_hists.push_back(new TH1D("vqqfatjet_score_ptbinOF"+tag, "vqqfatjet_score_ptbinOF"+tag, 1000, 0, 1));
    }
    hbb_hists.push_back(new TH1D("hbbfatjet_score_ptbinOF_etabinOF", "hbbfatjet_score_ptbinOF_etabinOF", 1000, 0, 1));
    vqq_hists.push_back(new TH1D("vqqfatjet_score_ptbinOF_etabinOF", "vqqfatjet_score_ptbinOF_etabinOF", 1000, 0, 1));

    // Book histograms
    unsigned int hist_i = 0;
    float pt_low, pt_high, eta_low, eta_high;
    for (unsigned int edge_i = 0; edge_i < pt_edges.size() - 1; ++edge_i)
    {
        pt_low = pt_edges.at(edge_i);
        pt_high = pt_edges.at(edge_i+1);
        for (unsigned int edge_j = 0; edge_j < eta_edges.size() - 1; ++edge_j)
        {
            eta_low = eta_edges.at(edge_j);
            eta_high = eta_edges.at(edge_j+1);
            cutflow.bookHist1D<TH1D>(
                "SemiMerged_Geq4Jets", hbb_hists.at(hist_i), 
                [&, pt_low, pt_high, eta_low, eta_high]() 
                {
                    double pt = arbol.getLeaf<double>("hbbfatjet_pt");
                    double eta = fabs(arbol.getLeaf<double>("hbbfatjet_eta"));
                    return (pt >= pt_low) && (pt < pt_high) && (eta >= eta_low) && (eta < eta_high);
                },
                [&]() { return arbol.getLeaf<double>("hbbfatjet_score"); }
            );
            cutflow.bookHist1D<TH1D>(
                "SemiMerged_Geq4Jets", vqq_hists.at(hist_i), 
                [&, pt_low, pt_high, eta_low, eta_high]() 
                {
                    double pt = arbol.getLeaf<double>("ld_vqqfatjet_pt");
                    double eta = fabs(arbol.getLeaf<double>("ld_vqqfatjet_eta"));
                    return (pt >= pt_low) && (pt < pt_high) && (eta >= eta_low) && (eta < eta_high);
                },
                [&]() { return arbol.getLeaf<double>("ld_vqqfatjet_score"); }
            );
            hist_i++;
        }
    }
    for (unsigned int edge_i = 0; edge_i < pt_edges.size() - 1; ++edge_i)
    {
        pt_low = pt_edges.at(edge_i);
        pt_high = pt_edges.at(edge_i+1);
        eta_low = eta_edges.back();
        cutflow.bookHist1D<TH1D>(
            "SemiMerged_Geq4Jets", hbb_hists.at(hist_i), 
            [&, pt_low, pt_high, eta_low]() 
            {
                double pt = arbol.getLeaf<double>("hbbfatjet_pt");
                double eta = fabs(arbol.getLeaf<double>("hbbfatjet_eta"));
                return (pt >= pt_low) && (pt < pt_high) && (eta >= eta_low);
            },
            [&]() { return arbol.getLeaf<double>("hbbfatjet_score"); }
        );
        cutflow.bookHist1D<TH1D>(
            "SemiMerged_Geq4Jets", vqq_hists.at(hist_i), 
            [&, pt_low, pt_high, eta_low]() 
            {
                double pt = arbol.getLeaf<double>("ld_vqqfatjet_pt");
                double eta = fabs(arbol.getLeaf<double>("ld_vqqfatjet_eta"));
                return (pt >= pt_low) && (pt < pt_high) && (eta >= eta_low);
            },
            [&]() { return arbol.getLeaf<double>("ld_vqqfatjet_score"); }
        );
        hist_i++;
    }
    for (unsigned int edge_j = 0; edge_j < eta_edges.size() - 1; ++edge_j)
    {
        eta_low = eta_edges.at(edge_j);
        eta_high = eta_edges.at(edge_j+1);
        pt_low = pt_edges.back();
        cutflow.bookHist1D<TH1D>(
            "SemiMerged_Geq4Jets", hbb_hists.at(hist_i), 
            [&, pt_low, eta_low, eta_high]() 
            {
                double pt = arbol.getLeaf<double>("hbbfatjet_pt");
                double eta = fabs(arbol.getLeaf<double>("hbbfatjet_eta"));
                return (pt >= pt_low) && (eta >= eta_low) && (eta < eta_high);
            },
            [&]() { return arbol.getLeaf<double>("hbbfatjet_score"); }
        );
        cutflow.bookHist1D<TH1D>(
            "SemiMerged_Geq4Jets", vqq_hists.at(hist_i), 
            [&, pt_low, eta_low, eta_high]() 
            {
                double pt = arbol.getLeaf<double>("ld_vqqfatjet_pt");
                double eta = fabs(arbol.getLeaf<double>("ld_vqqfatjet_eta"));
                return (pt >= pt_low) && (eta >= eta_low) && (eta < eta_high);
            },
            [&]() { return arbol.getLeaf<double>("ld_vqqfatjet_score"); }
        );
        hist_i++;
    }
    cutflow.bookHist1D<TH1D>(
        "SemiMerged_Geq4Jets", hbb_hists.back(), 
        [&]() 
        {
            double pt = arbol.getLeaf<double>("hbbfatjet_pt");
            double eta = fabs(arbol.getLeaf<double>("hbbfatjet_eta"));
            return (pt >= pt_edges.back()) && (eta >= eta_edges.back());
        },
        [&]() { return arbol.getLeaf<double>("hbbfatjet_score"); }
    );
    cutflow.bookHist1D<TH1D>(
        "SemiMerged_Geq4Jets", vqq_hists.back(), 
        [&]() 
        {
            double pt = arbol.getLeaf<double>("ld_vqqfatjet_pt");
            double eta = fabs(arbol.getLeaf<double>("ld_vqqfatjet_eta"));
            return (pt >= pt_edges.back()) && (eta >= eta_edges.back());
        },
        [&]() { return arbol.getLeaf<double>("ld_vqqfatjet_score"); }
    );
    */

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
                std::vector<std::string> cuts_to_check = {
                    "AllMerged_SaveVariables"
                };
                std::vector<bool> checkpoints = cutflow.run(cuts_to_check);
                if (checkpoints.at(0)) { arbol.fill(); }

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
    cutflow.writeHists(arbol.tfile);
    arbol.write();
    return 0;
}
