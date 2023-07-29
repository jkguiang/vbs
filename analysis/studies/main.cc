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
    arbol.newBranch<double>("reweight_c2v_eq_3", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    // analysis.initCorrections();
    analysis.initCutflow();

    arbol.newBranch<double>("ld_fatjet_xbb", -999);
    arbol.newBranch<double>("ld_fatjet_xwqq", -999);
    arbol.newBranch<double>("ld_fatjet_xvqq", -999);
    arbol.newBranch<double>("ld_fatjet_pt", -999);
    arbol.newBranch<double>("ld_fatjet_eta", -999);
    arbol.newBranch<double>("ld_fatjet_phi", -999);
    arbol.newBranch<double>("ld_fatjet_mass", -999);
    arbol.newBranch<double>("ld_fatjet_msoftdrop", -999);
    // arbol.newBranch<double>("md_fatjet_xbb", -999);
    // arbol.newBranch<double>("md_fatjet_xwqq", -999);
    // arbol.newBranch<double>("md_fatjet_xvqq", -999);
    // arbol.newBranch<double>("md_fatjet_pt", -999);
    // arbol.newBranch<double>("md_fatjet_eta", -999);
    // arbol.newBranch<double>("md_fatjet_phi", -999);
    // arbol.newBranch<double>("md_fatjet_mass", -999);
    // arbol.newBranch<double>("md_fatjet_msoftdrop", -999);
    arbol.newBranch<double>("tr_fatjet_xbb", -999);
    arbol.newBranch<double>("tr_fatjet_xwqq", -999);
    arbol.newBranch<double>("tr_fatjet_xvqq", -999);
    arbol.newBranch<double>("tr_fatjet_pt", -999);
    arbol.newBranch<double>("tr_fatjet_eta", -999);
    arbol.newBranch<double>("tr_fatjet_phi", -999);
    arbol.newBranch<double>("tr_fatjet_mass", -999);
    arbol.newBranch<double>("tr_fatjet_msoftdrop", -999);

    // adding branches for compining the two vqq ak4 jets four vectors
    arbol.newBranch<double>("vqqjets_pt", -999);
    arbol.newBranch<double>("vqqjets_phi", -999);
    arbol.newBranch<double>("vqqjets_mass", -999);
    arbol.newBranch<double>("vqqjets_eta", -999);

    arbol.newBranch<int>("bQuarksInHiggsJet", -999);




    // combining the two vqq ak4 jets four vectors
    Cut* addition_vqqjets = new LambdaCut(
        "Addition_Vqqjets",
        [&]()
        {
            // combining the two vqq ak4 jets four vectors
            LorentzVector ld_vqqjet_p4 = cutflow.globals.getVal<LorentzVector>("ld_vqqjet_p4");
            LorentzVector tr_vqqjet_p4 = cutflow.globals.getVal<LorentzVector>("tr_vqqjet_p4");
            // adding leaves for compining the two vqq ak4 jets four vectors
            arbol.setLeaf<double>("vqqjets_pt",(ld_vqqjet_p4 + tr_vqqjet_p4).pt());
            arbol.setLeaf<double>("vqqjets_phi",(ld_vqqjet_p4 + tr_vqqjet_p4).phi());
            arbol.setLeaf<double>("vqqjets_mass",(ld_vqqjet_p4 + tr_vqqjet_p4).mass());
            arbol.setLeaf<double>("vqqjets_eta",(ld_vqqjet_p4 + tr_vqqjet_p4).eta());
            return true;
        }
    );
    cutflow.insert("SemiMerged_SaveVariables", addition_vqqjets, Right);

        // truth tagging
    Cut* hbbfatjet_n_true_higgsbquark = new LambdaCut(
        "Hbbfatjet_n_true_higgsbquark",
        [&]()
        {

            LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
            std::vector<LorentzVector> bQuarks;
            int bQuarksInHiggsJet = 0;
            for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
            {
                int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
                if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
                int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
                if (abs(gen_pdgId)==5 && gen_mother_pdgId==25)
                {
                    bQuarks.push_back(nt.GenPart_p4().at(gen_i));
                }
            }
            for (unsigned int i=0; i < bQuarks.size(); i++)
            {
                double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,bQuarks.at(i));
                if (deltaR<0.8)
                {
                    bQuarksInHiggsJet++;
                }
            }
            arbol.setLeaf<int>("bQuarksInHiggsJet",bQuarksInHiggsJet);
            return true;


        }
    );
    cutflow.insert("SemiMerged_SaveVariables", hbbfatjet_n_true_higgsbquark, Right);



    // ld_fatjet_pt
    //
    //
    //
    //         return true;
    //     }
    // );
    // cutflow.insert("SemiMerged_SelectVVHFatJets", set_ptsorted_fatjets, Right);

    // if (cli.variation != "nofix")
    // {
    //     TFile* pnet_pdf_file = new TFile("data/vbsvvhjets_sfs/qcd_pnet_pdfs.root");
    //     TH2D* xbb_pdf2D = (TH2D*) pnet_pdf_file->Get("ParticleNet_Xbb_PDF_2D");
    //     TH3D* xvqq_pdf3D = (TH3D*) pnet_pdf_file->Get("ParticleNet_XVqq_PDF_3Dalt");
    //     TH3D* xwqq_pdf3D = (TH3D*) pnet_pdf_file->Get("ParticleNet_XWqq_PDF_3Dalt");
    //     Cut* replace_pnets = new LambdaCut(
    //         "AllMerged_ReplacePNetsQCD",
    //         [&, xbb_pdf2D, xvqq_pdf3D, xwqq_pdf3D]()
    //         {
    //             TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
    //             if (file_name.Contains("QCD"))
    //             {
    //                 LorentzVectors fatjet_p4s = cutflow.globals.getVal<LorentzVectors>("good_fatjet_p4s");
    //                 Doubles fatjet_xbbs;
    //                 Doubles fatjet_xvqqs;
    //                 Doubles fatjet_xwqqs;
    //                 int xbin;
    //                 int ybin;
    //                 for (auto fatjet_p4 : fatjet_p4s)
    //                 {
    //                     double fatjet_pt = fatjet_p4.pt();
    //                     // Generate Hbb fat jet score
    //                     xbin = xbb_pdf2D->GetXaxis()->FindBin(fatjet_pt);
    //                     TH1D* xbb_pdf1D = xbb_pdf2D->ProjectionY("xbb_projy", xbin, xbin);
    //                     xbb_pdf1D->Rebin(10);
    //                     double xbb = xbb_pdf1D->GetRandom();
    //                     // Generate XVqq fat jet score
    //                     xbin = xvqq_pdf3D->GetXaxis()->FindBin(fatjet_pt);
    //                     ybin = xvqq_pdf3D->GetYaxis()->FindBin(xbb);
    //                     TH1D* xvqq_pdf1D = xvqq_pdf3D->ProjectionZ("xvqq_projz", xbin, xbin, ybin, ybin);
    //                     xvqq_pdf1D->Rebin(10);
    //                     double xvqq = xvqq_pdf1D->GetRandom();
    //                     // Generate XWqq fat jet score
    //                     xbin = xwqq_pdf3D->GetXaxis()->FindBin(fatjet_pt);
    //                     ybin = xwqq_pdf3D->GetYaxis()->FindBin(xbb);
    //                     TH1D* xwqq_pdf1D = xwqq_pdf3D->ProjectionZ("xwqq_projz", xbin, xbin, ybin, ybin);
    //                     xwqq_pdf1D->Rebin(10);
    //                     double xwqq = xwqq_pdf1D->GetRandom();
    //
    //                     fatjet_xbbs.push_back(xbb);
    //                     fatjet_xvqqs.push_back(xvqq);
    //                     fatjet_xwqqs.push_back(xwqq);
    //                 }
    //                 cutflow.globals.setVal<Doubles>("good_fatjet_xbbtags", fatjet_xbbs);
    //                 cutflow.globals.setVal<Doubles>("good_fatjet_xvqqtags", fatjet_xvqqs);
    //                 cutflow.globals.setVal<Doubles>("good_fatjet_xwqqtags", fatjet_xwqqs);
    //             }
    //             return true;
    //         }
    //     );
    //     cutflow.insert("Exactly2FatJets", replace_pnets, Right);
    // }

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
                    "SemiMerged_SaveVariables"
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
    arbol.write();
    return 0;
}
