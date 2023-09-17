#include "vbsvvhjets/collections.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "cutflow.h"
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
    arbol.newBranch<double>("reweights", -999);

    // Initialize Arbol for PDF variations
    Arbol pdf_arbol = Arbol(
        cli.output_dir+"/"+cli.output_name+"_pdf.root",
        "pdf_"+cli.output_ttree
    );
    for (int i = 0; i < 101; ++i)
    {
        pdf_arbol.newBranch<double>("lhe_pdf_"+std::to_string(i), -999);
    }
    pdf_arbol.newBranch<double>("event_weight", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCorrections();
    analysis.initCutflow();

    arbol.newBranch<double>("ld_fatjet_xbb", -999);
    arbol.newBranch<double>("ld_fatjet_xwqq", -999);
    arbol.newBranch<double>("ld_fatjet_xvqq", -999);
    arbol.newBranch<double>("ld_fatjet_pt", -999);
    arbol.newBranch<double>("ld_fatjet_eta", -999);
    arbol.newBranch<double>("ld_fatjet_phi", -999);
    arbol.newBranch<double>("ld_fatjet_mass", -999);
    arbol.newBranch<double>("ld_fatjet_msoftdrop", -999);
    arbol.newBranch<double>("md_fatjet_xbb", -999);
    arbol.newBranch<double>("md_fatjet_xwqq", -999);
    arbol.newBranch<double>("md_fatjet_xvqq", -999);
    arbol.newBranch<double>("md_fatjet_pt", -999);
    arbol.newBranch<double>("md_fatjet_eta", -999);
    arbol.newBranch<double>("md_fatjet_phi", -999);
    arbol.newBranch<double>("md_fatjet_mass", -999);
    arbol.newBranch<double>("md_fatjet_msoftdrop", -999);
    arbol.newBranch<double>("tr_fatjet_xbb", -999);
    arbol.newBranch<double>("tr_fatjet_xwqq", -999);
    arbol.newBranch<double>("tr_fatjet_xvqq", -999);
    arbol.newBranch<double>("tr_fatjet_pt", -999);
    arbol.newBranch<double>("tr_fatjet_eta", -999);
    arbol.newBranch<double>("tr_fatjet_phi", -999);
    arbol.newBranch<double>("tr_fatjet_mass", -999);
    arbol.newBranch<double>("tr_fatjet_msoftdrop", -999);

    Cut* set_ptsorted_fatjets = new LambdaCut(
        "AllMerged_SetPtSortedFatJetVariables",
        [&]() 
        {
            LorentzVectors fatjet_p4s = cutflow.globals.getVal<LorentzVectors>("good_fatjet_p4s");
            Doubles fatjet_xbbs = cutflow.globals.getVal<Doubles>("good_fatjet_xbbtags");
            Doubles fatjet_xvqqs = cutflow.globals.getVal<Doubles>("good_fatjet_xvqqtags");
            Doubles fatjet_xwqqs = cutflow.globals.getVal<Doubles>("good_fatjet_xwqqtags");
            Doubles fatjet_masses = cutflow.globals.getVal<Doubles>("good_fatjet_masses");
            Doubles fatjet_msoftdrops = cutflow.globals.getVal<Doubles>("good_fatjet_msoftdrops");
            std::vector<unsigned int> vvh_gidx;
            vvh_gidx.push_back(cutflow.globals.getVal<unsigned int>("ld_vqqfatjet_gidx"));
            vvh_gidx.push_back(cutflow.globals.getVal<unsigned int>("tr_vqqfatjet_gidx"));
            vvh_gidx.push_back(cutflow.globals.getVal<unsigned int>("hbbfatjet_gidx"));
            std::sort(
                vvh_gidx.begin(), vvh_gidx.end(), 
                [&](unsigned int gidx1, unsigned int gidx2)
                {
                    return fatjet_p4s.at(gidx1).pt() > fatjet_p4s.at(gidx2).pt();
                }
            );
            unsigned int ld_gidx = vvh_gidx.at(0); // leading
            unsigned int md_gidx = vvh_gidx.at(1); // middling
            unsigned int tr_gidx = vvh_gidx.at(2); // trailing
            arbol.setLeaf<double>("ld_fatjet_xbb", fatjet_xbbs.at(ld_gidx));
            arbol.setLeaf<double>("ld_fatjet_xwqq", fatjet_xwqqs.at(ld_gidx));
            arbol.setLeaf<double>("ld_fatjet_xvqq", fatjet_xvqqs.at(ld_gidx));
            arbol.setLeaf<double>("ld_fatjet_pt", fatjet_p4s.at(ld_gidx).pt());
            arbol.setLeaf<double>("ld_fatjet_eta", fatjet_p4s.at(ld_gidx).eta());
            arbol.setLeaf<double>("ld_fatjet_phi", fatjet_p4s.at(ld_gidx).phi());
            arbol.setLeaf<double>("ld_fatjet_mass", fatjet_masses.at(ld_gidx));
            arbol.setLeaf<double>("ld_fatjet_msoftdrop", fatjet_msoftdrops.at(ld_gidx));
            arbol.setLeaf<double>("md_fatjet_xbb", fatjet_xbbs.at(md_gidx));
            arbol.setLeaf<double>("md_fatjet_xwqq", fatjet_xwqqs.at(md_gidx));
            arbol.setLeaf<double>("md_fatjet_xvqq", fatjet_xvqqs.at(md_gidx));
            arbol.setLeaf<double>("md_fatjet_pt", fatjet_p4s.at(md_gidx).pt());
            arbol.setLeaf<double>("md_fatjet_eta", fatjet_p4s.at(md_gidx).eta());
            arbol.setLeaf<double>("md_fatjet_phi", fatjet_p4s.at(md_gidx).phi());
            arbol.setLeaf<double>("md_fatjet_mass", fatjet_masses.at(md_gidx));
            arbol.setLeaf<double>("md_fatjet_msoftdrop", fatjet_msoftdrops.at(md_gidx));
            arbol.setLeaf<double>("tr_fatjet_xbb", fatjet_xbbs.at(tr_gidx));
            arbol.setLeaf<double>("tr_fatjet_xwqq", fatjet_xwqqs.at(tr_gidx));
            arbol.setLeaf<double>("tr_fatjet_xvqq", fatjet_xvqqs.at(tr_gidx));
            arbol.setLeaf<double>("tr_fatjet_pt", fatjet_p4s.at(tr_gidx).pt());
            arbol.setLeaf<double>("tr_fatjet_eta", fatjet_p4s.at(tr_gidx).eta());
            arbol.setLeaf<double>("tr_fatjet_phi", fatjet_p4s.at(tr_gidx).phi());
            arbol.setLeaf<double>("tr_fatjet_mass", fatjet_masses.at(tr_gidx));
            arbol.setLeaf<double>("tr_fatjet_msoftdrop", fatjet_msoftdrops.at(tr_gidx));
            return true;
        }
    );
    cutflow.insert("AllMerged_SelectVVHFatJets", set_ptsorted_fatjets, Right);

    if (cli.variation != "nofix")
    {
        TFile* pnet_pdf_file = new TFile("data/vbsvvhjets_sfs/qcd_pnet_pdfs.root");
        TH2D* xbb_pdf2D = (TH2D*) pnet_pdf_file->Get("ParticleNet_Xbb_PDF_2D");
        TH3D* xvqq_pdf3D = (TH3D*) pnet_pdf_file->Get("ParticleNet_XVqq_PDF_3Dalt");
        TH3D* xwqq_pdf3D = (TH3D*) pnet_pdf_file->Get("ParticleNet_XWqq_PDF_3Dalt");
        Cut* replace_pnets = new LambdaCut(
            "AllMerged_ReplacePNetsQCD",
            [&, xbb_pdf2D, xvqq_pdf3D, xwqq_pdf3D]()
            {
                TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
                if (file_name.Contains("QCD"))
                {
                    LorentzVectors fatjet_p4s = cutflow.globals.getVal<LorentzVectors>("good_fatjet_p4s");
                    Doubles fatjet_xbbs;
                    Doubles fatjet_xvqqs;
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
                        // Generate XVqq fat jet score
                        xbin = xvqq_pdf3D->GetXaxis()->FindBin(fatjet_pt);
                        ybin = xvqq_pdf3D->GetYaxis()->FindBin(xbb);
                        TH1D* xvqq_pdf1D = xvqq_pdf3D->ProjectionZ("xvqq_projz", xbin, xbin, ybin, ybin);
                        xvqq_pdf1D->Rebin(10);
                        double xvqq = xvqq_pdf1D->GetRandom();
                        // Generate XWqq fat jet score
                        xbin = xwqq_pdf3D->GetXaxis()->FindBin(fatjet_pt);
                        ybin = xwqq_pdf3D->GetYaxis()->FindBin(xbb);
                        TH1D* xwqq_pdf1D = xwqq_pdf3D->ProjectionZ("xwqq_projz", xbin, xbin, ybin, ybin);
                        xwqq_pdf1D->Rebin(10);
                        double xwqq = xwqq_pdf1D->GetRandom();

                        fatjet_xbbs.push_back(xbb);
                        fatjet_xvqqs.push_back(xvqq);
                        fatjet_xwqqs.push_back(xwqq);
                    }
                    cutflow.globals.setVal<Doubles>("good_fatjet_xbbtags", fatjet_xbbs);
                    cutflow.globals.setVal<Doubles>("good_fatjet_xvqqtags", fatjet_xvqqs);
                    cutflow.globals.setVal<Doubles>("good_fatjet_xwqqtags", fatjet_xwqqs);
                }
                return true;
            }
        );
        cutflow.insert("Geq3FatJets", replace_pnets, Right);
    }

    Cut* save_pdfweights = new LambdaCut(
        "AllMerged_SavePDFWeights",
        [&]()
        {
            if (nt.isData()) { return true; }
            for (int i = 0; i < 101; ++i)
            {
                TString branch_name = "lhe_pdf_"+std::to_string(i);
                if (nt.nLHEPdfWeight() >= 101)
                {
                    pdf_arbol.setLeaf<double>(branch_name, nt.LHEPdfWeight().at(i));
                }
                else
                {
                    pdf_arbol.setLeaf<double>(branch_name, 1.);
                }
            }
            return true;
        }
    );
    cutflow.insert("AllMerged_SaveVariables", save_pdfweights, Right);

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
                pdf_arbol.resetBranches();
                cutflow.globals.resetVars();

                nt.GetEntry(entry);

                // Run cutflow
                std::vector<std::string> cuts_to_check = {
                    // "AllMerged_SaveVariables"
                    "AllMerged_SavePDFWeights"
                };
                std::vector<bool> checkpoints = cutflow.run(cuts_to_check);
                if (checkpoints.at(0)) 
                { 
                    arbol.fill(); 
                    pdf_arbol.fill(); 
                }

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
    arbol.write();
    pdf_arbol.write();
    return 0;
}
