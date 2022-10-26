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

typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> LorentzVector;

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize LHE (converted to ROOT) reader
    LesHouche lhe;

    // Initialize Looper
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);
    // Event branches
    arbol.newBranch<double>("ld_Q_pz", -999);
    arbol.newBranch<double>("tr_Q_pz", -999);
    arbol.newBranch<double>("ld_Q_E", -999);
    arbol.newBranch<double>("tr_Q_E", -999);
    arbol.newBranch<double>("H_pt", -999);
    arbol.newBranch<double>("H_eta", -999);
    arbol.newBranch<double>("H_phi", -999);
    arbol.newBranch<double>("W_pt", -999);
    arbol.newBranch<double>("W_eta", -999);
    arbol.newBranch<double>("W_phi", -999);
    arbol.newBranch<double>("W_sign", -999);
    arbol.newBranch<double>("W_pol", -999);
    arbol.newBranch<double>("M_WH", -999);
    arbol.newBranch<double>("ld_VBS_pt", -999);
    arbol.newBranch<double>("tr_VBS_pt", -999);
    arbol.newBranch<double>("M_jj", -999);
    arbol.newBranch<double>("deta_jj", -999);
    arbol.newBranch<double>("ST", -999);

    // Other trees
    TList* beam_ttrees = new TList();
    TList* processes_ttrees = new TList();

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
            /*
                ***********************************************************************
                *    Row   * Instance *      IDUP *       P_X *       P_Y *       P_Z *
                ***********************************************************************
                *        0 *        0 *         2 *         0 *         0 * 2357.0878 *
                *        0 *        1 *         2 *        -0 *        -0 * -1026.484 *
                *        0 *        2 *        24 * -295.5993 * 170.60359 * 120.55848 *
                *        0 *        3 *        25 *  447.0713 * -72.30899 * -558.5735 *
                *        0 *        4 *         2 * 6.1413710 * 44.818886 * -249.5060 *
                *        0 *        5 *         1 * -157.6133 * -143.1134 * 2018.1248 *
                *        1 *        0 *         1 *         0 *         0 * 601.18000 *
                *        1 *        1 *         1 *        -0 *        -0 * -1823.473 *
                *        1 *        2 *       -24 * 393.32631 * 414.61886 * -505.6129 *
                *        1 *        3 *        25 * -341.9177 * -392.6790 * -129.0019 *
                *        1 *        4 *         2 * -72.27269 * 101.08511 * 239.61820 *
                *        1 *        5 *         1 * 20.864125 * -123.0248 * -827.2972 *
                *        2 *        0 *         2 *         0 *         0 * 1939.6407 *
                *      ... *      ... *       ... *       ... *       ... *       ... *
            */

            // Incoming partons
            double q1_pz = lhe.P_Z.At(0);
            double q2_pz = lhe.P_Z.At(1);
            double q1_E = lhe.E.At(0); // should be == fabs(pz)
            double q2_E = lhe.E.At(1); // should be == fabs(pz)
            arbol.setLeaf<double>("ld_Q_pz", (q1_pz > q2_pz) ? q1_pz : q2_pz);
            arbol.setLeaf<double>("tr_Q_pz", (q1_pz > q2_pz) ? q2_pz : q1_pz);
            arbol.setLeaf<double>("ld_Q_E", (q1_pz > q2_pz) ? q1_E : q2_E);
            arbol.setLeaf<double>("tr_Q_E", (q1_pz > q2_pz) ? q2_E : q1_E);
            // W boson
            LorentzVector W_p4;
            W_p4.SetPxPyPzE(lhe.P_X.At(2), lhe.P_Y.At(2), lhe.P_Z.At(2), lhe.E.At(2));
            int W_pdgID = lhe.IDUP.At(2);
            arbol.setLeaf<double>("W_pt", W_p4.pt());
            arbol.setLeaf<double>("W_eta", W_p4.eta());
            arbol.setLeaf<double>("W_phi", W_p4.phi());
            arbol.setLeaf<double>("W_sign", (W_pdgID > 0) - (W_pdgID < 0));
            arbol.setLeaf<double>("W_pol", lhe.SPINUP.At(2));
            // H boson
            LorentzVector H_p4;
            H_p4.SetPxPyPzE(lhe.P_X.At(3), lhe.P_Y.At(3), lhe.P_Z.At(3), lhe.E.At(3));
            arbol.setLeaf<double>("H_pt", H_p4.pt());
            arbol.setLeaf<double>("H_eta", H_p4.eta());
            arbol.setLeaf<double>("H_phi", H_p4.phi());
            // WH system
            arbol.setLeaf<double>("M_WH", (W_p4 + H_p4).mass());
            // VBS jets
            LorentzVector VBS1_p4;
            VBS1_p4.SetPxPyPzE(lhe.P_X.At(4), lhe.P_Y.At(4), lhe.P_Z.At(4), lhe.E.At(4));
            LorentzVector VBS2_p4;
            VBS2_p4.SetPxPyPzE(lhe.P_X.At(5), lhe.P_Y.At(5), lhe.P_Z.At(5), lhe.E.At(5));
            if (VBS1_p4.pt() > VBS2_p4.pt())
            {
                arbol.setLeaf<double>("ld_VBS_pt", VBS1_p4.pt());
                arbol.setLeaf<double>("tr_VBS_pt", VBS2_p4.pt());
            }
            else
            {
                arbol.setLeaf<double>("ld_VBS_pt", VBS2_p4.pt());
                arbol.setLeaf<double>("tr_VBS_pt", VBS1_p4.pt());
            }
            arbol.setLeaf<double>("M_jj", (VBS1_p4 + VBS2_p4).mass());
            arbol.setLeaf<double>("deta_jj", VBS1_p4.eta() - VBS2_p4.eta());
            // Other
            arbol.setLeaf<double>("ST", W_p4.pt() + H_p4.pt());
            return true;
        }
    );
    cutflow.insert("Bookkeeping", cut, Right);

    // Run looper
    looper.run(
        [&](TTree* ttree) 
        { 
            lhe.Init(ttree); 
            // Store metadata ttree
            TTree* beam_ttree = ((TTree*)ttree->GetCurrentFile()->Get("Beam"))->CloneTree();
            beam_ttree->SetDirectory(0);
            beam_ttrees->Add(beam_ttree);
            TTree* processes_ttree = ((TTree*)ttree->GetCurrentFile()->Get("Processes"))->CloneTree();
            processes_ttree->SetDirectory(0);
            processes_ttrees->Add(processes_ttree);
        },
        [&](int entry) 
        {
            lhe.GetEntry(entry);
            lhe.Process(entry);
            // Reset tree
            arbol.resetBranches();
            // Run cutflow
            bool passed = cutflow.run("SetFlatVariables");
            if (passed) { arbol.fill(); }
            return;
        }
    );

    // Wrap up
    TTree* merged_beam_ttrees = TTree::MergeTrees(beam_ttrees);
    merged_beam_ttrees->SetName("Beam");
    TTree* merged_processes_ttrees = TTree::MergeTrees(processes_ttrees);
    merged_processes_ttrees->SetName("Processes");

    cutflow.print();

    arbol.tfile->cd();
    merged_beam_ttrees->Write();
    merged_processes_ttrees->Write();
    arbol.write();
    return 0;
}
