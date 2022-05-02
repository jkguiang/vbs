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
    // Event branches
    arbol.newBranch<float>("H_pt", -999);
    arbol.newBranch<float>("H_eta", -999);
    arbol.newBranch<float>("H_phi", -999);
    arbol.newBranch<float>("W_pt", -999);
    arbol.newBranch<float>("W_eta", -999);
    arbol.newBranch<float>("W_phi", -999);
    arbol.newBranch<float>("W_sign", -999);
    arbol.newBranch<float>("W_pol", -999);
    arbol.newBranch<float>("M_WH", -999);
    arbol.newBranch<float>("ld_VBS_pt", -999);
    arbol.newBranch<float>("tr_VBS_pt", -999);
    arbol.newBranch<float>("M_jj", -999);
    arbol.newBranch<float>("deta_jj", -999);

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
            // W boson
            LorentzVector W_p4;
            W_p4.SetPxPyPzE(lhe.P_X.At(0), lhe.P_Y.At(0), lhe.P_Z.At(0), lhe.E.At(0));
            int W_pid = lhe.PID.At(0);
            arbol.setLeaf<float>("W_pt", W_p4.pt());
            arbol.setLeaf<float>("W_eta", W_p4.eta());
            arbol.setLeaf<float>("W_phi", W_p4.phi());
            arbol.setLeaf<float>("W_sign", (W_pid > 0) - (W_pid < 0));
            arbol.setLeaf<float>("W_pol", lhe.polarization.At(0));
            // H boson
            LorentzVector H_p4;
            H_p4.SetPxPyPzE(lhe.P_X.At(1), lhe.P_Y.At(1), lhe.P_Z.At(1), lhe.E.At(1));
            arbol.setLeaf<float>("H_pt", H_p4.pt());
            arbol.setLeaf<float>("H_eta", H_p4.eta());
            arbol.setLeaf<float>("H_phi", H_p4.phi());
            // WH system
            arbol.setLeaf<float>("M_WH", (W_p4 + H_p4).mass());
            // VBS jets
            LorentzVector VBS1_p4;
            VBS1_p4.SetPxPyPzE(lhe.P_X.At(2), lhe.P_Y.At(2), lhe.P_Z.At(2), lhe.E.At(2));
            LorentzVector VBS2_p4;
            VBS2_p4.SetPxPyPzE(lhe.P_X.At(3), lhe.P_Y.At(3), lhe.P_Z.At(3), lhe.E.At(3));
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
            bool passed = cutflow.runUntil("SetFlatVariables");
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
