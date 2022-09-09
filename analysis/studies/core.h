#ifndef CORE_H
#define CORE_H

#include "scalefactors.h"
// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "ElectronSelections.h"
#include "MuonSelections.h"
#include "Tools/goodrun.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<double> Doubles;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

namespace Core
{

struct Analysis
{
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;
    NanoScaleFactorsUL sfs;
    bool init_branches;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbol(arbol_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;
        if (cli.variation == "jec_up") { sfs = NanoScaleFactorsUL(2, 1); }
        else if (cli.variation == "jec_dn") { sfs = NanoScaleFactorsUL(-2, 1); }
        else if (cli.variation == "jer_up") { sfs = NanoScaleFactorsUL(1, 2); }
        else if (cli.variation == "jer_dn") { sfs = NanoScaleFactorsUL(1, -2); }
        else { sfs = NanoScaleFactorsUL(1, 1); }
        init_branches = false;
    };

    virtual void initBranches() { /* Initialize Arbol branches */ };

    virtual void init()
    {
        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        gconf.GetConfigs(nt.year());
        gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("16APV"));

        sfs.init(file_name);

        if (cli.is_data)
        {
            if (nt.year() == 2016)
            {
                if (gconf.isAPV)
                {
                    set_goodrun_file("data/golden_jsons/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON_formatted.txt");
                }
                else
                {
                    set_goodrun_file("data/golden_jsons/Cert_271036-325175_13TeV_Combined161718_JSON_snt.txt");
                }
            }
            else if (nt.year() == 2017)
            {
                set_goodrun_file("data/golden_jsons/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON_snt.txt");
            }
            else if (nt.year() == 2018)
            {
                set_goodrun_file("data/golden_jsons/Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON_snt.txt");
            }
        }
    };
};

struct VBSAnalysis : Analysis
{
    VBSAnalysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Lepton globals
        cutflow.globals.newVar<LorentzVectors>("good_lep_p4s", {});
        cutflow.globals.newVar<Integers>("good_lep_pdgIDs", {});
        cutflow.globals.newVar<Integers>("good_lep_idxs", {});
        cutflow.globals.newVar<Integers>("good_lep_jet_idxs", {});
        // Jet globals
        cutflow.globals.newVar<LorentzVectors>("good_jet_p4s", {});
        cutflow.globals.newVar<Integers>("good_jet_idxs", {});
        cutflow.globals.newVar<Doubles>("good_jet_btags", {});
        // Fat jet (AK8) globals
        cutflow.globals.newVar<LorentzVectors>("good_fatjet_p4s", {});
        cutflow.globals.newVar<Integers>("good_fatjet_idxs", {});
        cutflow.globals.newVar<Doubles>("good_fatjet_hbbtags", {}); // ParticleNet
        cutflow.globals.newVar<Doubles>("good_fatjet_xbbtags", {}); // ParticleNet
        cutflow.globals.newVar<Doubles>("good_fatjet_masses", {});  // ParticleNet
        cutflow.globals.newVar<Doubles>("good_fatjet_msoftdrops", {});
        // VBS jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vbs_jet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vbs_jet_p4");
        cutflow.globals.newVar<int>("ld_vbs_jet_idx");
        cutflow.globals.newVar<int>("tr_vbs_jet_idx");
    };

    virtual void initBranches()
    {
        // Jet (AK4) branches
        arbol.newBranch<double>("HT", -999);
        arbol.newBranch<int>("n_jets_pt30", -999);
        arbol.newBranch<int>("n_loose_b_jets", -999);
        arbol.newBranch<int>("n_medium_b_jets", -999);
        arbol.newBranch<int>("n_tight_b_jets", -999);
        // Jet (AK8) branches
        arbol.newBranch<int>("n_fatjets", -999);
        // VBS jet branches
        arbol.newBranch<double>("ld_vbs_jet_pt", -999);
        arbol.newBranch<double>("tr_vbs_jet_pt", -999);
        arbol.newBranch<double>("M_jj", -999);
        arbol.newBranch<double>("deta_jj", -999);
        arbol.newBranch<double>("dR_jj", -999);
        // Other branches
        arbol.newBranch<double>("xsec_sf", -999);
        arbol.newBranch<int>("event", -999);
        arbol.newBranch<double>("MET", -999);

        init_branches = true;
    };
};

class DressedCut : public Cut
{
public:
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;
    NanoScaleFactorsUL& sfs;
    bool& init_branches;

    DressedCut(std::string new_name, Analysis& a) 
    : Cut(new_name), arbol(a.arbol), nt(a.nt), cli(a.cli), globals(a.cutflow.globals), sfs(a.sfs), 
      init_branches(a.init_branches)
    {
        // Do nothing
    };
};

