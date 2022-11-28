#include "../vbswh.h"
#include "cuts.h"
// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "histflow.h"
#include "utilities.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "ElectronSelections.h"
#include "MuonSelections.h"
#include "tqdm.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<double> Doubles;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

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
    int q1_sign = (q1_pdgID > 0) - (q1_pdgID < 0);
    int q2_sign = (q2_pdgID > 0) - (q2_pdgID < 0);
    return (q1_sign*getChargeQx3(q1_pdgID) + q2_sign*getChargeQx3(q2_pdgID)) / 3.;
}

int getPartonPairNum(unsigned int pdg_id1, unsigned int pdg_id2)
{
    if (pdg_id1 == 5 && pdg_id2 == 5) { return 0; } // b, b
    if ((pdg_id1 == 5 && pdg_id2 == 4) || (pdg_id1 == 4 && pdg_id2 == 5)) { return 1; } // b, c
    if ((pdg_id1 == 5 && pdg_id2 == 3) || (pdg_id1 == 3 && pdg_id2 == 5)) { return 2; } // b, s
    if ((pdg_id1 == 5 && pdg_id2 == 2) || (pdg_id1 == 2 && pdg_id2 == 5)) { return 3; } // b, u
    if ((pdg_id1 == 5 && pdg_id2 == 1) || (pdg_id1 == 1 && pdg_id2 == 5)) { return 4; } // b, d
    if (pdg_id1 == 4 && pdg_id2 == 4) { return 5; } // c, c
    if ((pdg_id1 == 4 && pdg_id2 == 3) || (pdg_id1 == 3 && pdg_id2 == 4)) { return 6; } // c, s
    if ((pdg_id1 == 4 && pdg_id2 == 2) || (pdg_id1 == 2 && pdg_id2 == 4)) { return 7; } // c, u
    if ((pdg_id1 == 4 && pdg_id2 == 1) || (pdg_id1 == 1 && pdg_id2 == 4)) { return 8; } // c, d
    if (pdg_id1 == 3 && pdg_id2 == 3) { return 9; } // s, s
    if ((pdg_id1 == 3 && pdg_id2 == 2) || (pdg_id1 == 2 && pdg_id2 == 3)) { return 10; } // s, u
    if ((pdg_id1 == 3 && pdg_id2 == 1) || (pdg_id1 == 1 && pdg_id2 == 3)) { return 11; } // s, d
    if (pdg_id1 == 2 && pdg_id2 == 2) { return 12; } // u, u
    if ((pdg_id1 == 2 && pdg_id2 == 1) || (pdg_id1 == 1 && pdg_id2 == 2)) { return 13; } // u, d
    if (pdg_id1 == 1 && pdg_id2 == 1) { return 14; } // d, d
    else { return -1; }
}

