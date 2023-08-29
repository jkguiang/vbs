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

#include "MCTools.h"

// // function to dump LHE Part
// void dumpLHEParticleInfos(std::vector<int> filter_pdgid)
// {
//     dumpLHEParticleInfoHeader();
//     for (unsigned int idx = 0; idx < LHEPart_pdgId.size(); ++idx)
//     {
//         int pdg_id = LHEPart_pdgId.at(idx);
//         if (filter_pdgid.size() != 0)
//             if (std::find(filter_pdgid.begin(), filter_pdgid.end(), pdg_id) == filter_pdgid.end())
//                 continue;
//         dumpLHEParticleInfo(idx);
//     }
// }
//
// void dumpLHEParticleInfoHeader()
// {
//     // Header
//     cout << "                  " << "   pt    " << "  phi  " << "      eta   " << "    mass  " << "status " << "spin  " << endl;
//     cout << "------------------------------------------------------------------------------------" << endl;
// }
//
// void dumpLHEParticleInfo(int idx)
// {
//     TDatabasePDG pdg;
//     int pdg_id = LHEPart_pdgId.at(idx);
//     float pt = LHEPart_pt.at(idx);
//     float eta = LHEPart_eta.at(idx);
//     float phi = LHEPart_phi.at(idx);
//     float mass = LHEPart_mass.at(idx);
//     int status = LHEPart_status.at(idx);
//     int spin = LHEPart_spin.at(idx);
//
//     const char* particle = (abs(pdg_id) == 4124) ? "Lambda_c*" : pdg.GetParticle(pdg_id)->GetName();
//
//     cout << setw(4)  << left  <<                    idx                      << " "
//          << setw(10) << left  <<                    particle                 << " "
//          << setw(7)  << right << setprecision(4) << pt                       << "  "
//          << setw(7)  << right << setprecision(4) << phi                      << "  "
//          << setw(10) << right << setprecision(4) << eta                      << "  "
//          << setw(10) << right << setprecision(4) << mass                     << "  "
//          << setw(4)  << right <<                    status                   << "  "
//          << setw(4)  << right <<                    spin                     << "  " << endl;
// }




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
    arbol.newBranch<int>("wBosonsInHiggsJet", -999);
    arbol.newBranch<int>("zBosonsInHiggsJet", -999);
    arbol.newBranch<int>("cQuarksInHiggsJet", -999);
    arbol.newBranch<int>("tLeptonsInHiggsJet", -999);
    arbol.newBranch<int>("gluonsInHiggsJet", -999);
    arbol.newBranch<int>("gammasInHiggsJet", -999);

    arbol.newBranch<double>("ld_w_ld_fermion_pt",-999);
    arbol.newBranch<double>("ld_w_ld_fermion_eta",-999);
    arbol.newBranch<double>("ld_w_ld_fermion_phi",-999);
    arbol.newBranch<double>("ld_w_ld_fermion_mass",-999);
    arbol.newBranch<double>("ld_w_tr_fermion_pt",-999);
    arbol.newBranch<double>("ld_w_tr_fermion_eta",-999);
    arbol.newBranch<double>("ld_w_tr_fermion_phi",-999);
    arbol.newBranch<double>("ld_w_tr_fermion_mass",-999);
    arbol.newBranch<double>("tr_w_ld_fermion_pt",-999);
    arbol.newBranch<double>("tr_w_ld_fermion_eta",-999);
    arbol.newBranch<double>("tr_w_ld_fermion_phi",-999);
    arbol.newBranch<double>("tr_w_ld_fermion_mass",-999);
    arbol.newBranch<double>("tr_w_tr_fermion_pt",-999);
    arbol.newBranch<double>("tr_w_tr_fermion_eta",-999);
    arbol.newBranch<double>("tr_w_tr_fermion_phi",-999);
    arbol.newBranch<double>("tr_w_tr_fermion_mass",-999);

    arbol.newBranch<double>("ld_w_ld_fermion_pdgId",-999);
    arbol.newBranch<double>("ld_w_tr_fermion_pdgId",-999);
    arbol.newBranch<double>("tr_w_ld_fermion_pdgId",-999);
    arbol.newBranch<double>("tr_w_tr_fermion_pdgId",-999);

    // gen level
    arbol.newBranch<double>("gen_ld_w_ld_fermion_pt",-999);
    arbol.newBranch<double>("gen_ld_w_ld_fermion_eta",-999);
    arbol.newBranch<double>("gen_ld_w_ld_fermion_phi",-999);
    arbol.newBranch<double>("gen_ld_w_ld_fermion_mass",-999);
    arbol.newBranch<double>("gen_ld_w_tr_fermion_pt",-999);
    arbol.newBranch<double>("gen_ld_w_tr_fermion_eta",-999);
    arbol.newBranch<double>("gen_ld_w_tr_fermion_phi",-999);
    arbol.newBranch<double>("gen_ld_w_tr_fermion_mass",-999);
    arbol.newBranch<double>("gen_tr_w_ld_fermion_pt",-999);
    arbol.newBranch<double>("gen_tr_w_ld_fermion_eta",-999);
    arbol.newBranch<double>("gen_tr_w_ld_fermion_phi",-999);
    arbol.newBranch<double>("gen_tr_w_ld_fermion_mass",-999);
    arbol.newBranch<double>("gen_tr_w_tr_fermion_pt",-999);
    arbol.newBranch<double>("gen_tr_w_tr_fermion_eta",-999);
    arbol.newBranch<double>("gen_tr_w_tr_fermion_phi",-999);
    arbol.newBranch<double>("gen_tr_w_tr_fermion_mass",-999);

    arbol.newBranch<double>("gen_ld_w_ld_fermion_pdgId",-999);
    arbol.newBranch<double>("gen_ld_w_tr_fermion_pdgId",-999);
    arbol.newBranch<double>("gen_tr_w_ld_fermion_pdgId",-999);
    arbol.newBranch<double>("gen_tr_w_tr_fermion_pdgId",-999);

    // Initialize branches for leading and trailing h_bb
    arbol.newBranch<double>("gen_ld_h_bb_pt", -999);
    arbol.newBranch<double>("gen_ld_h_bb_eta", -999);
    arbol.newBranch<double>("gen_ld_h_bb_phi", -999);
    arbol.newBranch<double>("gen_ld_h_bb_mass", -999);
    arbol.newBranch<double>("gen_tr_h_bb_pt", -999);
    arbol.newBranch<double>("gen_tr_h_bb_eta", -999);
    arbol.newBranch<double>("gen_tr_h_bb_phi", -999);
    arbol.newBranch<double>("gen_tr_h_bb_mass", -999);

    // Initialize branches for leading and trailing h_non_bb
    arbol.newBranch<double>("gen_ld_h_non_bb_pt", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_eta", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_phi", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_mass", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_pt", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_eta", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_phi", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_mass", -999);

    // Initialize branches for pdgId
    arbol.newBranch<double>("gen_ld_h_bb_pdgId", -999);
    arbol.newBranch<double>("gen_tr_h_bb_pdgId", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_pdgId", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_pdgId", -999);



    // Initialize branches for vbf_quarks
    arbol.newBranch<double>("gen_ld_vbf_quarks_pt", -999);
    arbol.newBranch<double>("gen_ld_vbf_quarks_eta", -999);
    arbol.newBranch<double>("gen_ld_vbf_quarks_phi", -999);
    arbol.newBranch<double>("gen_ld_vbf_quarks_mass", -999);

    arbol.newBranch<double>("gen_tr_vbf_quarks_pt", -999);
    arbol.newBranch<double>("gen_tr_vbf_quarks_eta", -999);
    arbol.newBranch<double>("gen_tr_vbf_quarks_phi", -999);
    arbol.newBranch<double>("gen_tr_vbf_quarks_mass", -999);

    // We know that on gen level: element 0,1 are the incoming quarks and 2,3,4 are the WWH, and 5,6 are VBF quarks. we are gonna use that knowledge to get the WWH decay products and collect them using some if then statements
    //
    Cut* allWWH_products_identification_genlevel = new LambdaCut(
        "AllWWH_products_identification_genlevel",
        [&]()
        {

            std::vector<LorentzVector> ww_p4;
            std::vector<LorentzVector> h_p4;
            std::vector<LorentzVector> vbf_quarks_p4;
            std::vector<int> ww_idx;
            std::vector<int> vbf_quarks_idx;

            int h_idx=0;
            for (unsigned int gen_i = 0; gen_i <nt.nGenPart() ; gen_i++)
            {
                int pdgId = nt.GenPart_pdgId().at(gen_i);
                // analyze the wwh bosons products
                if ((gen_i== 19 || gen_i==20 || gen_i==21) && nt.GenPart_status().at(gen_i)==62)
                {
                    if (abs(pdgId)==24)
                    {
                        ww_p4.push_back(nt.GenPart_p4().at(gen_i));
                        ww_idx.push_back(gen_i);
                    }

                    if (abs(pdgId)==25)
                    {
                        h_p4.push_back(nt.GenPart_p4().at(gen_i));
                        h_idx=gen_i;
                    }
                }
                // the vbf quarks
                if (gen_i==5 || gen_i==6)
                {
                    vbf_quarks_p4.push_back(nt.GenPart_p4().at(gen_i));
                    vbf_quarks_idx.push_back(gen_i);
                }
            }
            // Error handling for ww_p4
            if (ww_p4.empty() || ww_p4.size() < 2) {
                // Handle error
                std::cout << "ww_p4 size: " << ww_p4.size() << std::endl;

                return false;
            }
            int ld_ww_idx = (ww_p4[0].pt() > ww_p4[1].pt()) ? ww_idx[0] : ww_idx[1];
            int tr_ww_idx = (ww_p4[0].pt() > ww_p4[1].pt()) ? ww_idx[1] : ww_idx[0];
            int ld_vbf_idx = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_idx[0] : vbf_quarks_idx[1];
            int tr_vbf_idx = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_idx[1] : vbf_quarks_idx[0];
std::cout << "ld_ww_idx: " << ld_ww_idx << ", tr_ww_idx: " << tr_ww_idx << std::endl;

            std::vector<LorentzVector> ld_ww_fermions_p4;
            std::vector<LorentzVector> tr_ww_fermions_p4;
            std::vector<int> ld_ww_fermions_pdgId;
            std::vector<int> tr_ww_fermions_pdgId;
            std::vector<LorentzVector> h_bb_p4;
            std::vector<LorentzVector> h_non_bb_p4;
            std::vector<int> h_bb_pdgId;
            std::vector<int> h_non_bb_pdgId;


            for (unsigned int gen_ii = 0; gen_ii < nt.nGenPart(); gen_ii++)
            {
                int pdgId = nt.GenPart_pdgId().at(gen_ii);


// dumpGenParticleInfos();
                if (ld_ww_idx==nt.GenPart_genPartIdxMother().at(gen_ii))
                {
                    ld_ww_fermions_p4.push_back(nt.GenPart_p4().at(gen_ii));
                    ld_ww_fermions_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));
                    std::cout << "gen_ii for " << gen_ii << endl;
                    std::cout << "---------------------------------------------------------- " << endl;


                }
                else if (tr_ww_idx==nt.GenPart_genPartIdxMother().at(gen_ii))
                {
                    tr_ww_fermions_p4.push_back(nt.GenPart_p4().at(gen_ii));
                    tr_ww_fermions_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));

                }

                if (h_idx==nt.GenPart_genPartIdxMother().at(gen_ii) && abs(pdgId)==5)
                {
                    h_bb_p4.push_back(nt.GenPart_p4().at(gen_ii));
                    h_bb_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));

                }
                else if (h_idx==nt.GenPart_genPartIdxMother().at(gen_ii) && abs(pdgId)!=5)
                {
                    h_non_bb_p4.push_back(nt.GenPart_p4().at(gen_ii));
                    h_non_bb_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));

                }
            }
            // Error handling for ld_ww_fermions_p4
            if (ld_ww_fermions_p4.empty() || ld_ww_fermions_p4.size() < 2) {
                // Handle error
                std::cout << "ld_ww_fermions_p4 size: " << ld_ww_fermions_p4.size() << std::endl;

                return false;
            }

            // Error handling for tr_ww_fermions_p4
            if (tr_ww_fermions_p4.empty() || tr_ww_fermions_p4.size() < 2) {
                // Handle error
                std::cout << "tr_ww_fermions_p4 size: " << tr_ww_fermions_p4.size() << std::endl;

                return false;
            }

            // Error handling for h_bb_p4
            if (h_bb_p4.empty() || h_bb_p4.size() < 2) {
                // Handle error
                std::cout << "h_bb_p4 size: " << h_bb_p4.size() << std::endl;

                return false;
            }

            // Error handling for h_non_bb_p4
            if (h_non_bb_p4.empty() || h_non_bb_p4.size() < 2) {
                // Handle error
                std::cout << "h_non_bb_p4 size: " << h_non_bb_p4.size() << std::endl;

                return false;
            }
            LorentzVector ld_w_ld_fermion = (ld_ww_fermions_p4[0].pt() > ld_ww_fermions_p4[1].pt()) ? ld_ww_fermions_p4[0] : ld_ww_fermions_p4[1] ; // leading W's leading fermion
            LorentzVector ld_w_tr_fermion = (ld_ww_fermions_p4[0].pt() > ld_ww_fermions_p4[1].pt()) ? ld_ww_fermions_p4[1] : ld_ww_fermions_p4[0]; // leading W's trailing fermion
            LorentzVector tr_w_ld_fermion = (tr_ww_fermions_p4[0].pt() > tr_ww_fermions_p4[1].pt()) ? tr_ww_fermions_p4[0] : tr_ww_fermions_p4[1]; // trailing W's leading fermion
            LorentzVector tr_w_tr_fermion = (tr_ww_fermions_p4[0].pt() > tr_ww_fermions_p4[1].pt()) ? tr_ww_fermions_p4[1] : tr_ww_fermions_p4[0]; // trailing W's trailing fermion

            LorentzVector ld_h_bb = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_p4[0] : h_bb_p4[1] ; // leading bb quark for the h->bb
            LorentzVector tr_h_bb = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_p4[1] : h_bb_p4[0] ; // trainling bb quark for the h->bb
            LorentzVector ld_h_non_bb = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_p4[0] : h_non_bb_p4[1] ; // leading bb quark for the h non bb
            LorentzVector tr_h_non_bb = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_p4[1] : h_non_bb_p4[0] ; // trainling bb quark for the h non bb

            int ld_w_ld_fermion_pdgId = (ld_ww_fermions_p4[0].pt() > ld_ww_fermions_p4[1].pt()) ? ld_ww_fermions_pdgId[0] : ld_ww_fermions_pdgId[1] ; // leading W's leading fermion;
            int ld_w_tr_fermion_pdgId = (ld_ww_fermions_p4[0].pt() > ld_ww_fermions_p4[1].pt()) ? ld_ww_fermions_pdgId[1] : ld_ww_fermions_pdgId[0] ; // leading W's trailing fermion;
            int tr_w_ld_fermion_pdgId = (tr_ww_fermions_p4[0].pt() > tr_ww_fermions_p4[1].pt()) ? tr_ww_fermions_pdgId[0] : tr_ww_fermions_pdgId[1] ; // trainling W's leading fermion;
            int tr_w_tr_fermion_pdgId = (tr_ww_fermions_p4[0].pt() > tr_ww_fermions_p4[1].pt()) ? tr_ww_fermions_pdgId[1] : tr_ww_fermions_pdgId[0] ; // trainling W's trailing fermion;

            int ld_h_bb_pdgId = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_pdgId[0] : h_bb_pdgId[1] ; // leading bb quark for the h->bb
            int tr_h_bb_pdgId = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_pdgId[1] : h_bb_pdgId[0] ; // tr bb quark for the h->bb
            int ld_h_non_bb_pdgId = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_pdgId[0] : h_non_bb_pdgId[1] ; // leading bb quark for the non h bb
            int tr_h_non_bb_pdgId = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_pdgId[1] : h_non_bb_pdgId[0] ; // tr bb quark for the non h bb

            LorentzVector ld_vbf_quarks_p4 = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_p4[0] : vbf_quarks_p4[1] ; // leading vbf quarks
            LorentzVector tr_vbf_quarks_p4 = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_p4[1] : vbf_quarks_p4[0] ; // trailing vbf quarks

            // for the ww fermions
            arbol.setLeaf<double>("gen_ld_w_ld_fermion_pt",ld_w_ld_fermion.pt());
            arbol.setLeaf<double>("gen_ld_w_ld_fermion_eta",ld_w_ld_fermion.eta());
            arbol.setLeaf<double>("gen_ld_w_ld_fermion_phi",ld_w_ld_fermion.phi());
            arbol.setLeaf<double>("gen_ld_w_ld_fermion_mass",ld_w_ld_fermion.mass());
            arbol.setLeaf<double>("gen_ld_w_tr_fermion_pt",ld_w_tr_fermion.pt());
            arbol.setLeaf<double>("gen_ld_w_tr_fermion_eta",ld_w_tr_fermion.eta());
            arbol.setLeaf<double>("gen_ld_w_tr_fermion_phi",ld_w_tr_fermion.phi());
            arbol.setLeaf<double>("gen_ld_w_tr_fermion_mass",ld_w_tr_fermion.mass());
            arbol.setLeaf<double>("gen_tr_w_ld_fermion_pt",tr_w_ld_fermion.pt());
            arbol.setLeaf<double>("gen_tr_w_ld_fermion_eta",tr_w_ld_fermion.eta());
            arbol.setLeaf<double>("gen_tr_w_ld_fermion_phi",tr_w_ld_fermion.phi());
            arbol.setLeaf<double>("gen_tr_w_ld_fermion_mass",tr_w_ld_fermion.mass());
            arbol.setLeaf<double>("gen_tr_w_tr_fermion_pt",tr_w_tr_fermion.pt());
            arbol.setLeaf<double>("gen_tr_w_tr_fermion_eta",tr_w_tr_fermion.eta());
            arbol.setLeaf<double>("gen_tr_w_tr_fermion_phi",tr_w_tr_fermion.phi());
            arbol.setLeaf<double>("gen_tr_w_tr_fermion_mass",tr_w_tr_fermion.mass());

            arbol.setLeaf<double>("gen_ld_w_ld_fermion_pdgId",ld_w_ld_fermion_pdgId);
            arbol.setLeaf<double>("gen_ld_w_tr_fermion_pdgId",ld_w_tr_fermion_pdgId);
            arbol.setLeaf<double>("gen_tr_w_ld_fermion_pdgId",tr_w_ld_fermion_pdgId);
            arbol.setLeaf<double>("gen_tr_w_tr_fermion_pdgId",tr_w_tr_fermion_pdgId);

            // the h decay products
            arbol.setLeaf<double>("gen_ld_h_bb_pt",ld_h_bb.pt());
            arbol.setLeaf<double>("gen_ld_h_bb_eta",ld_h_bb.eta());
            arbol.setLeaf<double>("gen_ld_h_bb_phi",ld_h_bb.phi());
            arbol.setLeaf<double>("gen_ld_h_bb_mass",ld_h_bb.mass());
            arbol.setLeaf<double>("gen_tr_h_bb_pt",tr_h_bb.pt());
            arbol.setLeaf<double>("gen_tr_h_bb_eta",tr_h_bb.eta());
            arbol.setLeaf<double>("gen_tr_h_bb_phi",tr_h_bb.phi());
            arbol.setLeaf<double>("gen_tr_h_bb_mass",tr_h_bb.mass());
            arbol.setLeaf<double>("gen_ld_h_non_bb_pt",ld_h_non_bb.pt());
            arbol.setLeaf<double>("gen_ld_h_non_bb_eta",ld_h_non_bb.eta());
            arbol.setLeaf<double>("gen_ld_h_non_bb_phi",ld_h_non_bb.phi());
            arbol.setLeaf<double>("gen_ld_h_non_bb_mass",ld_h_non_bb.mass());
            arbol.setLeaf<double>("gen_tr_h_non_bb_pt",tr_h_non_bb.pt());
            arbol.setLeaf<double>("gen_tr_h_non_bb_eta",tr_h_non_bb.eta());
            arbol.setLeaf<double>("gen_tr_h_non_bb_phi",tr_h_non_bb.phi());
            arbol.setLeaf<double>("gen_tr_h_non_bb_mass",tr_h_non_bb.mass());

            arbol.setLeaf<double>("gen_ld_h_bb_pdgId",ld_h_bb_pdgId);
            arbol.setLeaf<double>("gen_tr_h_bb_pdgId",tr_h_bb_pdgId);
            arbol.setLeaf<double>("gen_ld_h_non_bb_pdgId",ld_h_non_bb_pdgId);
            arbol.setLeaf<double>("gen_tr_h_non_bb_pdgId",tr_h_non_bb_pdgId);

            // the two vbf quarks branches
            arbol.setLeaf<double>("gen_ld_vbf_quarks_pt",ld_vbf_quarks_p4.pt());
            arbol.setLeaf<double>("gen_ld_vbf_quarks_eta",ld_vbf_quarks_p4.eta());
            arbol.setLeaf<double>("gen_ld_vbf_quarks_phi",ld_vbf_quarks_p4.phi());
            arbol.setLeaf<double>("gen_ld_vbf_quarks_mass",ld_vbf_quarks_p4.mass());
            arbol.setLeaf<double>("gen_tr_vbf_quarks_pt",tr_vbf_quarks_p4.pt());
            arbol.setLeaf<double>("gen_tr_vbf_quarks_eta",tr_vbf_quarks_p4.eta());
            arbol.setLeaf<double>("gen_tr_vbf_quarks_phi",tr_vbf_quarks_p4.phi());
            arbol.setLeaf<double>("gen_tr_vbf_quarks_mass",tr_vbf_quarks_p4.mass());


            return true;
        }
    );
    cutflow.insert("SemiMerged_SaveVariables", allWWH_products_identification_genlevel, Right);

    // //       mb try looking at GenPart_* print statements and look at the "status code" print out for each event only the GenPart that corresponds to nt.GenPart_status()[idx] == 22
    // //
    // // if my understanding is correct you will only see usually 3 particles
    // //
    // // and they will likely be just either "WWH" or "WHH"
    // //
    // // 1. confirm first if that's true.
    // // 2. if that is true, see for some events the full decays and see whether that seems to separate the WWH vs. WHH very cleanly
    // Cut* genpart_status_22 = new LambdaCut(
    //     "GenPart_Status_22",
    //     [&]()
    //     {
    //         std::vector<int> particles_status_22;
    //         int higgs_counter = 0; // Counter for Higgs particles
    //
    //         for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
    //         {
    //             if (nt.GenPart_status().at(gen_i) == 22)
    //             {
    //                 particles_status_22.push_back(nt.GenPart_pdgId().at(gen_i));
    //             }
    //         }
    //
    //         // Count Higgs particles in particles_status_22
    //         for (int pdgId : particles_status_22)
    //         {
    //             if (pdgId == 25)
    //             {
    //                 higgs_counter++;
    //             }
    //         }
    //
    //         if (particles_status_22.size() == 3)
    //         {
    //             std::sort(particles_status_22.begin(), particles_status_22.end());  // For consistency in printing.
    //
    //             for (int i = 0; i < particles_status_22.size(); i++)
    //             {
    //                 std::cout << particles_status_22[i] << " ";  // Printing out the pdgIds.
    //             }
    //             std::cout << "\n";
    //             std::cout << "Higgs count: " << higgs_counter << std::endl; // Printing the Higgs count
    //             std::cout << "------------------------------------------------" << std::endl;
    //             dumpGenParticleInfos();
    //             return true;
    //         }
    //         else
    //         {
    //             std::cout << "failed the particle_status_22==3 condition" << std::endl;
    //             for (int ii = 0; ii < particles_status_22.size(); ii++)
    //             {
    //                 std::cout << particles_status_22[ii] << " ";  // Printing out the pdgIds.
    //             }
    //             std::cout << "\n";
    //             std::cout << "Higgs count: " << higgs_counter << std::endl; // Printing the Higgs count
    //             std::cout << "------------------------------------------------" << std::endl;
    //             dumpGenParticleInfos();
    //             return false;
    //         }
    //     }
    // );
    // cutflow.insert("SemiMerged_SaveVariables", genpart_status_22, Right);






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
//     int count_failed_pairs_fermions = 0;
//     int count_lessthan4_fermions = 0;
//     // int pass_counter = 0;
//
// // classification of the decay of ww to fermions on gen level and seeing which came from which
 // Cut* ww_fermions = new LambdaCut(