class Bookkeeping : public DressedCut
{
public:
    Bookkeeping(std::string name, Analysis& analysis) : DressedCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        if (init_branches)
        {
            arbol.setLeaf<double>("xsec_sf", cli.is_data ? 1. : cli.scale_factor*nt.genWeight());
            arbol.setLeaf<int>("event", nt.event());
            arbol.setLeaf<double>("MET", nt.MET_pt());
        }
        return (cli.is_data) ? goodrun(nt.run(), nt.luminosityBlock()) : true;
    };

    double weight()
    {
        return cli.scale_factor*nt.genWeight();
    };
};

class SelectLeptons : public DressedCut
{
public:
    SelectLeptons(std::string name, Analysis& analysis) : DressedCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        // Lepton ID sf
        double lep_sf = 1.;
        // Percent errors (up/down) on sf
        double err_up = 0.;
        double err_dn = 0.;

        LorentzVectors good_lep_p4s;
        Integers good_lep_pdgIDs;
        Integers good_lep_idxs;
        Integers good_lep_jet_idxs;
        // Loop over electrons
        for (unsigned int i = 0; i < nt.nElectron(); ++i)
        {
            if (!ttH_UL::electronID(i, ttH::IDveto, nt.year())) { continue; }
            LorentzVector el_p4 = nt.Electron_p4().at(i);
            // Store basic info
            good_lep_p4s.push_back(el_p4);
            good_lep_pdgIDs.push_back(-nt.Electron_charge().at(i)*11);
            good_lep_idxs.push_back(i);
            good_lep_jet_idxs.push_back(nt.Electron_jetIdx().at(i));
            if (cli.is_data) { continue; }
            // Get scale factor (and up/down variations)
            double el_eta = fabs(std::max(std::min(el_p4.eta(), 2.4999f), -2.4999f));
            double el_pt = el_p4.pt();
            // event --> reco
            lep_sf *= sfs.el_reco->getSF(el_eta, el_pt);
            err_up += std::pow(sfs.el_reco->getErr(el_eta, el_pt), 2);
            err_dn += std::pow(sfs.el_reco->getErr(el_eta, el_pt), 2);
            // reco --> loose ttH ID
            lep_sf *= sfs.el_iso_loose->getSF(el_eta, el_pt);
            err_up += std::pow(sfs.el_iso_loose->getErr(el_eta, el_pt), 2);
            err_dn += std::pow(sfs.el_iso_loose->getErr(el_eta, el_pt), 2);
            // loose ttH ID --> tight ttH ID
            lep_sf *= sfs.el_tth_tight->getSF(el_eta, el_pt);
            err_up += std::pow(sfs.el_tth_tight->getErr(el_eta, el_pt), 2);
            err_dn += std::pow(sfs.el_tth_tight->getErr(el_eta, el_pt), 2);
        }
        // Loop over muons
        for (unsigned int i = 0; i < nt.nMuon(); ++i)
        {
            if (!ttH_UL::muonID(i, ttH::IDveto, nt.year())) { continue; }
            LorentzVector mu_p4 = nt.Muon_p4().at(i);
            // Store basic info
            good_lep_p4s.push_back(mu_p4);
            good_lep_pdgIDs.push_back(-nt.Muon_charge().at(i)*13);
            good_lep_idxs.push_back(i);
            good_lep_jet_idxs.push_back(nt.Muon_jetIdx().at(i));
            if (cli.is_data) { continue; }
            // Get scale factor (and up/down variations)
            double mu_eta = fabs(mu_p4.eta());
            double mu_pt = mu_p4.pt();
            // event --> loose POG ID
            lep_sf *= sfs.mu_pog_loose->getSF(mu_eta, mu_pt);
            err_up += std::pow(sfs.mu_pog_loose->getErr(mu_eta, mu_pt), 2);
            err_dn += std::pow(sfs.mu_pog_loose->getErr(mu_eta, mu_pt), 2);
            // loose POG ID --> loose ttH ID
            lep_sf *= sfs.mu_iso_loose->getSF(mu_eta, mu_pt);
            err_up += std::pow(sfs.mu_iso_loose->getErr(mu_eta, mu_pt), 2);
            err_dn += std::pow(sfs.mu_iso_loose->getErr(mu_eta, mu_pt), 2);
            // loose ttH ID --> tight ttH ID
            lep_sf *= sfs.mu_tth_tight->getSF(mu_eta, mu_pt);
            err_up += std::pow(sfs.mu_tth_tight->getErr(mu_eta, mu_pt), 2);
            err_dn += std::pow(sfs.mu_tth_tight->getErr(mu_eta, mu_pt), 2);
        }

