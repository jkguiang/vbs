#include "vbswh/collections.h"
#include "vbswh/cuts.h"
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

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);
    arbol.newBranch<int>("lhe_parton1_pdgID", -999);
    arbol.newBranch<int>("lhe_parton2_pdgID", -999);
    arbol.newBranch<int>("lhe_lep_pdgID", -999);
    arbol.newBranch<int>("lhe_ld_q_pdgID", -999);
    arbol.newBranch<int>("lhe_tr_q_pdgID", -999);
    arbol.newBranch<double>("lhe_ld_q_pt", -999);
    arbol.newBranch<double>("lhe_tr_q_pt", -999);
    arbol.newBranch<double>("lhe_ld_q_eta", -999);
    arbol.newBranch<double>("lhe_tr_q_eta", -999);
    arbol.newBranch<double>("lhe_M_qq", -999);
    arbol.newBranch<double>("lhe_deta_qq", -999);
    arbol.newBranch<double>("lhe_M_lnu", -999);
    arbol.newBranch<double>("lhe_M_lnuqq", -999);
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

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");
    cutflow.globals.newVar<LorentzVector>("gen_ld_q_p4");
    cutflow.globals.newVar<LorentzVector>("gen_tr_q_p4");
    cutflow.globals.newVar<LorentzVector>("gen_lep_p4");

    // Pack above into VBSWH struct (also adds branches)
    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    analysis.initCutflow();

    Cut* search_lhe = new LambdaCut(
        "DEBUG_FindLHEQuarks",
        [&]()
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            if (file_name.Contains("EWKW") || file_name.Contains("EWKZ") || file_name.Contains("WWTo"))
            {
                arbol.setLeaf<int>("lhe_parton1_pdgID", nt.LHEPart_pdgId().at(0));
                arbol.setLeaf<int>("lhe_parton2_pdgID", nt.LHEPart_pdgId().at(1));
                LorentzVector q1_p4 = nt.LHEPart_p4().at(4);
                LorentzVector q2_p4 = nt.LHEPart_p4().at(5);
                arbol.setLeaf<double>("lhe_M_qq", (q1_p4 + q2_p4).M());
                arbol.setLeaf<double>("lhe_deta_qq", q1_p4.eta() - q2_p4.eta());
                if (q1_p4.pt() > q2_p4.pt())
                {
                    arbol.setLeaf<int>("lhe_ld_q_pdgID", nt.LHEPart_pdgId().at(4));
                    arbol.setLeaf<int>("lhe_tr_q_pdgID", nt.LHEPart_pdgId().at(5));
                    arbol.setLeaf<double>("lhe_ld_q_pt", nt.LHEPart_pt().at(4));
                    arbol.setLeaf<double>("lhe_tr_q_pt", nt.LHEPart_pt().at(5));
                    arbol.setLeaf<double>("lhe_ld_q_eta", nt.LHEPart_eta().at(4));
                    arbol.setLeaf<double>("lhe_tr_q_eta", nt.LHEPart_eta().at(5));
                }
                else
                {
                    arbol.setLeaf<int>("lhe_ld_q_pdgID", nt.LHEPart_pdgId().at(5));
                    arbol.setLeaf<int>("lhe_tr_q_pdgID", nt.LHEPart_pdgId().at(4));
                    arbol.setLeaf<double>("lhe_ld_q_pt", nt.LHEPart_pt().at(5));
                    arbol.setLeaf<double>("lhe_tr_q_pt", nt.LHEPart_pt().at(4));
                    arbol.setLeaf<double>("lhe_ld_q_eta", nt.LHEPart_eta().at(5));
                    arbol.setLeaf<double>("lhe_tr_q_eta", nt.LHEPart_eta().at(4));
                }
                if (file_name.Contains("To1L1Nu") || file_name.Contains("ToLNu"))
                {
                    LorentzVector lep_p4 = nt.LHEPart_p4().at(2);
                    LorentzVector nu_p4 = nt.LHEPart_p4().at(3);
                    arbol.setLeaf<double>("lhe_M_lnu", (lep_p4 + nu_p4).M());
                    arbol.setLeaf<int>("lhe_lep_pdgID", nt.LHEPart_pdgId().at(2));
                    arbol.setLeaf<double>("lhe_M_lnuqq", (lep_p4 + nu_p4 + q1_p4 + q2_p4).M());
                }
            }
            return true;
        }
    );
    cutflow.insert("Bookkeeping", search_lhe, Right);

    Cut* search_gen = new LambdaCut(
        "DEBUG_FindGenParticles",
        [&]()
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            if (file_name.Contains("EWKWPlus") || file_name.Contains("EWKWMinus"))
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
                            else if (gen_mother_i == 0)
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
            }
            return true;
        }
    );
    cutflow.insert(search_lhe, search_gen, Right);

    Cut* match_gen = new LambdaCut(
        "DEBUG_MatchGenParticles",
        [&]()
        {
            if (!arbol.getLeaf<bool>("gen_found_all")) { return true; }
            LorentzVector gen_ld_q_p4 = cutflow.globals.getVal<LorentzVector>("gen_ld_q_p4");
            LorentzVector gen_tr_q_p4 = cutflow.globals.getVal<LorentzVector>("gen_tr_q_p4");
            LorentzVector gen_lep_p4 = cutflow.globals.getVal<LorentzVector>("gen_lep_p4");
            LorentzVector ld_vbs_p4 = cutflow.globals.getVal<LorentzVector>("ld_vbsjet_p4");
            LorentzVector tr_vbs_p4 = cutflow.globals.getVal<LorentzVector>("tr_vbsjet_p4");
            LorentzVector lep_p4 = cutflow.globals.getVal<LorentzVector>("lep_p4");
            LorentzVector hbb_p4 = cutflow.globals.getVal<LorentzVector>("hbbjet_p4");
            arbol.setLeaf<bool>("gen_lep_is_reco_match", ROOT::Math::VectorUtil::DeltaR(gen_lep_p4, lep_p4) <= 0.4);

            int n_gen_q_in_hbb_cone = 0;
            bool ld_q_in_hbb_cone = false;
            bool tr_q_in_hbb_cone = false;
            if (ROOT::Math::VectorUtil::DeltaR(gen_ld_q_p4, hbb_p4) <= 0.8) 
            { 
                ld_q_in_hbb_cone = true;
                n_gen_q_in_hbb_cone++; 
            }
            if (ROOT::Math::VectorUtil::DeltaR(gen_tr_q_p4, hbb_p4) <= 0.8) 
            { 
                tr_q_in_hbb_cone = true;
                n_gen_q_in_hbb_cone++; 
            }
            arbol.setLeaf<int>("n_gen_q_in_hbb_cone", n_gen_q_in_hbb_cone);
            arbol.setLeaf<bool>("gen_ld_q_in_hbb_cone", ld_q_in_hbb_cone);
            arbol.setLeaf<bool>("gen_tr_q_in_hbb_cone", tr_q_in_hbb_cone);

            double ld_ld_dR = ROOT::Math::VectorUtil::DeltaR(gen_ld_q_p4, ld_vbs_p4);
            double tr_tr_dR = ROOT::Math::VectorUtil::DeltaR(gen_tr_q_p4, tr_vbs_p4);
            double ld_tr_dR = ROOT::Math::VectorUtil::DeltaR(gen_ld_q_p4, tr_vbs_p4);
            double tr_ld_dR = ROOT::Math::VectorUtil::DeltaR(gen_tr_q_p4, ld_vbs_p4);

            int n_gen_q_double_matches = 0;
            if ((ld_ld_dR <= 0.4 && tr_tr_dR <= 0.4) || (ld_tr_dR <= 0.4 && tr_ld_dR <= 0.4))
            {
                arbol.setLeaf<float>("n_gen_q_VBS_matches", 2);
                if (ld_q_in_hbb_cone) { n_gen_q_double_matches++; }
                if (tr_q_in_hbb_cone) { n_gen_q_double_matches++; }
            }
            else if ((ld_ld_dR <= 0.4 && tr_ld_dR <= 0.4) || (ld_tr_dR <= 0.4 && tr_tr_dR <= 0.4))
            {
                arbol.setLeaf<float>("n_gen_q_VBS_matches", 1.5);
                if (ld_q_in_hbb_cone) { n_gen_q_double_matches++; }
                if (tr_q_in_hbb_cone) { n_gen_q_double_matches++; }
            }
            else if (ld_ld_dR <= 0.4 || ld_tr_dR <= 0.4)
            {
                arbol.setLeaf<float>("n_gen_q_VBS_matches", 1);
                if (ld_q_in_hbb_cone) { n_gen_q_double_matches++; }
            }
            else if (tr_tr_dR <= 0.4 || tr_ld_dR <= 0.4)
            {
                arbol.setLeaf<float>("n_gen_q_VBS_matches", 1);
                if (tr_q_in_hbb_cone) { n_gen_q_double_matches++; }
            }
            else
            {
                arbol.setLeaf<float>("n_gen_q_VBS_matches", 0);
            }
            arbol.setLeaf<int>("n_gen_q_double_matches", n_gen_q_double_matches);
            return true;
        }
    );
    cutflow.insert("SelectVBSJetsMaxE", match_gen, Right);

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
                bool passed = cutflow.run("SelectVBSJetsMaxE");
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