//             "WW_fermions",
//             [&]()
//             {
//
//                 // LorentzVector v_p4 = nt.GenPart_p4();
//                 std::vector<LorentzVector> fermions_ww_p4;
//                 // fermions_pair_ww_p4 is vector having two fermions from one w and two from the other w after pplying dR<0.8
//                 std::vector<LorentzVector> fermions_pair_from_ww;
//                 std::vector<int> pdgId_fermions_ww;
//
//                 std::vector<unsigned int> fermions_ww_originalIdx; // Store original indices here
//
//                 LorentzVector ld_w_ld_fermion; // leading W's leading fermion
//                 LorentzVector ld_w_tr_fermion; // leading W's trailing fermion
//                 LorentzVector tr_w_ld_fermion; // trailing W's leading fermion
//                 LorentzVector tr_w_tr_fermion; // trailing W's trailing fermion
//
//                 int ld_w_ld_fermion_pdgId;
//                 int ld_w_tr_fermion_pdgId;
//                 int tr_w_ld_fermion_pdgId;
//                 int tr_w_tr_fermion_pdgId;
//
//                 for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
//                 {
//                     int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
//                     if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
//                     int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
//                     // Check the grandmother
//                     int gen_grandmother_idx = nt.GenPart_genPartIdxMother().at(nt.GenPart_genPartIdxMother().at(gen_i));
//                     int gen_grandmother_pdgId= -1;
//                         gen_grandmother_pdgId = nt.GenPart_pdgId().at(gen_grandmother_idx);
//
//                     // }
//
//                     if ((abs(gen_pdgId)==11 || abs(gen_pdgId)==12 || abs(gen_pdgId)==13 ||abs(gen_pdgId)==14 ||abs(gen_pdgId)==15 ||abs(gen_pdgId)==16
//                     ||abs(gen_pdgId)==17 ||abs(gen_pdgId)==18  ||abs(gen_pdgId)==2 || abs(gen_pdgId)==1 ||abs(gen_pdgId)==4 ||abs(gen_pdgId)==3 ) && abs(gen_mother_pdgId)==24 && (gen_grandmother_pdgId==-1 || abs(gen_grandmother_pdgId)==24))
//                     {
//                         fermions_ww_p4.push_back(nt.GenPart_p4().at(gen_i));
//                         fermions_ww_originalIdx.push_back(gen_i);  // Add the original index to the separate vector
//
//                     }
//                 }
// //                 if (fermions_ww_p4.size() < 4) {
//
// //                     for (size_t i = 0; i < fermions_ww_p4.size(); ++i) {
// //     std::cout << "eta: " << fermions_ww_p4[i].eta()
// //               << ", py: " << fermions_ww_p4[i].phi()
// //               << ", pz: " << fermions_ww_p4[i].mass()
// //               << ", size: " <<       fermions_ww_p4.size()
// //               << ", energy: " << fermions_ww_p4[i].pt() << std::endl;
// //
// //                   // dumpGenParticleInfos();
// // }
//                 if (fermions_ww_p4.size() < 4) {
//                     std::cout << "Less than 4 fermions from W decays found!" << std::endl;
//                     count_lessthan4_fermions++;
//
//                     std::cout << count_lessthan4_fermions << std::endl;
//                     // dumpLHEParticleInfos();
//
//                     return false;
//                 }
//                 std::set<unsigned int> usedIndices;
//                 for (unsigned int i=0; i < fermions_ww_p4.size()-1; i++)
//                 {
//                     if (usedIndicesnt ii = i+1; ii < fermions_ww_p4.size(); ii++)
//                     {
//                         if (usedIndices.count(ii)) continue; // Skip this fermion if it's already been paired
//                         int motherIdx2 = nt.GenPart_genPartIdxMother().at(fermions_ww_originalIdx.at(ii));
//
//                         // double deltaR = ROOT::Math::VectorUtil::DeltaR(fermions_ww_p4.at(ii),fermions_ww_p4.at(i));
//                         if (motherIdx1 == motherIdx2 && motherIdx1 != -1)
//                         {
//                             fermions_pair_from_ww.push_back(fermions_ww_p4.at(i));
//                             fermions_pair_from_ww.push_back(fermions_ww_p4.at(ii));
//                             pdgId_fermions_ww.push_back(nt.GenPart_pdgId().at(fermions_ww_originalIdx.at(i)));
//                             pdgId_fermions_ww.push_back(nt.GenPart_pdgId().at(fermions_ww_originalIdx.at(ii)));
//
//                             // Mark these fermions as used
//                             usedIndices.insert(i);
//                             usedIndices.insert(ii);
//
//                             break; // exit the inner loop once a pair is found for the current fermion
//                         }
//                     }
//
//                 }
//
//
//                 if (fermions_pair_from_ww.size() < 4) {
//                     std::cout << "Not enough fermions can be paired based on deltaR criterion!" << std::endl;
//                     count_failed_pairs_fermions++;
//                     std::cout << count_failed_pairs_fermions << std::endl;
//                     return false;
//                 }
//
//                 double pt_W1 = (fermions_pair_from_ww[0] + fermions_pair_from_ww[1]).Pt();
//                 double pt_W2 = (fermions_pair_from_ww[2] + fermions_pair_from_ww[3]).Pt();
//                 if (pt_W1 > pt_W2)
//                 {
//                     if (fermions_pair_from_ww[0].pt()> fermions_pair_from_ww[1].pt())
//                     {
//                         ld_w_ld_fermion = fermions_pair_from_ww[0];
//                         ld_w_tr_fermion = fermions_pair_from_ww[1];
//
//                         ld_w_ld_fermion_pdgId=pdgId_fermions_ww[0];
//                         ld_w_tr_fermion_pdgId=pdgId_fermions_ww[1];
//                     }
//                     else{
//                         ld_w_ld_fermion = fermions_pair_from_ww[1];
//                         ld_w_tr_fermion = fermions_pair_from_ww[0];
//
//                         ld_w_ld_fermion_pdgId = pdgId_fermions_ww[1];
//                         ld_w_tr_fermion_pdgId = pdgId_fermions_ww[0];
//                     }
//                     if (fermions_pair_from_ww[2].pt()> fermions_pair_from_ww[3].pt())
//                     {
//                         tr_w_ld_fermion = fermions_pair_from_ww[2];
//                         tr_w_tr_fermion = fermions_pair_from_ww[3];
//
//                         tr_w_ld_fermion_pdgId = pdgId_fermions_ww[2];
//                         tr_w_tr_fermion_pdgId = pdgId_fermions_ww[3];
//                     }
//                     else{
//                         tr_w_ld_fermion = fermions_pair_from_ww[3];
//                         tr_w_tr_fermion = fermions_pair_from_ww[2];
//
//                         tr_w_ld_fermion_pdgId = pdgId_fermions_ww[3];
//                         tr_w_tr_fermion_pdgId = pdgId_fermions_ww[2];
//                     }
//
//                 }
//                 else
//                 {
//                     if (fermions_pair_from_ww[0].pt()> fermions_pair_from_ww[1].pt())
//                     {
//                         tr_w_ld_fermion = fermions_pair_from_ww[0];
//                         tr_w_tr_fermion = fermions_pair_from_ww[1];
//
//                         tr_w_ld_fermion_pdgId = pdgId_fermions_ww[0];
//                         tr_w_tr_fermion_pdgId = pdgId_fermions_ww[1];
//                     }
//                     else{
//                         tr_w_ld_fermion = fermions_pair_from_ww[1];
//                         tr_w_tr_fermion = fermions_pair_from_ww[0];
//
//                         tr_w_ld_fermion_pdgId = pdgId_fermions_ww[1];
//                         tr_w_tr_fermion_pdgId = pdgId_fermions_ww[0];
//                     }
//                     if (fermions_pair_from_ww[2].pt()> fermions_pair_from_ww[3].pt())
//                     {
//                         ld_w_ld_fermion = fermions_pair_from_ww[2];
//                         ld_w_tr_fermion = fermions_pair_from_ww[3];
//
//                         ld_w_ld_fermion_pdgId = pdgId_fermions_ww[2];
//                         ld_w_tr_fermion_pdgId = pdgId_fermions_ww[3];
//                     }
//                     else{
//                         ld_w_ld_fermion = fermions_pair_from_ww[3];
//                         ld_w_tr_fermion = fermions_pair_from_ww[2];
//
//                         ld_w_ld_fermion_pdgId = pdgId_fermions_ww[3];
//                         ld_w_tr_fermion_pdgId = pdgId_fermions_ww[2];
//                     }
//
//                 }
//
//                 arbol.setLeaf<double>("ld_w_ld_fermion_pt",ld_w_ld_fermion.pt());
//                 arbol.setLeaf<double>("ld_w_ld_fermion_eta",ld_w_ld_fermion.eta());
//                 arbol.setLeaf<double>("ld_w_ld_fermion_phi",ld_w_ld_fermion.phi());
//                 arbol.setLeaf<double>("ld_w_ld_fermion_mass",ld_w_ld_fermion.mass());
//                 arbol.setLeaf<double>("ld_w_tr_fermion_pt",ld_w_tr_fermion.pt());
//                 arbol.setLeaf<double>("ld_w_tr_fermion_eta",ld_w_tr_fermion.eta());
//                 arbol.setLeaf<double>("ld_w_tr_fermion_phi",ld_w_tr_fermion.phi());
//                 arbol.setLeaf<double>("ld_w_tr_fermion_mass",ld_w_tr_fermion.mass());
//                 arbol.setLeaf<double>("tr_w_ld_fermion_pt",tr_w_ld_fermion.pt());
//                 arbol.setLeaf<double>("tr_w_ld_fermion_eta",tr_w_ld_fermion.eta());
//                 arbol.setLeaf<double>("tr_w_ld_fermion_phi",tr_w_ld_fermion.phi());
//                 arbol.setLeaf<double>("tr_w_ld_fermion_mass",tr_w_ld_fermion.mass());
//                 arbol.setLeaf<double>("tr_w_tr_fermion_pt",tr_w_tr_fermion.pt());
//                 arbol.setLeaf<double>("tr_w_tr_fermion_eta",tr_w_tr_fermion.eta());
//                 arbol.setLeaf<double>("tr_w_tr_fermion_phi",tr_w_tr_fermion.phi());
//                 arbol.setLeaf<double>("tr_w_tr_fermion_mass",tr_w_tr_fermion.mass());
//
//                 arbol.setLeaf<double>("ld_w_ld_fermion_pdgId",ld_w_ld_fermion_pdgId);
//                 arbol.setLeaf<double>("ld_w_tr_fermion_pdgId",ld_w_tr_fermion_pdgId);
//                 arbol.setLeaf<double>("tr_w_ld_fermion_pdgId",tr_w_ld_fermion_pdgId);
//                 arbol.setLeaf<double>("tr_w_tr_fermion_pdgId",tr_w_tr_fermion_pdgId);
//                 // std:: cout << "fermion mass"<<tr_w_tr_fermion.mass()<< endl;
//
//               // pass_counter++;
//               // std::cout << pass_counter<<endl;
//
//                 return true;
//
//
//             }
//         );
//         cutflow.insert("SemiMerged_SaveVariables", ww_fermions, Right);

    //truth tagging hbb
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

    // choosing hbbfatjet_n_true_higgsbquark to be equal to 0 to see the actual graphs without higgs

    // Cut* bQuarksInHiggsJeteq0 = new LambdaCut(
    //     "BQuarksInHiggsJeteq0",
    //     [&]()
    //     {
    //         return arbol.getLeaf<int>("bQuarksInHiggsJet")==0;
    //
    //     }
    // );
    // cutflow.insert(hbbfatjet_n_true_higgsbquark, bQuarksInHiggsJeteq0, Right);



