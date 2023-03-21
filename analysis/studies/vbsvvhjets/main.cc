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

    // Initialize Cutflow
    Histflow cutflow = Histflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCorrections();
    analysis.initCutflow();

    TFile* pnet_pdf_file = new TFile("data/vbsvvhjets_sfs/qcd_pnet_pdfs.root");
    TH2D* xbb_pdf2D = (TH2D*) pnet_pdf_file->Get("ParticleNet_Xbb_PDF_2D");
    TH3D* xwqq_pdf3D = (TH3D*) pnet_pdf_file->Get("ParticleNet_XWqq_PDF_3Dalt");
    Cut* replace_pnets = new LambdaCut(
        "ReplacePNetsQCD",
        [&, xbb_pdf2D, xwqq_pdf3D]()
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            if (file_name.Contains("QCD"))
            {
                LorentzVectors fatjet_p4s = cutflow.globals.getVal<LorentzVectors>("good_fatjet_p4s");
                Doubles fatjet_xbbs;
                Doubles fatjet_xwqqs;
                int xbin;
                int ybin;
                for (auto fatjet_p4 : fatjet_p4s)
                {
                    double fatjet_pt = fatjet_p4.pt();
                    // Generate Hbb fat jet score
                    xbin = xbb_pdf2D->GetXaxis()->FindBin(fatjet_pt);
                    TH1D* xbb_pdf1D = xbb_pdf2D->ProjectionY("xbb_projy", xbin, xbin);
                    xbb_pdf1D->Rebin(10);
                    double xbb = xbb_pdf1D->GetRandom();
                    // Generate XWqq fat jet score
                    xbin = xwqq_pdf3D->GetXaxis()->FindBin(fatjet_pt);
                    ybin = xwqq_pdf3D->GetYaxis()->FindBin(xbb);
                    TH1D* xwqq_pdf1D = xwqq_pdf3D->ProjectionZ("xwqq_projz", xbin, xbin, ybin, ybin);
                    xwqq_pdf1D->Rebin(10);
                    double xwqq = xwqq_pdf1D->GetRandom();

                    fatjet_xbbs.push_back(xbb);
                    fatjet_xwqqs.push_back(xwqq);
                }
                cutflow.globals.setVal<Doubles>("good_fatjet_xbbtags", fatjet_xbbs);
                cutflow.globals.setVal<Doubles>("good_fatjet_xwqqtags", fatjet_xwqqs);
            }
            return true;
        }
    );
    cutflow.insert("Geq3FatJets", replace_pnets, Right);

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
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