        globals.setVal<LorentzVectors>("good_lep_p4s", good_lep_p4s);
        globals.setVal<Integers>("good_lep_pdgIDs", good_lep_pdgIDs);
        globals.setVal<Integers>("good_lep_idxs", good_lep_idxs);
        globals.setVal<Integers>("good_lep_jet_idxs", good_lep_jet_idxs);

        if (!init_branches) { return true; }

        // Store lepton sf and its up/down variations
        if (!cli.is_data)
        {
            // Finish error computation
            err_up = std::sqrt(err_up);
            err_dn = std::sqrt(err_dn);
            arbol.setLeaf<double>("lep_sf", lep_sf);
            arbol.setLeaf<double>("lep_sf_up", lep_sf + err_up*lep_sf);
            arbol.setLeaf<double>("lep_sf_dn", lep_sf - err_dn*lep_sf);
        }
        else
        {
            arbol.setLeaf<double>("lep_sf", 1.);
            arbol.setLeaf<double>("lep_sf_up", 1.);
            arbol.setLeaf<double>("lep_sf_dn", 1.);
        }
        return true;
    };
};

class SelectJets : public DressedCut
{
public:
    LorentzVectors good_lep_p4s;
    Integers good_lep_jet_idxs;

    SelectJets(std::string name, Analysis& analysis) : DressedCut(name, analysis) 
    {
        // Do nothing
    };

    virtual void loadOverlapVars()
    {
        good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        good_lep_jet_idxs = globals.getVal<Integers>("good_lep_jet_idxs");
    };

    bool overlapsLepton(int jet_i, LorentzVector jet_p4)
    {
        for (unsigned int lep_i = 0; lep_i < good_lep_p4s.size(); ++lep_i)
        {
            int lep_jet_idx = good_lep_jet_idxs.at(lep_i);
            if (lep_jet_idx == -999)
            {
                LorentzVector lep_p4 = good_lep_p4s.at(lep_i);
                if (ROOT::Math::VectorUtil::DeltaR(lep_p4, jet_p4) < 0.4) 
                {
                    return true;
                }
            }
            else if (lep_jet_idx == int(jet_i))
            {
                return true;
            }
        }
        return false;
    };

    virtual bool isOverlap(int jet_i, LorentzVector jet_p4)
    {
        return overlapsLepton(jet_i, jet_p4);
    };

    bool evaluate()
    {
        loadOverlapVars();
        int n_loose_b_jets = 0;
        int n_medium_b_jets = 0;
        int n_tight_b_jets = 0;
        int n_jets_pt30 = 0;
        double ht = 0.;
        Doubles good_jet_btags;
        LorentzVectors good_jet_p4s;
        Integers good_jet_idxs;
        /* FIXME: not used, see FIXME below
        int jer_seed = (
            1 + (nt.run() << 20) 
            + (nt.luminosityBlock() << 10) 
            + nt.event() 
            + (nt.nJet() > 0 ? nt.Jet_eta().at(0)/0.01 : 0)
        );
        */
        for (unsigned int jet_i = 0; jet_i < nt.nJet(); ++jet_i)
        {
            // Read jet p4
            LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
            // Apply JECs/JERs
            jet_p4 = sfs.applyJEC(jet_p4);
            /* FIXME: GenJet_* branches missing from current skim
            jet_p4 = sfs.applyJER(
                jer_seed, 
                jet_p4, 
                nt.fixedGridRhoFastjetAll(), 
                nt.GenJet_p4()
            );
            */
            if (jet_p4.pt() < 20) { continue; }
            // Apply jet ID
            int jet_id = nt.Jet_jetId().at(jet_i);
            if (nt.year() == 2016 && jet_id < 1) { continue; }
            if (nt.year() > 2016 && jet_id < 2) { continue; }
            // Perform overlap check against leptons
            if (isOverlap(jet_i, jet_p4)) { continue; }
            // Perform b-tagging (for b-veto); only possible for pt > 20 GeV and |eta| < 2.4
            bool is_btagged_loose = false;
            bool is_btagged_medium = false;
            bool is_btagged_tight = false;
            double deepflav_btag = nt.Jet_btagDeepFlavB().at(jet_i);
            if (fabs(jet_p4.eta()) < 2.4) 
            {
                // Check DeepJet vs. working points in NanoCORE global config (gconf)
                is_btagged_loose = deepflav_btag > gconf.WP_DeepFlav_loose;
                is_btagged_medium = deepflav_btag > gconf.WP_DeepFlav_medium;
                is_btagged_tight = deepflav_btag > gconf.WP_DeepFlav_tight;
            }
            // Count b-tags and save jet info
            if (is_btagged_tight) 
            {
                good_jet_btags.push_back(deepflav_btag);
                n_tight_b_jets++;
            }
            else if (is_btagged_medium)
            {
                good_jet_btags.push_back(deepflav_btag);
                n_medium_b_jets++;
            }
            else if (is_btagged_loose)
            {
                good_jet_btags.push_back(deepflav_btag);
                n_loose_b_jets++;
            }
            else
            {
                good_jet_btags.push_back(-999.);
            }
            // Save additional jet info
            n_jets_pt30++;
            ht += jet_p4.pt();
            good_jet_p4s.push_back(jet_p4);
            good_jet_idxs.push_back(jet_i);
        }

        globals.setVal<Doubles>("good_jet_btags", good_jet_btags);
        globals.setVal<LorentzVectors>("good_jet_p4s", good_jet_p4s);
        globals.setVal<Integers>("good_jet_idxs", good_jet_idxs);

        if (!init_branches) { return true; }

        arbol.setLeaf<int>("n_loose_b_jets", n_loose_b_jets);
        arbol.setLeaf<int>("n_medium_b_jets", n_medium_b_jets);
        arbol.setLeaf<int>("n_tight_b_jets", n_tight_b_jets);
        arbol.setLeaf<int>("n_jets_pt30", n_jets_pt30);
        arbol.setLeaf<double>("HT", ht);

        return true;
    };
};

