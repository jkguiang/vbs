#include "vbsvvhjets/collections.h"
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
    arbol.newBranch<double>("reweight_c2v_eq_3", -999);
    arbol.setLeaf<double>("hbbfatjet_pdfscore");
    arbol.setLeaf<double>("ld_vqqfatjet_pdfscore");
    arbol.setLeaf<double>("tr_vqqfatjet_pdfscore");

    // Initialize Cutflow
    Histflow cutflow = Histflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCorrections();
    analysis.initCutflow();

    TFile* pnet_pdf_file = new TFile("data/vbsvvhjets_sfs/qcd_pnet_pdfs.root");
    TH2D* xbb_pdf = (TH2D*) pnet_pdf_file->Get("ParticleNet_Xbb_PDF_2D");
    TH2D* xvqq_pdf = (TH2D*) pnet_pdf_file->Get("ParticleNet_XVqq_PDF_2D");
    Cut* replace_pnets = new LambdaCut(
        "ReplacePNetsQCD",
        [&, xbb_pdf, xvqq_pdf]()
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            int bin;
            TH1D* pdf;
            if (file_name.Contains("QCD"))
            {
                LorentzVector hbbfatjet_p4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
                LorentzVector ld_vqqfatjet_p4 = cutflow.globals.getVal<LorentzVector>("ld_vqqfatjet_p4");
                LorentzVector tr_vqqfatjet_p4 = cutflow.globals.getVal<LorentzVector>("tr_vqqfatjet_p4");
                // Replace Hbb fat jet score
                bin = xbb_pdf->GetXaxis()->FindBin(hbbfatjet_p4.pt());
                pdf = xbb_pdf->ProjectionY("xbb_projy", bin, bin);
                pdf->Rebin(10);
                double max_xbb = -999;
                for (unsigned int i = 0; i < 3; ++i)
                {
                    double xbb = pdf->GetRandom();
                    if (xbb > max_xbb)
                    {
                        max_xbb = xbb;
                    }
                }
                arbol.setLeaf<double>("hbbfatjet_pdfscore", max_xbb);
                // Replace leading Vqq fat jet score
                bin = xvqq_pdf->GetXaxis()->FindBin(ld_vqqfatjet_p4.pt());
                pdf = xvqq_pdf->ProjectionY("xvqq_projy1", bin, bin);
                pdf->Rebin(10);
                arbol.setLeaf<double>("ld_vqqfatjet_pdfscore", pdf->GetRandom());
                // Replace trailing Vqq fat jet score
                bin = xvqq_pdf->GetXaxis()->FindBin(tr_vqqfatjet_p4.pt());
                pdf = xvqq_pdf->ProjectionY("xvqq_projy2", bin, bin);
                pdf->Rebin(10);
                arbol.setLeaf<double>("tr_vqqfatjet_pdfscore", pdf->GetRandom());
            }
            else
            {
                arbol.setLeaf<double>("hbbfatjet_pdfscore", arbol.getLeaf<double>("hbbfatjet_score"));
                arbol.setLeaf<double>("ld_vqqfatjet_pdfscore", arbol.getLeaf<double>("ld_vqqfatjet_score"));
                arbol.setLeaf<double>("tr_vqqfatjet_pdfscore", arbol.getLeaf<double>("tr_vqqfatjet_score"));
            }
            return true;
        }
    );
    cutflow.insert("AllMerged_SelectVVHFatJets", replace_pnets, Right);

    /* "Naive" QCD ParticleNet scale factors: Did not work very well
    QCDPNetXbbSFs qcd_xbb_sfs = new QCDPNetXbbSFs();
    QCDPNetXVqqSFs qcd_xvqq_sfs = new QCDPNetXVqqSFs();
    Cut* allmerged_apply_qcd_sfs = new LambdaCut(
        "AllMerged_ApplyQCDSFs",
        [&]()
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            if (file_name.Contains("QCD"))
            {
                LorentzVector hbbfatjet_p4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
                LorentzVector ld_vqqfatjet_p4 = cutflow.globals.getVal<LorentzVector>("ld_vqqfatjet_p4");
                LorentzVector tr_vqqfatjet_p4 = cutflow.globals.getVal<LorentzVector>("tr_vqqfatjet_p4");
                double hbbfatjet_score = arbol.getLeaf<double>("hbbfatjet_score");
                double ld_vqqfatjet_score = arbol.getLeaf<double>("ld_vqqfatjet_score");
                double tr_vqqfatjet_score = arbol.getLeaf<double>("tr_vqqfatjet_score");
                arbol.setLeaf<double>(
                    "qcd_xbb_sf", 
                    qcd_xbb_sfs->getSF(hbbfatjet_p4.pt(), hbbfatjet_p4.eta(), hbbfatjet_score)
                );
                arbol.setLeaf<double>(
                    "qcd_xvqq_sf", 
                    (qcd_xvqq_sfs->getSF(ld_vqqfatjet_p4.pt(), ld_vqqfatjet_p4.eta(), ld_vqqfatjet_score)
                     *qcd_xvqq_sfs->getSF(tr_vqqfatjet_p4.pt(), tr_vqqfatjet_p4.eta(), tr_vqqfatjet_score))
                );
            }
            else
            {
                arbol.setLeaf<double>("qcd_xbb_sf", 1.);
                arbol.setLeaf<double>("qcd_xvqq_sf", 1.);
            }
            return true;
        }
    );
    cutflow.insert("AllMerged_SelectVVHFatJets", allmerged_apply_qcd_sfs, Right);
    */

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
            // qcd_xbb_sfs->init(file_name);
            // qcd_xvqq_sfs->init(file_name);
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
    // cutflow.writeHists(arbol.tfile);
    arbol.write();
    return 0;
}
