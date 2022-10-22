// RAPIDO
#include "LesHouche.h"
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "Math/LorentzVector.h"
#include "Math/GenVector/PtEtaPhiM4D.h"
#include "stdio.h"

typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<float>> LorentzVector;

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    LesHouche lhe;
    Looper looper = Looper(cli.input_tchain);

    // Initialize Arbol
    TFile* output_tfile = new TFile(
        TString(cli.output_dir+"/"+cli.output_name+".root"),
        "RECREATE"
    );
    Arbol arbol = Arbol(output_tfile);
    // Higgs
    arbol.newBranch<double>("H_pt", -999);
    arbol.newBranch<double>("H_eta", -999);
    arbol.newBranch<double>("H_phi", -999);
    // Leading W
    arbol.newBranch<double>("ld_W_pt", -999);
    arbol.newBranch<double>("ld_W_eta", -999);
    arbol.newBranch<double>("ld_W_phi", -999);
    arbol.newBranch<int>("ld_W_sign", -999);
    arbol.newBranch<double>("ld_W_pol", -999);
    // Trailing W
    arbol.newBranch<double>("tr_W_pt", -999);
    arbol.newBranch<double>("tr_W_eta", -999);
    arbol.newBranch<double>("tr_W_phi", -999);
    arbol.newBranch<int>("tr_W_sign", -999);
    arbol.newBranch<double>("tr_W_pol", -999);
    // Higgs + W + W system
    arbol.newBranch<double>("M_WWH", -999);
    // VBS jets
    arbol.newBranch<double>("ld_VBS_pt", -999);
    arbol.newBranch<double>("tr_VBS_pt", -999);
    arbol.newBranch<double>("M_jj", -999);
    arbol.newBranch<double>("deta_jj", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow();

    Cut* root = new LambdaCut(
        "Bookkeeping",
        [&]()
        {
            return true;
        }
    );
    cutflow.setRoot(root);

    Cut* cut = new LambdaCut(
        "SetFlatVariables",
        [&]()
        {
            // W bosons
            LorentzVector W1_p4;
            W1_p4.SetPxPyPzE(lhe.P_X.At(2), lhe.P_Y.At(2), lhe.P_Z.At(2), lhe.E.At(2));
            LorentzVector W2_p4;
            W2_p4.SetPxPyPzE(lhe.P_X.At(3), lhe.P_Y.At(3), lhe.P_Z.At(3), lhe.E.At(3));
            int W1_pid = lhe.IDUP.At(2);
            double W1_pol = lhe.SPINUP.At(2);
            int W2_pid = lhe.IDUP.At(3);
            double W2_pol = lhe.SPINUP.At(3);
            if (W1_p4.pt() > W2_p4.pt())
            {
                arbol.setLeaf<double>("ld_W_pt", W1_p4.pt());
                arbol.setLeaf<double>("ld_W_eta", W1_p4.eta());
                arbol.setLeaf<double>("ld_W_phi", W1_p4.phi());
                arbol.setLeaf<int>("ld_W_sign", (W1_pid > 0) - (W1_pid < 0));
                arbol.setLeaf<double>("ld_W_pol", W1_pol);
                arbol.setLeaf<double>("tr_W_pt", W2_p4.pt());
                arbol.setLeaf<double>("tr_W_eta", W2_p4.eta());
                arbol.setLeaf<double>("tr_W_phi", W2_p4.phi());
                arbol.setLeaf<int>("tr_W_sign", (W2_pid > 0) - (W2_pid < 0));
                arbol.setLeaf<double>("tr_W_pol", W2_pol);
            }
            else
            {
                arbol.setLeaf<double>("ld_W_pt", W2_p4.pt());
                arbol.setLeaf<double>("ld_W_eta", W2_p4.eta());
                arbol.setLeaf<double>("ld_W_phi", W2_p4.phi());
                arbol.setLeaf<int>("ld_W_sign", (W2_pid > 0) - (W2_pid < 0));
                arbol.setLeaf<double>("ld_W_pol", W2_pol);
                arbol.setLeaf<double>("tr_W_pt", W1_p4.pt());
                arbol.setLeaf<double>("tr_W_eta", W1_p4.eta());
                arbol.setLeaf<double>("tr_W_phi", W1_p4.phi());
                arbol.setLeaf<int>("tr_W_sign", (W1_pid > 0) - (W1_pid < 0));
                arbol.setLeaf<double>("tr_W_pol", W1_pol);
            }
            // H boson
            LorentzVector H_p4;
            H_p4.SetPxPyPzE(lhe.P_X.At(4), lhe.P_Y.At(4), lhe.P_Z.At(4), lhe.E.At(4));
            arbol.setLeaf<float>("H_pt", H_p4.pt());
            arbol.setLeaf<float>("H_eta", H_p4.eta());
            arbol.setLeaf<float>("H_phi", H_p4.phi());
            // WH system
            arbol.setLeaf<float>("M_WWH", (W1_p4 + W2_p4 + H_p4).mass());
            // VBS jets
            LorentzVector VBS1_p4;
            VBS1_p4.SetPxPyPzE(lhe.P_X.At(5), lhe.P_Y.At(5), lhe.P_Z.At(5), lhe.E.At(5));
            LorentzVector VBS2_p4;
            VBS2_p4.SetPxPyPzE(lhe.P_X.At(6), lhe.P_Y.At(6), lhe.P_Z.At(6), lhe.E.At(6));
            if (VBS1_p4.pt() > VBS2_p4.pt())
            {
                arbol.setLeaf<float>("ld_VBS_pt", VBS1_p4.pt());
                arbol.setLeaf<float>("tr_VBS_pt", VBS2_p4.pt());
            }
            else
            {
                arbol.setLeaf<float>("ld_VBS_pt", VBS2_p4.pt());
                arbol.setLeaf<float>("tr_VBS_pt", VBS1_p4.pt());
            }
            arbol.setLeaf<float>("M_jj", (VBS1_p4 + VBS2_p4).mass());
            arbol.setLeaf<float>("deta_jj", VBS1_p4.eta() - VBS2_p4.eta());
            return true;
        }
    );
    cutflow.insert("Bookkeeping", cut, Right);

    // Run looper
    looper.run(
        [&](TTree* ttree) { lhe.Init(ttree); },
        [&](int entry) 
        {
            lhe.GetEntry(entry);
            lhe.Process(entry);
            // Reset tree
            arbol.resetBranches();
            // Run cutflow
            bool passed = cutflow.run("SetFlatVariables");
            if (passed) { arbol.fillTTree(); }
            return;
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.writeCSV(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