int main(int argc, char** argv) 
{
    gconf.nanoAOD_ver = 9;

    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);
    // Initialize Arbusto
    Arbol arbol = Arbol(cli);
    arbol.newBranch<int>("lhe_parton1_pdgID", -999);
    arbol.newBranch<int>("lhe_parton2_pdgID", -999);
    arbol.newBranch<int>("lhe_ld_q_pdgID", -999);
    arbol.newBranch<int>("lhe_tr_q_pdgID", -999);
    arbol.newBranch<int>("lhe_lep_pdgID", -999);
    arbol.newBranch<double>("lhe_M_qq", -999);
    arbol.newBranch<double>("lhe_deta_qq", -999);
    arbol.newBranch<double>("lhe_M_lnu", -999);
    arbol.newBranch<double>("lhe_M_lnuqq", -999);
    arbol.newBranch<bool>("lhe_is_VBSW", false);
    arbol.newBranch<bool>("lhe_is_WW", false);
    arbol.newBranch<bool>("gen_is_VBSW", false);
    arbol.newBranch<bool>("gen_is_WW", false);
    arbol.newBranch<int>("gen_ld_q_pdgID", -999);
    arbol.newBranch<double>("gen_ld_q_pt", -999);
    arbol.newBranch<double>("gen_ld_q_eta", -999);
    arbol.newBranch<double>("gen_ld_q_phi", -999);
    arbol.newBranch<bool>("gen_ld_q_in_hbb_cone", false);
    arbol.newBranch<int>("gen_tr_q_pdgID", -999);
    arbol.newBranch<double>("gen_tr_q_pt", -999);
    arbol.newBranch<double>("gen_tr_q_eta", -999);
    arbol.newBranch<double>("gen_tr_q_phi", -999);
    arbol.newBranch<bool>("gen_tr_q_in_hbb_cone", false);
    arbol.newBranch<int>("gen_lep_pdgID", -999);
    arbol.newBranch<double>("gen_lep_pt", -999);
    arbol.newBranch<double>("gen_lep_eta", -999);
    arbol.newBranch<double>("gen_lep_phi", -999);
    arbol.newBranch<int>("n_gen_q_in_hbb_cone", -999);
    arbol.newBranch<float>("n_gen_q_VBS_matches", -999);
    arbol.newBranch<int>("n_gen_q_double_matches", -999);
    arbol.newBranch<bool>("gen_lep_is_reco_match", false);
    arbol.newBranch<bool>("gen_found_all", false);
    TList* runs = new TList();
    TList* lumis = new TList();

    // Initialize Cutflow
    Histflow cutflow = Histflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVector>("gen_ld_q_p4");
    cutflow.globals.newVar<LorentzVector>("gen_tr_q_p4");
    cutflow.globals.newVar<LorentzVector>("gen_lep_p4");
    cutflow.globals.newVar<LorentzVector>("lep_p4");
    cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
    cutflow.globals.newVar<LorentzVectors>("skim_veto_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("skim_loose_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("skim_tight_lep_p4s", {});
    cutflow.globals.newVar<Integers>("skim_good_fatjet_idxs", {});
    std::vector<std::string> cuts;

    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCutflow();

    Cut* select_leps = new SelectLeptonsNoUL("SelectLeptonsNoUL", analysis);
    cutflow.replace("SelectLeptons", select_leps);

    Cut* has_1lep = new Has1LepNoUL("Has1TightLepNoUL", analysis);
    cutflow.replace("Has1TightLep", has_1lep);

    Cut* ewk_only = new LambdaCut(
        "EWKWLNuOnly",
        [&]()
        {
            // TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            // return (file_name.Contains("EWKW") && file_name.Contains("WToLNu"));
            return true;
        }
    );
    cutflow.insert("Bookkeeping", ewk_only, Right);

    Cut* search_lhe = new LambdaCut(
        "FindLHEQuarks",
        [&]()
        {
            arbol.setLeaf<int>("lhe_parton1_pdgID", nt.LHEPart_pdgId().at(0));
            arbol.setLeaf<int>("lhe_parton2_pdgID", nt.LHEPart_pdgId().at(1));
            LorentzVector q1_p4 = nt.LHEPart_p4().at(4);
            LorentzVector q2_p4 = nt.LHEPart_p4().at(5);
            int q1_pdgID = nt.LHEPart_pdgId().at(4);
            int q2_pdgID = nt.LHEPart_pdgId().at(5);
            double M_qq = (q1_p4 + q2_p4).M();
            arbol.setLeaf<double>("lhe_M_qq", M_qq);
            arbol.setLeaf<double>("lhe_deta_qq", q1_p4.eta() - q2_p4.eta());
            if (q1_p4.pt() > q2_p4.pt())
            {
                arbol.setLeaf<int>("lhe_ld_q_pdgID", q1_pdgID);
                arbol.setLeaf<int>("lhe_tr_q_pdgID", q2_pdgID);
            }
            else
            {
                arbol.setLeaf<int>("lhe_ld_q_pdgID", q2_pdgID);
                arbol.setLeaf<int>("lhe_tr_q_pdgID", q1_pdgID);
            }
            LorentzVector lep_p4 = nt.LHEPart_p4().at(2);
            LorentzVector nu_p4 = nt.LHEPart_p4().at(3);
            double M_lnu = (lep_p4 + nu_p4).M();
            arbol.setLeaf<double>("lhe_M_lnu", M_lnu);
            arbol.setLeaf<int>("lhe_lep_pdgID", nt.LHEPart_pdgId().at(2));
            arbol.setLeaf<double>("lhe_M_lnuqq", (lep_p4 + nu_p4 + q1_p4 + q2_p4).M());
            double charge_qq = getChargeQQ(q1_pdgID, q2_pdgID);
            if (fabs(charge_qq) == 1 && M_qq >= 70 && M_qq < 90 && M_lnu >= 70 && M_lnu < 90)
            {
                arbol.setLeaf<bool>("lhe_is_WW", true);
                arbol.setLeaf<bool>("lhe_is_VBSW", false);
            }
            else if (M_qq >= 95)
            {
                arbol.setLeaf<bool>("lhe_is_WW", false);
                arbol.setLeaf<bool>("lhe_is_VBSW", true);
            }
            return true;
        }
    );
    cutflow.insert(ewk_only, search_lhe, Right);

    Cut* search_gen = new LambdaCut(
        "FindGenParticles",
        [&]()
        {
            int gen_lep_pdgID = -999;
            LorentzVector gen_lep_p4;
            Integers gen_W_q;
            Integers gen_vbs_q;
            int n_primordial_W = 0;
            int n_primordial_q = 0;
            for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); ++gen_i)
            {
                unsigned int gen_mother_i = nt.GenPart_genPartIdxMother().at(gen_i);
                if (gen_mother_i < 0 || gen_mother_i > nt.nGenPart()) { continue; }
                int gen_pdgID = nt.GenPart_pdgId().at(gen_i);
                int gen_status = nt.GenPart_status().at(gen_i);
                int gen_mother_pdgID = nt.GenPart_pdgId().at(gen_mother_i);
                int gen_mother_status = nt.GenPart_status().at(gen_mother_i);
                LorentzVector gen_p4 = nt.GenPart_p4().at(gen_i);
                switch (abs(gen_pdgID))
                {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    if (gen_mother_i == 0)
                    {
                        n_primordial_q += 1;
                    }
                    if (gen_status == 23)
                    {
                        if (abs(gen_mother_pdgID) == 24 && gen_mother_status == 62)
                        {
                            // Found q or q' from a W
                            gen_W_q.push_back(gen_i);
                        }
                        else if (gen_mother_i == 0 && abs(gen_mother_pdgID) <= 5)
                        {
                            // Found a VBS quark
                            gen_vbs_q.push_back(gen_i);
                        }
                    }
                    break;
                case 11: // electron
                case 13: // muon
                case 15: // tau
                    if ((gen_status == 1 || gen_status == 2 || gen_status == 23) 
                        && (abs(gen_mother_pdgID) == 24))
                    {
                        // Found lepton from a W
                        gen_lep_p4 = gen_p4;
                        gen_lep_pdgID = gen_pdgID;
                    }
                    break;
                case 24:
                    if (gen_mother_i == 0)
                    {
                        n_primordial_W += 1;
                    }
                default:
                    continue;
                    break;
                }
            }
            LorentzVector q1_p4, q2_p4;
            int q1_idx = 0;
            int q2_idx = 0;
            if (n_primordial_W == 1 && n_primordial_q == 2 && gen_vbs_q.size() == 2)
            {
                arbol.setLeaf<bool>("gen_is_VBSW", true);
                arbol.setLeaf<bool>("gen_is_WW", false);
                q1_idx = gen_vbs_q.at(0);
                q2_idx = gen_vbs_q.at(1);
                q1_p4 = nt.GenPart_p4().at(q1_idx);
                q2_p4 = nt.GenPart_p4().at(q2_idx);
            }
            else if (gen_W_q.size() == 2)
            {
                arbol.setLeaf<bool>("gen_is_VBSW", false);
                arbol.setLeaf<bool>("gen_is_WW", true);
                q1_idx = gen_W_q.at(0);
                q2_idx = gen_W_q.at(1);
                q1_p4 = nt.GenPart_p4().at(q1_idx);
                q2_p4 = nt.GenPart_p4().at(q2_idx);
            }
            int gen_ld_q_idx = (q1_p4.pt() > q2_p4.pt()) ? q1_idx : q2_idx;
            int gen_tr_q_idx = (q1_p4.pt() > q2_p4.pt()) ? q2_idx : q1_idx;
            LorentzVector gen_ld_q_p4 = (q1_p4.pt() > q2_p4.pt()) ? q1_p4 : q2_p4;
            LorentzVector gen_tr_q_p4 = (q1_p4.pt() > q2_p4.pt()) ? q2_p4 : q1_p4;
            arbol.setLeaf<int>("gen_ld_q_pdgID", nt.GenPart_pdgId().at(gen_ld_q_idx));
            arbol.setLeaf<double>("gen_ld_q_pt", gen_ld_q_p4.pt());
            arbol.setLeaf<double>("gen_ld_q_eta", gen_ld_q_p4.eta());
            arbol.setLeaf<double>("gen_ld_q_phi", gen_ld_q_p4.phi());
            arbol.setLeaf<int>("gen_tr_q_pdgID", nt.GenPart_pdgId().at(gen_tr_q_idx));
            arbol.setLeaf<double>("gen_tr_q_pt", gen_tr_q_p4.pt());
            arbol.setLeaf<double>("gen_tr_q_eta", gen_tr_q_p4.eta());
            arbol.setLeaf<double>("gen_tr_q_phi", gen_tr_q_p4.phi());
            arbol.setLeaf<int>("gen_lep_pdgID", gen_lep_pdgID);
            arbol.setLeaf<double>("gen_lep_pt", gen_lep_p4.pt());
            arbol.setLeaf<double>("gen_lep_eta", gen_lep_p4.eta());
            arbol.setLeaf<double>("gen_lep_phi", gen_lep_p4.phi());
            arbol.setLeaf<bool>("gen_found_all", (gen_lep_pdgID != -999) && (gen_W_q.size() == 2 || gen_vbs_q.size() == 2));
            cutflow.globals.setVal<LorentzVector>("gen_ld_q_p4", gen_ld_q_p4);
            cutflow.globals.setVal<LorentzVector>("gen_tr_q_p4", gen_tr_q_p4);
            cutflow.globals.setVal<LorentzVector>("gen_lep_p4", gen_lep_p4);
            return true;
        }
    );
    cutflow.insert(search_lhe, search_gen, Right);
    cuts.push_back(search_gen->name);

    // Find leptons
    Cut* findleps_skim = new LambdaCut(
        "SKIM_FindLeptons", 
        [&]() 
        { 
            LorentzVectors skim_veto_lep_p4s;
            LorentzVectors skim_loose_lep_p4s;
            LorentzVectors skim_tight_lep_p4s;
            for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
            {
                LorentzVector lep_p4 = nt.Electron_p4().at(elec_i);
                if (ttH::electronID(elec_i, ttH::IDveto, nt.year())) { skim_veto_lep_p4s.push_back(lep_p4); }
                if (ttH::electronID(elec_i, ttH::IDfakable, nt.year())) { skim_loose_lep_p4s.push_back(lep_p4); }
                if (ttH::electronID(elec_i, ttH::IDtight, nt.year())) { skim_tight_lep_p4s.push_back(lep_p4); }
            }
            for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
            {
                LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
                if (ttH::muonID(muon_i, ttH::IDveto, nt.year())) { skim_veto_lep_p4s.push_back(lep_p4); }
                if (ttH::muonID(muon_i, ttH::IDfakable, nt.year())) { skim_loose_lep_p4s.push_back(lep_p4); }
                if (ttH::muonID(muon_i, ttH::IDtight, nt.year())) { skim_tight_lep_p4s.push_back(lep_p4); }
            }
            cutflow.globals.setVal<LorentzVectors>("skim_veto_lep_p4s", skim_veto_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("skim_loose_lep_p4s", skim_loose_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("skim_tight_lep_p4s", skim_tight_lep_p4s);
            return true;
        }
    );
    cutflow.insert(search_gen, findleps_skim, Right);

    // Geq1VetoLep
    Cut* geq1vetolep_skim = new LambdaCut(
        "SKIM_Geq1VetoLep", [&]() { return cutflow.globals.getVal<LorentzVectors>("skim_veto_lep_p4s").size() >= 1; }
    );
    cutflow.insert(findleps_skim, geq1vetolep_skim, Right);
    cuts.push_back(geq1vetolep_skim->name);

    // Geq2Jets
    Cut* geq2jets_skim = new LambdaCut(
        "SKIM_Geq2Jets", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("skim_veto_lep_p4s");
            int n_jets = 0;
            for (unsigned int jet_i = 0; jet_i < nt.nJet(); jet_i++)
            {
                LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, jet_p4) < 0.4)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap && nt.Jet_pt().at(jet_i) > 20)
                {
                    n_jets++;
                }
            }
            return (n_jets >= 2);
        }
    );
    cutflow.insert(geq1vetolep_skim, geq2jets_skim, Right);
    cuts.push_back(geq2jets_skim->name);

    // Geq1FatJet
    Cut* geq1fatjet_skim = new LambdaCut(
        "SKIM_Geq1FatJetNoVetoLepOverlap", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("skim_veto_lep_p4s");
            int n_fatjets = 0;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap 
                    && nt.FatJet_mass().at(fatjet_i) > 10 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 10 
                    && nt.FatJet_pt().at(fatjet_i) > 200)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(geq2jets_skim, geq1fatjet_skim, Right);
    cuts.push_back(geq1fatjet_skim->name);

    // Exactly1Lep
    Cut* exactly1tightlep_postskim = new LambdaCut(
        "POSTSKIM_Exactly1TightLep", 
        [&]() 
        { 
            int n_loose_leps = cutflow.globals.getVal<LorentzVectors>("skim_loose_lep_p4s").size();
            int n_tight_leps = cutflow.globals.getVal<LorentzVectors>("skim_tight_lep_p4s").size();
            return (n_loose_leps == 1 && n_tight_leps == 1);
        }
    );
    cutflow.insert(geq1fatjet_skim, exactly1tightlep_postskim, Right);
    cuts.push_back(exactly1tightlep_postskim->name);

    // Geq1FatJet
    Cut* geq1fatjet_postskim = new LambdaCut(
        "POSTSKIM_Geq1FatJetNoTightLepOverlap", 
        [&]() 
        { 
            LorentzVector lep_p4 = cutflow.globals.getVal<LorentzVectors>("skim_tight_lep_p4s").at(0);
            int n_fatjets = 0;
            double hbbjet_score = -999.;
            LorentzVector hbbjet_p4;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                // Basic requirements
                if (nt.FatJet_pt().at(fatjet_i) <= 250) { continue; }
                if (nt.FatJet_mass().at(fatjet_i) <= 50) { continue; }
                if (nt.FatJet_msoftdrop().at(fatjet_i) <= 40) { continue; }
                // Remove lepton overlap
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8) { continue; }

                // Count good fat jets
                n_fatjets++;
                double pnet_xbb = nt.FatJet_particleNetMD_Xbb().at(fatjet_i);
                double pnet_qcd = nt.FatJet_particleNetMD_QCD().at(fatjet_i);
                double xbb_score = pnet_xbb/(pnet_xbb + pnet_qcd);
                if (xbb_score > hbbjet_score)
                {
                    hbbjet_p4 = fatjet_p4;
                    hbbjet_score = xbb_score;
                }
            }
            if (n_fatjets >= 1)
            {
                cutflow.globals.setVal<LorentzVector>("hbbjet_p4", hbbjet_p4);
                return true;
            }
            else
            {
                return false;
            }
        }
    );
    cutflow.insert(exactly1tightlep_postskim, geq1fatjet_postskim, Right);
    cuts.push_back(geq1fatjet_postskim->name);

    Cut* STgt800_postskim = new LambdaCut(
        "POSTSKIM_STgt800", 
        [&]() 
        { 
            double ST = (
                cutflow.globals.getVal<LorentzVectors>("skim_tight_lep_p4s").at(0).pt()
                + cutflow.globals.getVal<LorentzVector>("hbbjet_p4").pt()
                + nt.MET_pt()
            );
            return (ST > 800);
        }
    );
    cutflow.insert(geq1fatjet_postskim, STgt800_postskim, Right);
    cuts.push_back(STgt800_postskim->name);

    cuts.push_back("SelectVBSJetsMaxE");
    cuts.push_back("ApplyAk4GlobalBVeto");
    cuts.push_back("XbbGt0p9_MSDLt150");
    cuts.push_back("STGt1500");

    // DEBUG

    Cut* skim_fatjet_overlap = new LambdaCut(
        "SKIM_SelectFatJets", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("skim_veto_lep_p4s");
            Integers good_fatjet_idxs;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap)
                {
                    good_fatjet_idxs.push_back(fatjet_i);
                }
            }
            cutflow.globals.setVal<Integers>("skim_good_fatjet_idxs", good_fatjet_idxs);
            return (good_fatjet_idxs.size() >= 1);
        }
    );
    cutflow.insert(geq2jets_skim, skim_fatjet_overlap, Right);
    cuts.push_back(skim_fatjet_overlap->name);

    Cut* skim_fatjet_pt_sel = new LambdaCut(
        "SKIM_Geq1FatJet_pTgt200", 
        [&]() 
        { 
            int n_fatjets = 0;
            for (auto& fatjet_i : cutflow.globals.getVal<Integers>("skim_good_fatjet_idxs"))
            {
                if (nt.FatJet_pt().at(fatjet_i) > 200) { n_fatjets++; }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(skim_fatjet_overlap, skim_fatjet_pt_sel, Right);
    cuts.push_back(skim_fatjet_pt_sel->name);

    Cut* skim_fatjet_mass_sel = new LambdaCut(
        "SKIM_Geq1FatJet_Mgt10", 
        [&]() 
        { 
            int n_fatjets = 0;
            for (auto& fatjet_i : cutflow.globals.getVal<Integers>("skim_good_fatjet_idxs"))
            {
                if (nt.FatJet_pt().at(fatjet_i) > 200
                    && nt.FatJet_mass().at(fatjet_i) > 10)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(skim_fatjet_pt_sel, skim_fatjet_mass_sel, Right);
    cuts.push_back(skim_fatjet_mass_sel->name);

    Cut* skim_fatjet_msoftdrop_sel = new LambdaCut(
        "SKIM_Geq1FatJet_MSDgt10", 
        [&]() 
        { 
            int n_fatjets = 0;
            for (auto& fatjet_i : cutflow.globals.getVal<Integers>("skim_good_fatjet_idxs"))
            {
                if (nt.FatJet_pt().at(fatjet_i) > 200
                    && nt.FatJet_mass().at(fatjet_i) > 10 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 10)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(skim_fatjet_mass_sel, skim_fatjet_msoftdrop_sel, Right);
    cuts.push_back(skim_fatjet_msoftdrop_sel->name);

    Cut* postskim_fatjet_pt_sel = new LambdaCut(
        "POSTSKIM_Geq1FatJet_pTgt250", 
        [&]() 
        { 
            int n_fatjets = 0;
            for (auto& fatjet_i : cutflow.globals.getVal<Integers>("skim_good_fatjet_idxs"))
            {
                if (nt.FatJet_pt().at(fatjet_i) > 250) { n_fatjets++; }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(exactly1tightlep_postskim, postskim_fatjet_pt_sel, Right);
    cuts.push_back(postskim_fatjet_pt_sel->name);

    Cut* postskim_fatjet_mass_sel = new LambdaCut(
        "POSTSKIM_Geq1FatJet_Mgt50", 
        [&]() 
        { 
            int n_fatjets = 0;
            for (auto& fatjet_i : cutflow.globals.getVal<Integers>("skim_good_fatjet_idxs"))
            {
                if (nt.FatJet_pt().at(fatjet_i) > 250
                    && nt.FatJet_mass().at(fatjet_i) > 50)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(postskim_fatjet_pt_sel, postskim_fatjet_mass_sel, Right);
    cuts.push_back(postskim_fatjet_mass_sel->name);

    Cut* postskim_fatjet_msoftdrop_sel = new LambdaCut(
        "POSTSKIM_Geq1FatJet_MSDgt40", 
        [&]() 
        { 
            int n_fatjets = 0;
            for (auto& fatjet_i : cutflow.globals.getVal<Integers>("skim_good_fatjet_idxs"))
            {
                if (nt.FatJet_pt().at(fatjet_i) > 250
                    && nt.FatJet_mass().at(fatjet_i) > 50 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 40)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(postskim_fatjet_mass_sel, postskim_fatjet_msoftdrop_sel, Right);
    cuts.push_back(postskim_fatjet_msoftdrop_sel->name);

    // Initialize hists
    TH1D* parton1_abs_pdgID_hist = new TH1D("parton1_abs_pdgID", "lhe_parton1_pdgID", 6, 1, 7);
    TH1D* parton2_abs_pdgID_hist = new TH1D("parton2_abs_pdgID", "lhe_parton2_pdgID", 6, 1, 7);
    TH1D* partons_abs_pdgID_hist = new TH1D("partons_abs_pdgID", "partons_pdgID", 15, 0, 15);
    TH1D* parton1_pdgID_hist = new TH1D("parton1_pdgID", "lhe_parton1_pdgID", 14, -7, 7);
    TH1D* parton2_pdgID_hist = new TH1D("parton2_pdgID", "lhe_parton2_pdgID", 14, -7, 7);
    TH1D* lhe_M_qq_genclass_hist = new TH1D("lhe_M_qq_genclass", "lhe_M_qq", 150, 0, 150);
    TH1D* lhe_M_qq_lheclass_hist = new TH1D("lhe_M_qq_lheclass", "lhe_M_qq", 150, 0, 150);
    
    std::vector<std::pair<TH1D*, int>> hist_flavor_pairs;
    std::vector<TString> flavor_names = {"q", "d", "u", "s", "c", "b"};
    for (auto b : {"inb", "nob"})
    {
        for (auto i : {0, 1, 2, 3, 4, 5})
        {
            TString q = flavor_names.at(i);
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_VBSW_ld_"+q+"_pt_"+b, "gen_ld_q_pt", 1500, 0, 1500), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_VBSW_tr_"+q+"_pt_"+b, "gen_tr_q_pt", 1500, 0, 1500), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_WW_ld_"+q+"_pt_"+b, "gen_ld_q_pt", 1500, 0, 1500), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_WW_tr_"+q+"_pt_"+b, "gen_tr_q_pt", 1500, 0, 1500), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_VBSW_ld_"+q+"_eta_"+b, "gen_ld_q_eta", 2000, -10, 10), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_VBSW_tr_"+q+"_eta_"+b, "gen_tr_q_eta", 2000, -10, 10), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_WW_ld_"+q+"_eta_"+b, "gen_ld_q_eta", 2000, -10, 10), i));
            hist_flavor_pairs.push_back(std::make_pair(new TH1D("gen_WW_tr_"+q+"_eta_"+b, "gen_tr_q_eta", 2000, -10, 10), i));
        }
    }

    auto has_incoming_b = [&]() {
        return (
            abs(arbol.getLeaf<int>("lhe_parton1_pdgID")) == 5 
            || abs(arbol.getLeaf<int>("lhe_parton2_pdgID")) == 5
        );
    };

    // Book hists
    for (auto cut : cuts)
    {
        cutflow.bookHist1D<TH1D>(
            cut, parton1_abs_pdgID_hist, 
            [&]() { return true; },
            [&]() { return abs(arbol.getLeaf<int>("lhe_parton1_pdgID")); }
        );
        cutflow.bookHist1D<TH1D>(
            cut, parton2_abs_pdgID_hist, 
            [&]() { return true; },
            [&]() { return abs(arbol.getLeaf<int>("lhe_parton2_pdgID")); }
        );
        cutflow.bookHist1D<TH1D>(
            cut, partons_abs_pdgID_hist, 
            [&]() { return true; },
            [&]() 
            {
                return getPartonPairNum(
                    abs(arbol.getLeaf<int>("lhe_parton1_pdgID")),
                    abs(arbol.getLeaf<int>("lhe_parton2_pdgID"))
                );
            }
        );
        cutflow.bookHist1D<TH1D>(
            cut, parton1_pdgID_hist, 
            [&]() { return true; },
            [&]() { return arbol.getLeaf<int>("lhe_parton1_pdgID"); }
        );
        cutflow.bookHist1D<TH1D>(
            cut, parton2_pdgID_hist, 
            [&]() { return true; },
            [&]() { return arbol.getLeaf<int>("lhe_parton2_pdgID"); }
        );

        for (auto hist_flavor_pair : hist_flavor_pairs)
        {
            TH1D* hist = hist_flavor_pair.first;
            int flavor = hist_flavor_pair.second;
            TString name = hist->GetName();
            bool inb = name.Contains("inb");
            std::string genclass = name.Contains("WW") ? "gen_is_WW" : "gen_is_VBSW";
            std::string ldtr = name.Contains("_ld_") ? "ld" : "tr";
            cutflow.bookHist1D<TH1D>(
                cut, hist, 
                [&, flavor, inb, genclass]() 
                { 
                    return (
                        arbol.getLeaf<bool>(genclass) 
                        && arbol.getLeaf<bool>("gen_found_all")
                        && (flavor == 0 || abs(arbol.getLeaf<int>("gen_"+ldtr+"_q_pdgID")) == flavor)
                        && (inb ? has_incoming_b() : !has_incoming_b())
                    );
                },
                [&, hist]() { return arbol.getLeaf<double>(hist->GetTitle()); }
            );
        }

        cutflow.bookHist1D<TH1D>(
            cut, lhe_M_qq_genclass_hist, 
            [&]() { return arbol.getLeaf<bool>("gen_is_WW") && arbol.getLeaf<bool>("gen_found_all"); },
            [&]() { return arbol.getLeaf<double>("lhe_M_qq"); }
        );
        cutflow.bookHist1D<TH1D>(
            cut, lhe_M_qq_lheclass_hist, 
            [&]() { return arbol.getLeaf<bool>("lhe_is_WW"); },
            [&]() { return arbol.getLeaf<double>("lhe_M_qq"); }
        );
    }

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
            // Store metadata ttrees
            TTree* runtree = ((TTree*)ttree->GetCurrentFile()->Get("Runs"))->CloneTree();
            runtree->SetDirectory(0);
            runs->Add(runtree);
            TTree* lumitree = ((TTree*)ttree->GetCurrentFile()->Get("LuminosityBlocks"))->CloneTree();
            lumitree->SetDirectory(0);
            lumis->Add(lumitree);
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // reset branches and globals
                arbol.resetBranches();
                cutflow.globals.resetVars();
                // run cutflow
                nt.GetEntry(entry);
                cutflow.run();
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data) { cutflow.print(); }

    cutflow.writeHists(arbol.tfile);

    TTree* merged_runs = TTree::MergeTrees(runs);
    merged_runs->SetName("Runs");
    TTree* merged_lumis = TTree::MergeTrees(lumis);
    merged_lumis->SetName("LuminosityBlocks");

    arbol.tfile->cd();
    merged_runs->Write();
    merged_lumis->Write();
    arbol.write();
    return 0;
}