//     //truth tagging WWWWWWWWWWWWW Bosons
//     Cut* hwwfatjet_n_true_higgswboson = new LambdaCut(
//     		"Hwwfatjet_n_true_higgswboson",
//     		[&]()
//     		{
//
//     			LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
//     			std::vector<LorentzVector> wBosons;
//     			int wBosonsInHiggsJet = 0;
//     			for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
//     			{
//     				int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
//     				if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
//     				int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
//     				if (abs(gen_pdgId)==24 && gen_mother_pdgId==25)
//     				{
//     					wBosons.push_back(nt.GenPart_p4().at(gen_i));
//     				}
//     			}
//     			for (unsigned int i=0; i < wBosons.size(); i++)
//     			{
//     				double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,wBosons.at(i));
//     				if (deltaR<0.8)
//     				{
//     					wBosonsInHiggsJet++;
//     				}
//     			}
//     			arbol.setLeaf<int>("wBosonsInHiggsJet",wBosonsInHiggsJet);
//     			return true;
//
//
//     		}
//     	);
//     	cutflow.insert("SemiMerged_SaveVariables", hwwfatjet_n_true_higgswboson, Right);
//
//     // choosing hwwfatjet_n_true_higgsbquark to be equal to 0 to see the actual graphs withouy higgs
//
//     Cut* wBosonsInHiggsJeteq0 = new LambdaCut(
//     	"WBosonsInHiggsJeteq0",
//     	[&]()
//     	{
//     		return arbol.getLeaf<int>("wBosonsInHiggsJet")==0;
//
//     	}
//     );
//     cutflow.insert(hwwfatjet_n_true_higgswboson, wBosonsInHiggsJeteq0, Right);
//
//
//     //truth tagging ZZZZZZZZZZZZZZZZ bosons
//     Cut* hzzfatjet_n_true_higgszboson = new LambdaCut(
//     		"Hzzfatjet_n_true_higgszboson",
//     		[&]()
//     		{
//
//     			LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
//     			std::vector<LorentzVector> zBosons;
//     			int zBosonsInHiggsJet = 0;
//     			for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
//     			{
//     				int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
//     				if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
//     				int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
//     				if (abs(gen_pdgId)==23 && gen_mother_pdgId==25)
//     				{
//     					zBosons.push_back(nt.GenPart_p4().at(gen_i));
//     				}
//     			}
//     			for (unsigned int i=0; i < zBosons.size(); i++)
//     			{
//     				double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,zBosons.at(i));
//     				if (deltaR<0.8)
//     				{
//     					zBosonsInHiggsJet++;
//     				}
//     			}
//     			arbol.setLeaf<int>("zBosonsInHiggsJet",zBosonsInHiggsJet);
//     			return true;
//
//
//     		}
//     	);
//     	cutflow.insert("SemiMerged_SaveVariables", hzzfatjet_n_true_higgszboson, Right);
//
//     // choosing hzzfatjet_n_true_higgszboson to be equal to 0 to see the actual graphs without Higgs
//
//     Cut* zBosonsInHiggsJeteq0 = new LambdaCut(
//     	"ZBosonsInHiggsJeteq0",
//     	[&]()
//     	{
//     		return arbol.getLeaf<int>("zBosonsInHiggsJet")==0;
//
//     	}
//     );
//     cutflow.insert(hzzfatjet_n_true_higgszboson, zBosonsInHiggsJeteq0, Right);
//
//
//
//     //truth tagging Charm quarks cc
//     Cut* hccfatjet_n_true_higgscquark = new LambdaCut(
//     		"Hccfatjet_n_true_higgscquark",
//     		[&]()
//     		{
//
//     			LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
//     			std::vector<LorentzVector> cQuarks;
//     			int cQuarksInHiggsJet = 0;
//     			for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
//     			{
//     				int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
//     				if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
//     				int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
//     				if (abs(gen_pdgId)==4 && gen_mother_pdgId==25)
//     				{
//     					cQuarks.push_back(nt.GenPart_p4().at(gen_i));
//     				}
//     			}
//     			for (unsigned int i=0; i < cQuarks.size(); i++)
//     			{
//     				double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,cQuarks.at(i));
//     				if (deltaR<0.8)
//     				{
//     					cQuarksInHiggsJet++;
//     				}
//     			}
//     			arbol.setLeaf<int>("cQuarksInHiggsJet",cQuarksInHiggsJet);
//     			return true;
//     		}
//     	);
//     	cutflow.insert("SemiMerged_SaveVariables", hccfatjet_n_true_higgscquark, Right);
//
//     // choosing hccfatjet_n_true_higgscquark to be equal to 0 to see the actual graphs without Higgs
//
//     Cut* cQuarksInHiggsJeteq0 = new LambdaCut(
//     	"CQuarksInHiggsJeteq0",
//     	[&]()
//     	{
//     		return arbol.getLeaf<int>("cQuarksInHiggsJet")==0;
//     	}
//     );
//     cutflow.insert(hccfatjet_n_true_higgscquark, cQuarksInHiggsJeteq0, Right);
//
//
//     //truth tagging for Tau Leptons
//     Cut* httfatjet_n_true_higgstlepton = new LambdaCut(
//     		"Httfatjet_n_true_higgstlepton",
//     		[&]()
//     		{
//
//     			LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
//     			std::vector<LorentzVector> tLeptons;
//     			int tLeptonsInHiggsJet = 0;
//     			for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
//     			{
//     				int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
//     				if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
//     				int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
//     				if (abs(gen_pdgId)==15 && gen_mother_pdgId==25)
//     				{
//     					tLeptons.push_back(nt.GenPart_p4().at(gen_i));
//     				}
//     			}
//     			for (unsigned int i=0; i < tLeptons.size(); i++)
//     			{
//     				double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,tLeptons.at(i));
//     				if (deltaR<0.8)
//     				{
//     					tLeptonsInHiggsJet++;
//     				}
//     			}
//     			arbol.setLeaf<int>("tLeptonsInHiggsJet",tLeptonsInHiggsJet);
//     			return true;
//     		}
//     	);
//     	cutflow.insert("SemiMerged_SaveVariables", httfatjet_n_true_higgstlepton, Right);
//
//     // choosing httfatjet_n_true_higgstlepton to be equal to 0 to see the actual graphs without Higgs
//
//     Cut* tLeptonsInHiggsJeteq0 = new LambdaCut(
//     	"TLeptonsInHiggsJeteq0",
//     	[&]()
//     	{
//     		return arbol.getLeaf<int>("tLeptonsInHiggsJet")==0;
//     	}
//     );
//     cutflow.insert(httfatjet_n_true_higgstlepton, tLeptonsInHiggsJeteq0, Right);
//
//
//     //truth tagging gluon pair gg
//     Cut* hggfatjet_n_true_higgsgluon = new LambdaCut(
//     		"Hggfatjet_n_true_higgsgluon",
//     		[&]()
//     		{
//
//     			LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
//     			std::vector<LorentzVector> gluons;
//     			int gluonsInHiggsJet = 0;
//     			for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
//     			{
//     				int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
//     				if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
//     				int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
//     				if (abs(gen_pdgId)==21 && gen_mother_pdgId==25)
//     				{
//     					gluons.push_back(nt.GenPart_p4().at(gen_i));
//     				}
//     			}
//     			for (unsigned int i=0; i < gluons.size(); i++)
//     			{
//     				double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,gluons.at(i));
//     				if (deltaR<0.8)
//     				{
//     					gluonsInHiggsJet++;
//     				}
//     			}
//     			arbol.setLeaf<int>("gluonsInHiggsJet",gluonsInHiggsJet);
//     			return true;
//     		}
//     	);
//     	cutflow.insert("SemiMerged_SaveVariables", hggfatjet_n_true_higgsgluon, Right);
//
//     // choosing hggfatjet_n_true_higgsgluon to be equal to 0 to see the actual graphs without Higgs
//
//     Cut* gluonsInHiggsJeteq0 = new LambdaCut(
//     	"GluonsInHiggsJeteq0",
//     	[&]()
//     	{
//     		return arbol.getLeaf<int>("gluonsInHiggsJet")==0;
//     	}
//     );
//     cutflow.insert(hggfatjet_n_true_higgsgluon, gluonsInHiggsJeteq0, Right);
//
//
//     //truth tagging  gamma pair yy
// Cut* hggfatjet_n_true_higgsgamma = new LambdaCut(
// 		"Hggfatjet_n_true_higgsgamma",
// 		[&]()
// 		{
//
// 			LorentzVector higgsJetP4 = cutflow.globals.getVal<LorentzVector>("hbbfatjet_p4");
// 			std::vector<LorentzVector> gammas;
// 			int gammasInHiggsJet = 0;
// 			for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); gen_i++)
// 			{
// 				int gen_pdgId=nt.GenPart_pdgId().at(gen_i);
// 				if (nt.GenPart_genPartIdxMother().at(gen_i)<0) {continue;}
// 				int gen_mother_pdgId=nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i));
// 				if (abs(gen_pdgId)==22 && gen_mother_pdgId==25)
// 				{
// 					gammas.push_back(nt.GenPart_p4().at(gen_i));
// 				}
// 			}
// 			for (unsigned int i=0; i < gammas.size(); i++)
// 			{
// 				double deltaR = ROOT::Math::VectorUtil::DeltaR(higgsJetP4,gammas.at(i));
// 				if (deltaR<0.8)
// 				{
// 					gammasInHiggsJet++;
// 				}
// 			}
// 			arbol.setLeaf<int>("gammasInHiggsJet",gammasInHiggsJet);
// 			return true;
// 		}
// 	);
// 	cutflow.insert("SemiMerged_SaveVariables", hggfatjet_n_true_higgsgamma, Right);
//
// // choosing hggfatjet_n_true_higgsgamma to be equal to 0 to see the actual graphs without Higgs
//
// Cut* gammasInHiggsJeteq0 = new LambdaCut(
// 	"GammasInHiggsJeteq0",
// 	[&]()
// 	{
// 		return arbol.getLeaf<int>("gammasInHiggsJet")==0;
// 	}
// );
// cutflow.insert(hggfatjet_n_true_higgsgamma, gammasInHiggsJeteq0, Right);


    // choosing hbbfatjet_n_true_higgsbquark to be equal to 2 to see the actual graphs with higgs

    // Cut* bQuarksInHiggsJeteq2 = new LambdaCut(
    //     "BQuarksInHiggsJeteq2",
    //     [&]()
    //     {
    //         return arbol.getLeaf<int>("bQuarksInHiggsJet")==2;
    //
    //     }
    // );
    // cutflow.insert(hbbfatjet_n_true_higgsbquark, bQuarksInHiggsJeteq2, Right);

    // choosing hbbfatjet_n_true_higgsbquark to be equal to 0 to see the actual graphs withouy higgs


// trying to see the higgs which diddn't came from h->bb since when we investigated bQuarksInHiggsJet=0 we found little peak around the higgs although there hsouldn't be higgs



    // // Print the data
    // for (size_t i = 0; i < savedParticleIds.size(); i++)
    // {
    //     std::cout << "Particle ID: " << savedParticleIds[i] << ", count: " << savedParticleCounts[i] << std::endl;
    // }

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
