// RAPIDO
#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "histflow.h"
// ROOT
#include "TString.h"
#include "TObject.h"
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

int main(int argc, char** argv) 
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);
    // LHE quark branches
    arbol.newBranch<int>("lhe_charge_qq", -999);
    arbol.newBranch<double>("lhe_deta_qq", -999);
    arbol.newBranch<int>("lhe_ld_q_pdgID", -999);
    arbol.newBranch<double>("lhe_ld_q_pt", -999);
    arbol.newBranch<double>("lhe_ld_q_eta", -999);
    arbol.newBranch<double>("lhe_ld_q_phi", -999);
    arbol.newBranch<int>("lhe_tr_q_pdgID", -999);
    arbol.newBranch<double>("lhe_tr_q_pt", -999);
    arbol.newBranch<double>("lhe_tr_q_eta", -999);
    arbol.newBranch<double>("lhe_tr_q_phi", -999);
    // LHE lepton branches
    arbol.newBranch<int>("lhe_lep_pdgID", -999);
    arbol.newBranch<double>("lhe_lep_pt", -999);
    arbol.newBranch<double>("lhe_lep_eta", -999);
    arbol.newBranch<double>("lhe_lep_phi", -999);
    // LHE neutrino branches
    arbol.newBranch<int>("lhe_nu_pdgID", -999);
    arbol.newBranch<double>("lhe_nu_pt", -999);
    arbol.newBranch<double>("lhe_nu_eta", -999);
    arbol.newBranch<double>("lhe_nu_phi", -999);
    // Other LHE kinematics
    arbol.newBranch<double>("lhe_M_qq", -999);
    arbol.newBranch<double>("lhe_M_lnu", -999);
    arbol.newBranch<double>("lhe_M_lnuqq", -999);
    arbol.newBranch<double>("lhe_pt_qq", -999);
    arbol.newBranch<double>("lhe_pt_lnu", -999);
    arbol.newBranch<double>("lhe_pt_lnuqq", -999);
    // Other branches
    arbol.newBranch<bool>("lhe_is_WpWm", false);
    arbol.newBranch<bool>("lhe_is_WmWp", false);
    arbol.newBranch<double>("gen_weight", -999);
    arbol.newBranch<int>("event", -999);

    // Initialize TLists for metadata TTrees
    TList* runs = new TList();
    TList* lumis = new TList();

    // Initialize Cutflow
    Histflow cutflow = Histflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVector>("lep_p4");
    cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
    cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("loose_lep_p4s", {});
    cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s", {});

    Cut* base = new LambdaCut(
        "Base", 
        [&]() 
        { 
            arbol.setLeaf<int>("event", nt.event());
            arbol.setLeaf<double>("gen_weight", nt.genWeight());
            return true;
        },
        [&]() 
        { 
            return nt.genWeight();
        }
    );
    cutflow.setRoot(base);

    Cut* search_lhe = new LambdaCut(
        "FindLHEQuarks",
        [&]()
        {
            // Set LHE quark leaves
            int q1_pdgID = nt.LHEPart_pdgId().at(4);
            int q2_pdgID = nt.LHEPart_pdgId().at(5);
            LorentzVector q1_p4 = nt.LHEPart_p4().at(4);
            LorentzVector q2_p4 = nt.LHEPart_p4().at(5);
            arbol.setLeaf<int>("lhe_charge_qq", getChargeQQ(q1_pdgID, q2_pdgID));
            arbol.setLeaf<double>("lhe_deta_qq", q1_p4.eta() - q2_p4.eta());
            if (q1_p4.pt() > q2_p4.pt())
            {
                arbol.setLeaf<int>("lhe_ld_q_pdgID", q1_pdgID);
                arbol.setLeaf<double>("lhe_ld_q_pt", q1_p4.pt());
                arbol.setLeaf<double>("lhe_ld_q_eta", q1_p4.eta());
                arbol.setLeaf<double>("lhe_ld_q_phi", q1_p4.phi());
                arbol.setLeaf<int>("lhe_tr_q_pdgID", q2_pdgID);
                arbol.setLeaf<double>("lhe_tr_q_pt", q2_p4.pt());
                arbol.setLeaf<double>("lhe_tr_q_eta", q2_p4.eta());
                arbol.setLeaf<double>("lhe_tr_q_phi", q2_p4.phi());
            }
            else
            {
                arbol.setLeaf<int>("lhe_ld_q_pdgID", q2_pdgID);
                arbol.setLeaf<double>("lhe_ld_q_pt", q2_p4.pt());
                arbol.setLeaf<double>("lhe_ld_q_eta", q2_p4.eta());
                arbol.setLeaf<double>("lhe_ld_q_phi", q2_p4.phi());
                arbol.setLeaf<int>("lhe_tr_q_pdgID", q1_pdgID);
                arbol.setLeaf<double>("lhe_tr_q_pt", q1_p4.pt());
                arbol.setLeaf<double>("lhe_tr_q_eta", q1_p4.eta());
                arbol.setLeaf<double>("lhe_tr_q_phi", q1_p4.phi());
            }
            // Set LHE lepton leaves
            LorentzVector lep_p4 = nt.LHEPart_p4().at(2);
            arbol.setLeaf<int>("lhe_lep_pdgID", nt.LHEPart_pdgId().at(2));
            arbol.setLeaf<double>("lhe_lep_pt", lep_p4.pt());
            arbol.setLeaf<double>("lhe_lep_eta", lep_p4.eta());
            arbol.setLeaf<double>("lhe_lep_phi", lep_p4.phi());
            // Set LHE neutrino leaves
            LorentzVector nu_p4 = nt.LHEPart_p4().at(3);
            arbol.setLeaf<int>("lhe_nu_pdgID", nt.LHEPart_pdgId().at(3));
            arbol.setLeaf<double>("lhe_nu_pt", nu_p4.pt());
            arbol.setLeaf<double>("lhe_nu_eta", nu_p4.eta());
            arbol.setLeaf<double>("lhe_nu_phi", nu_p4.phi());
            // Set other LHE kinematics leaves
            arbol.setLeaf<double>("lhe_M_qq", (q1_p4 + q2_p4).M());
            arbol.setLeaf<double>("lhe_M_lnu", (lep_p4 + nu_p4).M());
            arbol.setLeaf<double>("lhe_M_lnuqq", (lep_p4 + nu_p4 + q1_p4 + q2_p4).M());
            arbol.setLeaf<double>("lhe_pt_qq", (q1_p4 + q2_p4).pt());
            arbol.setLeaf<double>("lhe_pt_lnu", (lep_p4 + nu_p4).pt());
            arbol.setLeaf<double>("lhe_pt_lnuqq", (lep_p4 + nu_p4 + q1_p4 + q2_p4).pt());
            return true;
        }
    );
    cutflow.insert(base, search_lhe, Right);

    Cut* W_qq_charge = new LambdaCut(
        "ChargeQQEq1",
        [&]()
        {
            return (abs(arbol.getLeaf<int>("lhe_charge_qq")) == 1);
        }
    );
    cutflow.insert(search_lhe, W_qq_charge, Right);

    Cut* M_qq_window = new LambdaCut(
        "MqqInWMassWindow",
        [&]()
        {
            double M_qq = arbol.getLeaf<double>("lhe_M_qq");
            return (M_qq >= 70 && M_qq < 90);
        }
    );
    cutflow.insert(W_qq_charge, M_qq_window, Right);

    Cut* M_lnu_window = new LambdaCut(
        "MlnuInWMassWindow",
        [&]()
        {
            double M_lnu = arbol.getLeaf<double>("lhe_M_lnu");
            return (M_lnu >= 70 && M_lnu < 90);
        }
    );
    cutflow.insert(M_qq_window, M_lnu_window, Right);

    Cut* WpWm_vs_WmWp = new LambdaCut(
        "SeparateWpWmFromWmWp",
        [&]()
        {
            int charge_qq = arbol.getLeaf<int>("lhe_charge_qq");
            int lep_pdgID = arbol.getLeaf<int>("lhe_lep_pdgID");
            if (charge_qq > 0 && lep_pdgID > 0)
            {
                arbol.setLeaf<bool>("lhe_is_WpWm", true);
            }
            else if (charge_qq < 0 && lep_pdgID < 0)
            {
                arbol.setLeaf<bool>("lhe_is_WmWp", true);
            }
            else
            {
                return false;
            }
            return true;
        }
    );
    cutflow.insert(M_lnu_window, WpWm_vs_WmWp, Right);

    // Exactly 1 lepton
    Cut* findleps = new LambdaCut(
        "SKIM_FindLeptons", 
        [&]() 
        { 
            LorentzVectors veto_lep_p4s;
            LorentzVectors loose_lep_p4s;
            LorentzVectors tight_lep_p4s;
            for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
            {
                LorentzVector lep_p4 = nt.Electron_p4().at(elec_i);
                if (ttH::electronID(elec_i, ttH::IDveto, nt.year())) { veto_lep_p4s.push_back(lep_p4); }
                if (ttH::electronID(elec_i, ttH::IDfakable, nt.year())) { loose_lep_p4s.push_back(lep_p4); }
                if (ttH::electronID(elec_i, ttH::IDtight, nt.year())) { tight_lep_p4s.push_back(lep_p4); }
            }
            for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
            {
                LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
                if (ttH::muonID(muon_i, ttH::IDveto, nt.year())) { veto_lep_p4s.push_back(lep_p4); }
                if (ttH::muonID(muon_i, ttH::IDfakable, nt.year())) { loose_lep_p4s.push_back(lep_p4); }
                if (ttH::muonID(muon_i, ttH::IDtight, nt.year())) { tight_lep_p4s.push_back(lep_p4); }
            }
            cutflow.globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("loose_lep_p4s", loose_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("tight_lep_p4s", tight_lep_p4s);
            return true;
        }
    );
    cutflow.insert(WpWm_vs_WmWp, findleps, Right);

    // Geq1VetoLep
    Cut* geq1vetolep = new LambdaCut(
        "SKIM_Geq1VetoLep", [&]() { return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1; }
    );
    cutflow.insert(findleps, geq1vetolep, Right);

    // Geq2Jets
    Cut* geq2jets = new LambdaCut(
        "SKIM_Geq2Jets", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s");
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
    cutflow.insert(geq1vetolep, geq2jets, Right);

    // Geq1FatJet
    Cut* geq1fatjet = new LambdaCut(
        "SKIM_Geq1FatJetNoVetoLepOverlap", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s");
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
    cutflow.insert(geq2jets, geq1fatjet, Right);

    // Initialize hists
    std::vector<TH1D*> hists;
    TH1D* M_qq_hist = new TH1D("M_qq_hist", "lhe_M_qq", 52, 49, 101);
    hists.push_back(M_qq_hist);
    TH1D* M_lnu_hist = new TH1D("M_lnu_hist", "lhe_M_lnu", 52, 49, 101);
    hists.push_back(M_lnu_hist);
    TH1D* M_lnuqq_hist = new TH1D("M_lnuqq_hist", "lhe_M_lnuqq", 50, 0, 4000);
    hists.push_back(M_lnuqq_hist);
    TH1D* M_lnuqq_hist_course = new TH1D("M_lnuqq_hist_course", "lhe_M_lnuqq", 20, 0, 4000);
    hists.push_back(M_lnuqq_hist_course);
    TH1D* pt_ld_q_hist = new TH1D("pt_ld_q_hist", "lhe_ld_q_pt", 150, 0, 1500);
    hists.push_back(pt_ld_q_hist);
    TH1D* pt_tr_q_hist = new TH1D("pt_tr_q_hist", "lhe_tr_q_pt", 150, 0, 1500);
    hists.push_back(pt_tr_q_hist);
    TH1D* pt_lep_hist = new TH1D("pt_lep_hist", "lhe_lep_pt", 150, 0, 1500);
    hists.push_back(pt_lep_hist);
    TH1D* pt_nu_hist = new TH1D("pt_nu_hist", "lhe_nu_pt", 150, 0, 1500);
    hists.push_back(pt_nu_hist);
    TH1D* pt_qq_hist = new TH1D("pt_qq_hist", "lhe_pt_qq", 300, 0, 3000);
    hists.push_back(pt_qq_hist);
    TH1D* pt_lnu_hist = new TH1D("pt_lnu_hist", "lhe_pt_lnu", 300, 0, 3000);
    hists.push_back(pt_lnu_hist);
    TH1D* pt_lnuqq_hist = new TH1D("pt_lnuqq_hist", "lhe_pt_lnuqq", 300, 0, 3000);
    hists.push_back(pt_lnuqq_hist);
    // Book hists
    for (auto cut : {WpWm_vs_WmWp, geq1vetolep, geq2jets, geq1fatjet})
    {
        for (auto hist : hists)
        {
            TString orig_name = hist->GetName();
            hist->SetName("isWpWm__" + orig_name);
            cutflow.bookHist1D<TH1D>(
                cut, hist, 
                [&]() { return arbol.getLeaf<bool>("lhe_is_WpWm"); },
                [&, hist]() { return arbol.getLeaf<double>(hist->GetTitle()); }
            );
            hist->SetName("isWmWp__" + orig_name);
            cutflow.bookHist1D<TH1D>(
                cut, hist, 
                [&]() { return arbol.getLeaf<bool>("lhe_is_WmWp"); },
                [&, hist]() { return arbol.getLeaf<double>(hist->GetTitle()); }
            );
            hist->SetName(orig_name);
        }
    }
    TH1D* M_qq_hist_nosel = new TH1D("M_qq_hist_nosel", "lhe_M_qq", 4000, 0, 4000);
    cutflow.bookHist1D<TH1D>(
        base, M_qq_hist_nosel, 
        [&]() { return arbol.getLeaf<bool>("lhe_is_WpWm"); },
        [&]() { return arbol.getLeaf<double>("lhe_M_qq"); }
    );

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
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
                // Reset branches and globals
                arbol.resetBranches();
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.run(WpWm_vs_WmWp);
                if (passed) { arbol.fill(); }
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.writeCSV(cli.output_dir);
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
