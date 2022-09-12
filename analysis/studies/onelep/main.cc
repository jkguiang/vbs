#include "../core.h"
#include "../scalefactors.h"
#include "../vbswh.h"
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
    // DEBUG
    arbol.newBranch<int>("lhe_ld_q_pdgID", -999);
    arbol.newBranch<int>("lhe_tr_q_pdgID", -999);
    arbol.newBranch<int>("lhe_lep_pdgID", -999);
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
    // DEBUG
    cutflow.globals.newVar<LorentzVector>("gen_ld_q_p4");
    cutflow.globals.newVar<LorentzVector>("gen_tr_q_p4");
    cutflow.globals.newVar<LorentzVector>("gen_lep_p4");

    // Pack above into VBSWH struct (also adds branches)
    VBSWH::Analysis analysis = VBSWH::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();

    // Bookkeeping
    Cut* bookkeeping = new Core::Bookkeeping("Bookkeeping", analysis);
    cutflow.setRoot(bookkeeping);

    // Lepton selection
    Cut* select_leps = new Core::SelectLeptons("SelectLeptons", analysis);
    cutflow.insert(bookkeeping, select_leps, Right);

    // == 1 lepton selection
    Cut* has_1lep = new VBSWH::Has1Lep("Has1TightLep", analysis);
    cutflow.insert(select_leps, has_1lep, Right);

    // Lepton has pT > 40
    Cut* lep_pt_gt40 = new LambdaCut(
        "LepPtGt40", [&]() { return arbol.getLeaf<double>("lep_pt") >= 40; }
    );
    cutflow.insert(has_1lep, lep_pt_gt40, Right);

    // Single-lepton triggers
    Cut* lep_triggers = new VBSWH::Passes1LepTriggers("Passes1LepTriggers", analysis);
    cutflow.insert(lep_pt_gt40, lep_triggers, Right);

    // Fat jet selection
    Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", analysis);
    cutflow.insert(lep_triggers, select_fatjets, Right);

    // Geq1FatJet
    Cut* geq1fatjet = new LambdaCut(
        "Geq1FatJet", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 1; }
    );
    cutflow.insert(select_fatjets, geq1fatjet, Right);

    // Hbb selection
    Cut* select_hbbjet = new VBSWH::SelectHbbFatJet("SelectHbbFatJet", analysis, true);
    cutflow.insert(geq1fatjet, select_hbbjet, Right);

    // Jet selection
    Cut* select_jets = new VBSWH::SelectJetsNoHbbOverlap("SelectJetsNoHbbOverlap", analysis);
    cutflow.insert(select_hbbjet, select_jets, Right);

    // Global AK4 b-veto
    Cut* save_ak4bveto = new LambdaCut(
        "SaveAk4GlobalBVeto", 
        [&]()
        {
            bool passes_bveto = true;
            for (auto& btag : cutflow.globals.getVal<Doubles>("good_jet_btags"))
            {
                if (btag > gconf.WP_DeepFlav_medium) 
                { 
                    passes_bveto = false;
                    break;
                }
            }
            arbol.setLeaf<bool>("passes_bveto", passes_bveto);
            return true;
        }
    );
    cutflow.insert(select_jets, save_ak4bveto, Right);

    // VBS jet selection
    Cut* select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SelectVBSJetsMaxE", analysis);
    cutflow.insert(save_ak4bveto, select_vbsjets_maxE, Right);

    // Basic VBS jet requirements
    Cut* vbsjets_presel = new LambdaCut(
        "MjjGt500_detajjGt3", 
        [&]()
        {
            return arbol.getLeaf<double>("M_jj") > 500 && fabs(arbol.getLeaf<double>("deta_jj")) > 3;
        }
    );
    cutflow.insert(select_vbsjets_maxE, vbsjets_presel, Right);

    /* Splits trigger by lepton flavor to check eff
    // Single-electron channel
    Cut* is_elec = new LambdaCut(
        "IsElectron", [&]() { return abs(arbol.getLeaf<int>("lep_pdgID")) == 11; }
    );
    cutflow.insert(vbsjets_presel, is_elec, Right);

    // Single-electron triggers
    Cut* elec_triggers = new VBSWH::Passes1LepTriggers("Passes1ElecTriggers", analysis);
    cutflow.insert(is_elec, elec_triggers, Right);

    // Single-muon channel
    Cut* is_muon = new LambdaCut(
        "IsMuon", [&]() { return abs(arbol.getLeaf<int>("lep_pdgID")) == 13; }
    );
    cutflow.insert(is_elec, is_muon, Left);

    // Single-muon triggers
    Cut* muon_triggers = new VBSWH::Passes1LepTriggers("Passes1MuonTriggers", analysis);
    cutflow.insert(is_muon, muon_triggers, Right);
    */

    Cut* xbb_presel = new LambdaCut(
        "XbbGt0p3", [&]() { return arbol.getLeaf<double>("hbbjet_score") > 0.3; }
    );
    cutflow.insert(vbsjets_presel, xbb_presel, Right);

    Cut* apply_ak4bveto = new LambdaCut(
        "ApplyAk4GlobalBVeto", [&]() { return arbol.getLeaf<bool>("passes_bveto"); }
    );
    cutflow.insert(xbb_presel, apply_ak4bveto, Right);
    
    Cut* vbs_SR = new LambdaCut(
        "MjjGt600_detajjGt4", 
        [&]() 
        { 
            return (
                arbol.getLeaf<double>("M_jj") > 600 
                && fabs(arbol.getLeaf<double>("deta_jj")) > 4
            ); 
        }
    );
    cutflow.insert(apply_ak4bveto, vbs_SR, Right);

    Cut* ST_SR = new LambdaCut(
        "STGt900", [&]() { return arbol.getLeaf<double>("ST") > 900; }
    );
    cutflow.insert(vbs_SR, ST_SR, Right);

    Cut* Hbb_SR = new LambdaCut(
        "XbbGt0p9_MSDLt150", 
        [&]() 
        { 
            return (
                arbol.getLeaf<double>("hbbjet_score") > 0.9 
                && arbol.getLeaf<double>("hbbjet_msoftdrop") < 150
            ); 
        }
    );
    cutflow.insert(ST_SR, Hbb_SR, Right);

    Cut* ST_SRtight = new LambdaCut(
        "STGt1500", [&]() { return arbol.getLeaf<double>("ST") > 1500; }
    );
    cutflow.insert(Hbb_SR, ST_SRtight, Right);

    // DEBUG DEBUG DEBUG
    Cut* search_lhe = new LambdaCut(
        "DEBUG_FindLHEQuarks",
        [&]()
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            if (file_name.Contains("EWKW") || file_name.Contains("EWKZ") || file_name.Contains("WWTo"))
            {
                LorentzVector q1_p4 = nt.LHEPart_p4().at(4);
                LorentzVector q2_p4 = nt.LHEPart_p4().at(5);
                arbol.setLeaf<double>("lhe_M_qq", (q1_p4 + q2_p4).M());
                arbol.setLeaf<double>("lhe_deta_qq", q1_p4.eta() - q2_p4.eta());
                if (q1_p4.pt() > q2_p4.pt())
                {
                    arbol.setLeaf<int>("lhe_ld_q_pdgID", nt.LHEPart_pdgId().at(4));
                    arbol.setLeaf<int>("lhe_tr_q_pdgID", nt.LHEPart_pdgId().at(5));
                }
                else
                {
                    arbol.setLeaf<int>("lhe_ld_q_pdgID", nt.LHEPart_pdgId().at(5));
                    arbol.setLeaf<int>("lhe_tr_q_pdgID", nt.LHEPart_pdgId().at(4));
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
    cutflow.insert(bookkeeping, search_lhe, Right);

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
                for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); ++gen_i)
                {
                    unsigned int gen_mother_i = nt.GenPart_genPartIdxMother().at(gen_i);
                    if (gen_mother_i < 0 || gen_mother_i > nt.nGenPart()) { continue; }
                    int gen_pdgID = nt.GenPart_pdgId().at(gen_i);
                    int gen_status = nt.GenPart_status().at(gen_i);
                    int gen_mother_pdgID = nt.GenPart_pdgId().at(gen_mother_i);
                    LorentzVector gen_p4 = nt.GenPart_p4().at(gen_i);
                    switch (abs(gen_pdgID))
                    {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        if (gen_status == 23)
                        {
                            if (abs(gen_mother_pdgID) == 24)
                            {
                                // Found q or q' from a W
                                gen_W_q.push_back(gen_i);
                            }
                            else if (gen_mother_i == 0 || gen_mother_i == 1)
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
                    default:
                        continue;
                        break;
                    }
                }
                LorentzVector q1_p4, q2_p4;
                int q1_idx = 0;
                int q2_idx = 0;
                if (gen_vbs_q.size() == 2)
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
    cutflow.insert(select_vbsjets_maxE, match_gen, Right);

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
                bool passed = cutflow.run(select_vbsjets_maxE);
                // bool passed = cutflow.run(bookkeeping);
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