class SelectFatJets : public DressedCut
{
public:
    SelectFatJets(std::string name, Analysis& analysis) : DressedCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_fatjet_p4s;
        Integers good_fatjet_idxs;
        Doubles good_fatjet_hbbtags;
        Doubles good_fatjet_xbbtags;
        Doubles good_fatjet_masses;
        Doubles good_fatjet_msoftdrops;
        LorentzVectors good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); ++fatjet_i)
        {
            // Basic requirements
            if (nt.FatJet_pt().at(fatjet_i) <= 250) { continue; }
            if (nt.FatJet_mass().at(fatjet_i) <= 50) { continue; }
            if (nt.FatJet_msoftdrop().at(fatjet_i) <= 40) { continue; }
            if (fabs(nt.FatJet_eta().at(fatjet_i)) >= 2.5) { continue; }
            // Remove lepton overlap
            LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
            bool is_overlap = false;
            for (auto& lep_p4 : good_lep_p4s)
            {
                if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8) 
                {
                    is_overlap = true;
                    break;
                }
            }
            if (is_overlap) { continue; }

            double pnet_hbb = nt.FatJet_particleNet_HbbvsQCD().at(fatjet_i);
            double pnet_xbb = nt.FatJet_particleNetMD_Xbb().at(fatjet_i);
            double pnet_qcd = nt.FatJet_particleNetMD_QCD().at(fatjet_i);

            // Store good fat jets
            good_fatjet_p4s.push_back(fatjet_p4);
            good_fatjet_idxs.push_back(fatjet_i);
            good_fatjet_hbbtags.push_back(pnet_hbb);
            good_fatjet_xbbtags.push_back(pnet_xbb/(pnet_xbb + pnet_qcd));
            good_fatjet_masses.push_back(nt.FatJet_particleNet_mass().at(fatjet_i));
            good_fatjet_msoftdrops.push_back(nt.FatJet_msoftdrop().at(fatjet_i));
        }
        globals.setVal<LorentzVectors>("good_fatjet_p4s", good_fatjet_p4s);
        globals.setVal<Integers>("good_fatjet_idxs", good_fatjet_idxs);
        globals.setVal<Doubles>("good_fatjet_hbbtags", good_fatjet_hbbtags);
        globals.setVal<Doubles>("good_fatjet_xbbtags", good_fatjet_xbbtags);
        globals.setVal<Doubles>("good_fatjet_masses", good_fatjet_masses);
        globals.setVal<Doubles>("good_fatjet_msoftdrops", good_fatjet_msoftdrops);

        if (!init_branches) { return true; }

        arbol.setLeaf<int>("n_fatjets", good_fatjet_p4s.size());

        return true;
    };
};

