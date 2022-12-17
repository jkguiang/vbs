#include "vbsvvhjets/collections.h"
// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "tqdm.h"

int getChargeQx3(int q_pdgID)
{
    switch (abs(q_pdgID))
    {
    case 1:
        return -1; // down
    case 2:
        return  2; // up
    case 3:
        return -1; // strange
    case 4:
        return  2; // charm
    case 5:
        return -1; // bottom
    case 6:
        return  2; // top
    default:
        return -999;
    }
}

double getChargeQQ(int q1_pdgID, int q2_pdgID)
{
    // Returns charge(q + q) or a nonsense number if 2 quarks are not provided
    int q1_sign = (q1_pdgID > 0) - (q1_pdgID < 0);
    int q2_sign = (q2_pdgID > 0) - (q2_pdgID < 0);
    return (q1_sign*getChargeQx3(q1_pdgID) + q2_sign*getChargeQx3(q2_pdgID))/3.;
}

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);
    arbol.newBranch<double>("fatjet1_pt", -999);
    arbol.newBranch<double>("fatjet1_eta", -999);
    arbol.newBranch<double>("fatjet1_phi", -999);
    arbol.newBranch<double>("fatjet1_wqq", -999);
    arbol.newBranch<double>("fatjet1_zqq", -999);
    arbol.newBranch<double>("fatjet1_hbb", -999);
    arbol.newBranch<double>("fatjet1_xqq", -999);
    arbol.newBranch<double>("fatjet1_xwqq", -999);
    arbol.newBranch<double>("fatjet1_xcc", -999);
    arbol.newBranch<double>("fatjet1_xbb", -999);
    arbol.newBranch<int>("fatjet1_match_pdgID", -999);

    arbol.newBranch<double>("fatjet2_pt", -999);
    arbol.newBranch<double>("fatjet2_eta", -999);
    arbol.newBranch<double>("fatjet2_phi", -999);
    arbol.newBranch<double>("fatjet2_wqq", -999);
    arbol.newBranch<double>("fatjet2_zqq", -999);
    arbol.newBranch<double>("fatjet2_hbb", -999);
    arbol.newBranch<double>("fatjet2_xqq", -999);
    arbol.newBranch<double>("fatjet2_xwqq", -999);
    arbol.newBranch<double>("fatjet2_xcc", -999);
    arbol.newBranch<double>("fatjet2_xbb", -999);
    arbol.newBranch<int>("fatjet2_match_pdgID", -999);

    arbol.newBranch<double>("fatjet3_pt", -999);
    arbol.newBranch<double>("fatjet3_eta", -999);
    arbol.newBranch<double>("fatjet3_phi", -999);
    arbol.newBranch<double>("fatjet3_wqq", -999);
    arbol.newBranch<double>("fatjet3_zqq", -999);
    arbol.newBranch<double>("fatjet3_hbb", -999);
    arbol.newBranch<double>("fatjet3_xqq", -999);
    arbol.newBranch<double>("fatjet3_xwqq", -999);
    arbol.newBranch<double>("fatjet3_xcc", -999);
    arbol.newBranch<double>("fatjet3_xbb", -999);
    arbol.newBranch<int>("fatjet3_match_pdgID", -999);
    arbol.newBranch<int>("fatjets_n_matches", -999);

    arbol.newBranch<bool>("gen_H_is_matched", false);
    arbol.newBranch<bool>("gen_H_is_bb", false);
    arbol.newBranch<double>("gen_H_pt", -999);
    arbol.newBranch<double>("gen_H_eta", -999);
    arbol.newBranch<double>("gen_H_phi", -999);

    arbol.newBranch<bool>("gen_ld_V_is_matched", false);
    arbol.newBranch<bool>("gen_ld_V_is_qq", false);
    arbol.newBranch<double>("gen_ld_V_pt", -999);
    arbol.newBranch<double>("gen_ld_V_eta", -999);
    arbol.newBranch<double>("gen_ld_V_phi", -999);

    arbol.newBranch<bool>("gen_tr_V_is_matched", false);
    arbol.newBranch<bool>("gen_tr_V_is_qq", false);
    arbol.newBranch<double>("gen_tr_V_pt", -999);
    arbol.newBranch<double>("gen_tr_V_eta", -999);
    arbol.newBranch<double>("gen_tr_V_phi", -999);
    arbol.newBranch<bool>("gen_is_allhadronic", false);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");
    cutflow.globals.newVar<LorentzVector>("fatjet1_p4");
    cutflow.globals.newVar<LorentzVector>("fatjet2_p4");
    cutflow.globals.newVar<LorentzVector>("fatjet3_p4");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCutflow();

    Cut* geq2_fatjets = new LambdaCut(
        "Geq2FatJets", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 2; }
    );
    cutflow.insert("SelectFatJets", geq2_fatjets, Right);

    Cut* save_candidates = new LambdaCut(
        "SaveBosonCandidates",
        [&]()
        {
            LorentzVectors fatjet_p4s = cutflow.globals.getVal<LorentzVectors>("good_fatjet_p4s");
            Doubles fatjet_wqqtags = cutflow.globals.getVal<Doubles>("good_fatjet_wqqtags");
            Doubles fatjet_zqqtags = cutflow.globals.getVal<Doubles>("good_fatjet_zqqtags");
            Doubles fatjet_hbbtags = cutflow.globals.getVal<Doubles>("good_fatjet_hbbtags");
            Doubles fatjet_xwqqtags = cutflow.globals.getVal<Doubles>("good_fatjet_xwqqtags");
            Doubles fatjet_xqqtags = cutflow.globals.getVal<Doubles>("good_fatjet_xqqtags");
            Doubles fatjet_xcctags = cutflow.globals.getVal<Doubles>("good_fatjet_xcctags");
            Doubles fatjet_xbbtags = cutflow.globals.getVal<Doubles>("good_fatjet_xbbtags");

            cutflow.globals.setVal<LorentzVector>("fatjet1_p4", fatjet_p4s.at(0));
            arbol.setLeaf<double>("fatjet1_pt",  fatjet_p4s.at(0).pt());
            arbol.setLeaf<double>("fatjet1_eta", fatjet_p4s.at(0).eta());
            arbol.setLeaf<double>("fatjet1_phi", fatjet_p4s.at(0).phi());
            arbol.setLeaf<double>("fatjet1_wqq", fatjet_wqqtags.at(0));
            arbol.setLeaf<double>("fatjet1_zqq", fatjet_zqqtags.at(0));
            arbol.setLeaf<double>("fatjet1_hbb", fatjet_hbbtags.at(0));
            arbol.setLeaf<double>("fatjet1_xqq", fatjet_xqqtags.at(0));
            arbol.setLeaf<double>("fatjet1_xcc", fatjet_xcctags.at(0));
            arbol.setLeaf<double>("fatjet1_xwqq", fatjet_xwqqtags.at(0));
            arbol.setLeaf<double>("fatjet1_xbb", fatjet_xbbtags.at(0));

            cutflow.globals.setVal<LorentzVector>("fatjet2_p4", fatjet_p4s.at(1));
            arbol.setLeaf<double>("fatjet2_pt",  fatjet_p4s.at(1).pt());
            arbol.setLeaf<double>("fatjet2_eta", fatjet_p4s.at(1).eta());
            arbol.setLeaf<double>("fatjet2_phi", fatjet_p4s.at(1).phi());
            arbol.setLeaf<double>("fatjet2_wqq", fatjet_wqqtags.at(1));
            arbol.setLeaf<double>("fatjet2_zqq", fatjet_zqqtags.at(1));
            arbol.setLeaf<double>("fatjet2_hbb", fatjet_hbbtags.at(1));
            arbol.setLeaf<double>("fatjet2_xqq", fatjet_xqqtags.at(1));
            arbol.setLeaf<double>("fatjet2_xcc", fatjet_xcctags.at(1));
            arbol.setLeaf<double>("fatjet2_xwqq", fatjet_xwqqtags.at(1));
            arbol.setLeaf<double>("fatjet2_xbb", fatjet_xbbtags.at(1));

            if (arbol.getLeaf<int>("n_fatjets") >= 3)
            {
                cutflow.globals.setVal<LorentzVector>("fatjet3_p4", fatjet_p4s.at(2));
                arbol.setLeaf<double>("fatjet3_pt",  fatjet_p4s.at(2).pt());
                arbol.setLeaf<double>("fatjet3_eta", fatjet_p4s.at(2).eta());
                arbol.setLeaf<double>("fatjet3_phi", fatjet_p4s.at(2).phi());
                arbol.setLeaf<double>("fatjet3_wqq", fatjet_wqqtags.at(2));
                arbol.setLeaf<double>("fatjet3_zqq", fatjet_zqqtags.at(2));
                arbol.setLeaf<double>("fatjet3_hbb", fatjet_hbbtags.at(2));
                arbol.setLeaf<double>("fatjet3_xqq", fatjet_xqqtags.at(2));
                arbol.setLeaf<double>("fatjet3_xcc", fatjet_xcctags.at(2));
                arbol.setLeaf<double>("fatjet3_xwqq", fatjet_xwqqtags.at(2));
                arbol.setLeaf<double>("fatjet3_xbb", fatjet_xbbtags.at(2));
            }
            return true;
        }
    );
    cutflow.insert(geq2_fatjets, save_candidates, Right);

    Cut* gen_matching = new LambdaCut(
        "MatchFatJetsToGenBosons",
        [&]()
        {
            int ld_V_idx = -999; // leading vector boson
            LorentzVectors boson_p4s = {};
            Integers boson_idxs = {};
            Integers boson_pdgIDs = {};
            std::vector<std::vector<int>> boson_children = {};
            for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); ++gen_i)
            {
                int pdgID = nt.GenPart_pdgId().at(gen_i);
                if (nt.GenPart_status().at(gen_i) == 62 && (abs(pdgID) == 23 || abs(pdgID) == 24 || abs(pdgID) == 25))
                {
                    LorentzVector boson_p4 = nt.GenPart_p4().at(gen_i);
                    std::vector<int> children = {};
                    switch (abs(pdgID))
                    {
                    case 23:
                    case 24:
                        if (ld_V_idx == -999 || boson_p4.pt() > boson_p4s.at(ld_V_idx).pt())
                        {
                            ld_V_idx = boson_p4s.size();
                        }
                        boson_p4s.push_back(boson_p4);
                        boson_idxs.push_back(gen_i);
                        boson_pdgIDs.push_back(pdgID);
                        boson_children.push_back(children);
                        break;
                    case 25:
                        boson_p4s.insert(boson_p4s.begin(), boson_p4);
                        boson_idxs.insert(boson_idxs.begin(), gen_i);
                        boson_pdgIDs.insert(boson_pdgIDs.begin(), pdgID);
                        boson_children.insert(boson_children.begin(), children);
                        if (ld_V_idx != -999) { ld_V_idx++; } // shift index of leading V by one, since H has been inserted
                        break;
                    }
                }
                else
                {
                    auto iter = std::find(
                        boson_idxs.begin(), boson_idxs.end(), 
                        nt.GenPart_genPartIdxMother().at(gen_i)
                    );
                    if (iter != boson_idxs.end())
                    {
                        boson_children.at(iter - boson_idxs.begin()).push_back(pdgID);
                    }
                }
            }

            if (boson_pdgIDs.size() != 3)
            {
                return false; // some events have W W W (W --> jj) H
            }
            for (unsigned int boson_i = 0; boson_i < boson_pdgIDs.size(); ++boson_i)
            {
                if (boson_children.at(boson_i).size() != 2)
                {
                    return false; // some events have H --> WW* --> W j j
                    break;
                }
            }

            std::iter_swap(boson_p4s.begin() + ld_V_idx, boson_p4s.begin() + 1);
            std::iter_swap(boson_idxs.begin() + ld_V_idx, boson_idxs.begin() + 1);
            std::iter_swap(boson_pdgIDs.begin() + ld_V_idx, boson_pdgIDs.begin() + 1);
            std::iter_swap(boson_children.begin() + ld_V_idx, boson_children.begin() + 1);

            std::vector<int> H_decay = boson_children.at(0);
            arbol.setLeaf<bool>("gen_H_is_bb", abs(H_decay.at(0)) == 5 && abs(H_decay.at(1)) == 5);

            std::vector<int> ld_V_decay = boson_children.at(1);
            if (abs(boson_pdgIDs.at(1)) == 23)
            {
                arbol.setLeaf<bool>("gen_ld_V_is_qq", getChargeQQ(ld_V_decay.at(0), ld_V_decay.at(1)) == 0);
            }
            else if (abs(boson_pdgIDs.at(1)) == 24)
            {
                arbol.setLeaf<bool>("gen_ld_V_is_qq", abs(getChargeQQ(ld_V_decay.at(0), ld_V_decay.at(1))) == 1);
            }

            std::vector<int> tr_V_decay = boson_children.at(2);
            if (abs(boson_pdgIDs.at(2)) == 23)
            {
                arbol.setLeaf<bool>("gen_tr_V_is_qq", getChargeQQ(tr_V_decay.at(0), tr_V_decay.at(1)) == 0);
            }
            else if (abs(boson_pdgIDs.at(2)) == 24)
            {
                arbol.setLeaf<bool>("gen_tr_V_is_qq", abs(getChargeQQ(tr_V_decay.at(0), tr_V_decay.at(1))) == 1);
            }

            arbol.setLeaf<bool>(
                "gen_is_allhadronic", 
                (arbol.getLeaf<bool>("gen_H_is_bb") 
                 && arbol.getLeaf<bool>("gen_ld_V_is_qq") 
                 && arbol.getLeaf<bool>("gen_tr_V_is_qq"))
            );

            arbol.setLeaf<double>("gen_H_pt",  boson_p4s.at(0).pt());
            arbol.setLeaf<double>("gen_H_eta", boson_p4s.at(0).eta());
            arbol.setLeaf<double>("gen_H_phi", boson_p4s.at(0).phi());

            arbol.setLeaf<double>("gen_ld_V_pt",  boson_p4s.at(1).pt());
            arbol.setLeaf<double>("gen_ld_V_eta", boson_p4s.at(1).eta());
            arbol.setLeaf<double>("gen_ld_V_phi", boson_p4s.at(1).phi());

            arbol.setLeaf<double>("gen_tr_V_pt",  boson_p4s.at(2).pt());
            arbol.setLeaf<double>("gen_tr_V_eta", boson_p4s.at(2).eta());
            arbol.setLeaf<double>("gen_tr_V_phi", boson_p4s.at(2).phi());

            LorentzVector fatjet1_p4 = cutflow.globals.getVal<LorentzVector>("fatjet1_p4");
            LorentzVector fatjet2_p4 = cutflow.globals.getVal<LorentzVector>("fatjet2_p4");
            LorentzVector fatjet3_p4 = cutflow.globals.getVal<LorentzVector>("fatjet3_p4");

            double matches[9] = {999., 999., 999., 999., 999., 999., 999., 999., 999.};
            for (unsigned int boson_i = 0; boson_i < 3; ++boson_i)
            {
                LorentzVector boson_p4 = boson_p4s.at(boson_i);
                matches[boson_i*3 + 0] = ROOT::Math::VectorUtil::DeltaR(boson_p4, fatjet1_p4);
                matches[boson_i*3 + 1] = ROOT::Math::VectorUtil::DeltaR(boson_p4, fatjet2_p4);
                if (arbol.getLeaf<int>("n_fatjets") >= 3)
                {
                    matches[boson_i*3 + 2] = ROOT::Math::VectorUtil::DeltaR(boson_p4, fatjet3_p4);
                }
            }

            int n_matches = 0;
            for (unsigned int cycle = 0; cycle < 3; ++cycle)
            {
                // Find the match with minimum dR(boson, fatjet)
                int best_match_i = (
                    std::min_element(std::begin(matches), std::end(matches)) - std::begin(matches)
                );
                int matched_boson_i = best_match_i/3;
                int matched_fatjet_j = best_match_i - matched_boson_i*3;
                if (matches[best_match_i] < 0.8)
                {
                    n_matches++;
                    switch (matched_fatjet_j)
                    {
                    case 0:
                        arbol.setLeaf<int>("fatjet1_match_pdgID", boson_pdgIDs.at(matched_boson_i));
                        break;
                    case 1:
                        arbol.setLeaf<int>("fatjet2_match_pdgID", boson_pdgIDs.at(matched_boson_i));
                        break;
                    case 2:
                        arbol.setLeaf<int>("fatjet3_match_pdgID", boson_pdgIDs.at(matched_boson_i));
                        break;
                    }
                    switch (matched_boson_i)
                    {
                    case 0:
                        arbol.setLeaf<int>("gen_H_is_matched", true);
                        break;
                    case 1:
                        arbol.setLeaf<int>("gen_ld_V_is_matched", true);
                        break;
                    case 2:
                        arbol.setLeaf<int>("gen_tr_V_is_matched", true);
                        break;
                    }
                    // Remove matched boson from consideration
                    matches[matched_boson_i*3 + 0] = 999.;
                    matches[matched_boson_i*3 + 1] = 999.;
                    matches[matched_boson_i*3 + 2] = 999.;
                    // Remove matched fat jet from consideration
                    matches[0 + matched_fatjet_j] = 999.;
                    matches[3 + matched_fatjet_j] = 999.;
                    matches[6 + matched_fatjet_j] = 999.;
                }
                else
                {
                    // All other matches are worse => no additional matches possible
                    break;
                }
            }

            arbol.setLeaf<int>("fatjets_n_matches", n_matches);
            return true;
        }
    );
    cutflow.insert(save_candidates, gen_matching, Right);

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
                // Run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.run(gen_matching);
                if (passed) { arbol.fill(); }
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