class SelectVBSJetsMaxE : public DressedCut
{
public:
    SelectVBSJetsMaxE(std::string name, Analysis& analysis) : DressedCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        // Select VBS jet candidates
        Integers vbs_jet_cand_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            if (good_jet_p4s.at(jet_i).pt() >= 30.) { vbs_jet_cand_idxs.push_back(jet_i); }
        }
        if (vbs_jet_cand_idxs.size() < 2) { return false; }
        // Sort candidates by pt
        std::sort(
            vbs_jet_cand_idxs.begin(), vbs_jet_cand_idxs.end(),
            [&](int i, int j) -> bool { return good_jet_p4s.at(i).pt() > good_jet_p4s.at(j).pt(); }
        );
        // Process candidates
        std::pair<int, int> vbs_jet_idxs;
        if (vbs_jet_cand_idxs.size() == 2)
        {
            vbs_jet_idxs = std::make_pair(vbs_jet_cand_idxs.at(0), vbs_jet_cand_idxs.at(1));
        }
        else
        {
            // Collect jets in pos/neg eta hemispheres
            Integers vbs_pos_eta_jet_idxs;
            Integers vbs_neg_eta_jet_idxs;
            for (auto& jet_i : vbs_jet_cand_idxs)
            {
                const LorentzVector& jet_p4 = good_jet_p4s.at(jet_i);
                if (jet_p4.eta() >= 0)
                {
                    vbs_pos_eta_jet_idxs.push_back(jet_i);
                }
                else
                {
                    vbs_neg_eta_jet_idxs.push_back(jet_i);
                }
            }
            // Sort the jets in each hemisphere by P
            std::sort(
                vbs_pos_eta_jet_idxs.begin(), vbs_pos_eta_jet_idxs.end(),
                [&](int i, int j) -> bool { return good_jet_p4s.at(i).P() > good_jet_p4s.at(j).P(); }
            );
            std::sort(
                vbs_neg_eta_jet_idxs.begin(), vbs_neg_eta_jet_idxs.end(),
                [&](int i, int j) -> bool { return good_jet_p4s.at(i).P() > good_jet_p4s.at(j).P(); }
            );
            // Select VBS jets
            if (vbs_pos_eta_jet_idxs.size() == 0)
            {
                // All candidates are in the -eta hemisphere
                vbs_jet_idxs = std::make_pair(vbs_neg_eta_jet_idxs.at(0), vbs_neg_eta_jet_idxs.at(1));
            }
            else if (vbs_neg_eta_jet_idxs.size() == 0)
            {
                // All candidates are in the +eta hemisphere
                vbs_jet_idxs = std::make_pair(vbs_pos_eta_jet_idxs.at(0), vbs_pos_eta_jet_idxs.at(1));
            }
            else
            {
                // Take the leading candidate (in P) from each hemisphere
                vbs_jet_idxs = std::make_pair(vbs_pos_eta_jet_idxs.at(0), vbs_neg_eta_jet_idxs.at(0));
            }
        }
        // Separate the two VBS jets into leading/trailing
        int ld_vbs_jet_idx;
        int tr_vbs_jet_idx;
        if (good_jet_p4s.at(vbs_jet_idxs.first).pt() > good_jet_p4s.at(vbs_jet_idxs.first).pt())
        {
            ld_vbs_jet_idx = vbs_jet_idxs.first;
            tr_vbs_jet_idx = vbs_jet_idxs.second;
        }
        else
        {
            ld_vbs_jet_idx = vbs_jet_idxs.second;
            tr_vbs_jet_idx = vbs_jet_idxs.first;
        }
        LorentzVector ld_vbs_jet_p4 = good_jet_p4s.at(ld_vbs_jet_idx);
        LorentzVector tr_vbs_jet_p4 = good_jet_p4s.at(tr_vbs_jet_idx);
        // Save VBS jet info
        globals.setVal<LorentzVector>("ld_vbs_jet_p4", ld_vbs_jet_p4);
        globals.setVal<int>("ld_vbs_jet_idx", ld_vbs_jet_idx);
        globals.setVal<LorentzVector>("tr_vbs_jet_p4", tr_vbs_jet_p4);
        globals.setVal<int>("tr_vbs_jet_idx", tr_vbs_jet_idx);

        if (!init_branches) { return true; }

        arbol.setLeaf<double>("ld_vbs_jet_pt", ld_vbs_jet_p4.pt());
        arbol.setLeaf<double>("tr_vbs_jet_pt", tr_vbs_jet_p4.pt());
        arbol.setLeaf<double>("M_jj", (ld_vbs_jet_p4 + tr_vbs_jet_p4).M());
        arbol.setLeaf<double>("deta_jj", ld_vbs_jet_p4.eta() - tr_vbs_jet_p4.eta());
        arbol.setLeaf<double>("dR_jj", ROOT::Math::VectorUtil::DeltaR(ld_vbs_jet_p4, tr_vbs_jet_p4));

        return true;
    };
};

}; // End namespace Core

#endif
