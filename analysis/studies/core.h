#ifndef CORE_H
#define CORE_H

// RAPIDO
#include "arbol.h"
#include "arbusto.h"
#include "cutflow.h"
#include "hepcli.h"
#include "utilities.h"          // Utilities::Variables
// VBS
#include "pku.h"                // PKU::IDLevel, PKU::passesElecID, PKU::passesMuonID
#include "sfs.h"                // LeptonSFs, BTagSFs
#include "jes.h"                // JetEnergyScales
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"    // DeltaR
// NanoCORE
#include "Nano.h"
#include "Config.h"             // gconf
#include "ElectronSelections.h" // ttH_UL::electronID
#include "MuonSelections.h"     // ttH_UL::muonID
#include "Tools/goodrun.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<double> Doubles;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

namespace Core
{

struct Skimmer
{
    Arbusto& arbusto;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;

    Skimmer(Arbusto& arbusto_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbusto(arbusto_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;
    };

    virtual void init(TTree* ttree)
    {
        arbusto.init(ttree);
        gconf.GetConfigs(nt.year());

        TString file_name = ttree->GetCurrentFile()->GetName();
        gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("NanoAODAPV") || file_name.Contains("UL16APV"));
    };
};

class SkimmerCut : public Cut
{
public:
    Arbusto& arbusto;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;

    SkimmerCut(std::string new_name, Skimmer& s) 
    : Cut(new_name), arbusto(s.arbusto), nt(s.nt), cli(s.cli), globals(s.cutflow.globals)
    {
        // Do nothing
    };
};

struct Analysis
{
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbol(arbol_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;

        // Lepton globals
        cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
        cutflow.globals.newVar<Integers>("veto_lep_pdgIDs", {});
        cutflow.globals.newVar<Integers>("veto_lep_idxs", {});
        cutflow.globals.newVar<Integers>("veto_lep_jet_idxs", {});
        // Jet globals
        cutflow.globals.newVar<LorentzVectors>("good_jet_p4s", {});
        cutflow.globals.newVar<Integers>("good_jet_idxs", {});
        // Fat jet (AK8) globals
        cutflow.globals.newVar<LorentzVectors>("good_fatjet_p4s", {});
        cutflow.globals.newVar<Integers>("good_fatjet_idxs", {});
        cutflow.globals.newVar<Doubles>("good_fatjet_hbbtags", {}); // ParticleNet
        cutflow.globals.newVar<Doubles>("good_fatjet_xbbtags", {}); // ParticleNet
        cutflow.globals.newVar<Doubles>("good_fatjet_masses", {});  // ParticleNet
        cutflow.globals.newVar<Doubles>("good_fatjet_msoftdrops", {});
        // VBS jet globals
        cutflow.globals.newVar<LorentzVector>("ld_vbsjet_p4");
        cutflow.globals.newVar<LorentzVector>("tr_vbsjet_p4");
        cutflow.globals.newVar<int>("ld_vbsjet_idx");
        cutflow.globals.newVar<int>("tr_vbsjet_idx");
    };

    virtual void initBranches()
    {
        // Jet (AK4) branches
        arbol.newBranch<double>("HT", -999);
        arbol.newBranch<int>("n_jets", -999);
        arbol.newBranch<int>("n_loose_b_jets", -999);
        arbol.newBranch<int>("n_medium_b_jets", -999);
        arbol.newBranch<int>("n_tight_b_jets", -999);
        arbol.newBranch<double>("btag_sf", -999);
        arbol.newBranch<double>("btag_sf_up", -999);
        arbol.newBranch<double>("btag_sf_dn", -999);
        // Jet (AK8) branches
        arbol.newBranch<int>("n_fatjets", -999);
        // VBS jet branches
        arbol.newBranch<double>("ld_vbsjet_pt", -999);
        arbol.newBranch<double>("ld_vbsjet_eta", -999);
        arbol.newBranch<double>("ld_vbsjet_phi", -999);
        arbol.newBranch<double>("tr_vbsjet_pt", -999);
        arbol.newBranch<double>("tr_vbsjet_eta", -999);
        arbol.newBranch<double>("tr_vbsjet_phi", -999);
        arbol.newBranch<double>("M_jj", -999);
        arbol.newBranch<double>("deta_jj", -999);
        arbol.newBranch<double>("abs_deta_jj", 999);
        arbol.newBranch<double>("dR_jj", -999);
        // Other branches
        arbol.newBranch<double>("xsec_sf", -999);
        arbol.newBranch<int>("event", -999);
        arbol.newBranch<double>("MET", -999);
        arbol.newBranch<double>("MET_up", -999);
        arbol.newBranch<double>("MET_dn", -999);
        arbol.newBranch<float>("lhe_muF0p5_muR0p5", -999);
        arbol.newBranch<float>("lhe_muF1p0_muR0p5", -999);
        arbol.newBranch<float>("lhe_muF2p0_muR0p5", -999);
        arbol.newBranch<float>("lhe_muF0p5_muR1p0", -999);
        arbol.newBranch<float>("lhe_muF1p0_muR1p0", -999);
        arbol.newBranch<float>("lhe_muF2p0_muR1p0", -999);
        arbol.newBranch<float>("lhe_muF0p5_muR2p0", -999);
        arbol.newBranch<float>("lhe_muF1p0_muR2p0", -999);
        arbol.newBranch<float>("lhe_muF2p0_muR2p0", -999);
    };

    virtual void init()
    {
        gconf.GetConfigs(nt.year());

        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        gconf.isAPV = (file_name.Contains("HIPM_UL2016") || file_name.Contains("NanoAODAPV") || file_name.Contains("UL16APV"));

        if (nt.isData())
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

class AnalysisCut : public Cut
{
public:
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;

    AnalysisCut(std::string new_name, Analysis& a) 
    : Cut(new_name), arbol(a.arbol), nt(a.nt), cli(a.cli), globals(a.cutflow.globals)
    {
        // Do nothing
    };
};

class Bookkeeping : public AnalysisCut
{
public:
    Bookkeeping(std::string name, Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        arbol.setLeaf<double>("xsec_sf", (nt.isData()) ? 1. : cli.scale_factor*nt.genWeight());
        arbol.setLeaf<int>("event", nt.event());
        return (nt.isData()) ? goodrun(nt.run(), nt.luminosityBlock()) : true;
    };

    double weight()
    {
        return (nt.isData()) ? 1. : cli.scale_factor*nt.genWeight();
    };
};

class SelectLeptons : public AnalysisCut
{
public:
    LeptonSFs* lep_sfs;

    SelectLeptons(std::string name, Analysis& analysis, LeptonSFs* lep_sfs) 
    : AnalysisCut(name, analysis)
    {
        this->lep_sfs = lep_sfs;
    };

    virtual bool passesVetoElecID(int elec_i)
    {
        return ttH_UL::electronID(elec_i, ttH::IDveto, nt.year());
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return ttH_UL::muonID(muon_i, ttH::IDveto, nt.year());
    };

    bool evaluate()
    {
        LorentzVectors veto_lep_p4s;
        Integers veto_lep_pdgIDs;
        Integers veto_lep_idxs;
        Integers veto_lep_jet_idxs;
        // Lepton ID sf
        double lep_sf = arbol.getLeaf<double>("lep_sf");
        // Percent errors (up/down) on sf
        double err_up = arbol.getLeaf<double>("lep_sf_up")/lep_sf - 1;
        double err_dn = 1 - arbol.getLeaf<double>("lep_sf_dn")/lep_sf;
        // Loop over electrons
        for (unsigned int i = 0; i < nt.nElectron(); ++i)
        {
            if (!passesVetoElecID(i)) { continue; }
            LorentzVector el_p4 = nt.Electron_p4().at(i);
            // Store basic info
            veto_lep_p4s.push_back(el_p4);
            veto_lep_pdgIDs.push_back(-nt.Electron_charge().at(i)*11);
            veto_lep_idxs.push_back(i);
            veto_lep_jet_idxs.push_back(nt.Electron_jetIdx().at(i));
            if (nt.isData() || lep_sfs == nullptr) { continue; }
            lep_sf *= lep_sfs->getElecSF(el_p4.pt(), el_p4.eta());
            err_up += std::pow(lep_sfs->getElecErrUp(el_p4.pt(), el_p4.eta()), 2);
            err_dn += std::pow(lep_sfs->getElecErrDn(el_p4.pt(), el_p4.eta()), 2);
        }
        // Loop over muons
        for (unsigned int i = 0; i < nt.nMuon(); ++i)
        {
            if (!passesVetoMuonID(i)) { continue; }
            LorentzVector mu_p4 = nt.Muon_p4().at(i);
            // Store basic info
            veto_lep_p4s.push_back(mu_p4);
            veto_lep_pdgIDs.push_back(-nt.Muon_charge().at(i)*13);
            veto_lep_idxs.push_back(i);
            veto_lep_jet_idxs.push_back(nt.Muon_jetIdx().at(i));
            if (nt.isData() || lep_sfs == nullptr) { continue; }
            lep_sf *= lep_sfs->getMuonSF(mu_p4.pt(), mu_p4.eta());
            err_up += std::pow(lep_sfs->getMuonErrUp(mu_p4.pt(), mu_p4.eta()), 2);
            err_dn += std::pow(lep_sfs->getMuonErrDn(mu_p4.pt(), mu_p4.eta()), 2);
        }

        globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
        globals.setVal<Integers>("veto_lep_pdgIDs", veto_lep_pdgIDs);
        globals.setVal<Integers>("veto_lep_idxs", veto_lep_idxs);
        globals.setVal<Integers>("veto_lep_jet_idxs", veto_lep_jet_idxs);

        // Store lepton sf and its up/down variations
        if (!nt.isData() && lep_sfs != nullptr)
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

class SelectLeptonsPKU : public SelectLeptons
{
public:
    SelectLeptonsPKU(std::string name, Analysis& analysis, LeptonSFsPKU* lep_sfs = nullptr) 
    : Core::SelectLeptons(name, analysis, lep_sfs) 
    {
        // Do nothing
    };

    bool passesVetoElecID(int elec_i)
    {
        return PKU::passesElecID(elec_i, PKU::IDveto);
    };

    bool passesVetoMuonID(int muon_i)
    {
        return PKU::passesMuonID(muon_i, PKU::IDveto);
    };
};

class SelectJets : public AnalysisCut
{
public:
    JetEnergyScales* jes;
    BTagSFs* btag_sfs;
    LorentzVectors veto_lep_p4s;
    Integers veto_lep_jet_idxs;

    SelectJets(std::string name, Analysis& analysis, JetEnergyScales* jes = nullptr, BTagSFs* btag_sfs = nullptr) 
    : AnalysisCut(name, analysis)
    {
        this->jes = jes;
        this->btag_sfs = btag_sfs;
    };

    virtual void loadOverlapVars()
    {
        veto_lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
        veto_lep_jet_idxs = globals.getVal<Integers>("veto_lep_jet_idxs");
    };

    bool overlapsLepton(int jet_i, LorentzVector jet_p4)
    {
        for (unsigned int lep_i = 0; lep_i < veto_lep_p4s.size(); ++lep_i)
        {
            int lep_jet_idx = veto_lep_jet_idxs.at(lep_i);
            if (lep_jet_idx == -999)
            {
                LorentzVector lep_p4 = veto_lep_p4s.at(lep_i);
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

    virtual bool isGoodJet(int jet_i, LorentzVector jet_p4)
    {
        if (jet_p4.pt() < 20) { return false; }
        int jet_id = nt.Jet_jetId().at(jet_i);
        if (nt.year() == 2016 && jet_id < 1) { return false; }
        if (nt.year() > 2016 && jet_id < 2) { return false; }
        return true;
    };

    bool evaluate()
    {
        loadOverlapVars();
        int n_loose_b_jets = 0;
        int n_medium_b_jets = 0;
        int n_tight_b_jets = 0;
        int n_jets = 0;
        double btag_sf = 1.;
        double btag_sf_up = 1.;
        double btag_sf_dn = 1.;
        double ht = 0.;
        LorentzVectors good_jet_p4s;
        Integers good_jet_idxs;
        int jer_seed = (
            1 + (nt.run() << 20) 
            + (nt.luminosityBlock() << 10) 
            + nt.event() 
            + (nt.nJet() > 0 ? nt.Jet_eta().at(0)/0.01 : 0)
        );
        double met_x = nt.MET_pt()*std::cos(nt.MET_phi());
        double met_y = nt.MET_pt()*std::sin(nt.MET_phi());
        for (unsigned int jet_i = 0; jet_i < nt.nJet(); ++jet_i)
        {
            LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
            // Subtract uncorrected jet pt
            met_x -= jet_p4.px();
            met_y -= jet_p4.py();
            // Apply JECs/JERs
            if (jes != nullptr)
            {
                if (!nt.isData())
                {
                    jet_p4 = jes->applyJER(
                        jer_seed, 
                        jet_p4, 
                        nt.fixedGridRhoFastjetAll(), 
                        nt.GenJet_p4()
                    );
                }
                jet_p4 = jes->applyJEC(jet_p4);
            }
            // Add corrected jet pt
            met_x += jet_p4.px();
            met_y += jet_p4.py();
            // Select good jets
            if (!isGoodJet(jet_i, jet_p4)) { continue; }
            if (isOverlap(jet_i, jet_p4)) { continue; }
            // Perform b-tagging (for b-veto); only possible for pt > 20 GeV and |eta| < 2.4
            if (fabs(jet_p4.eta()) < 2.4) 
            {
                // Check DeepJet vs. working points in NanoCORE global config (gconf)
                double deepflav_btag = nt.Jet_btagDeepFlavB().at(jet_i);
                if (deepflav_btag > gconf.WP_DeepFlav_tight) 
                {
                    n_tight_b_jets++;
                }
                if (deepflav_btag > gconf.WP_DeepFlav_medium)
                {
                    n_medium_b_jets++;
                }
                else
                {
                    if (!nt.isData() && btag_sfs != nullptr)
                    {
                        int flavor = nt.Jet_hadronFlavour().at(jet_i);
                        double jet_pt = jet_p4.pt();
                        double jet_abseta = fabs(jet_p4.eta());
                        double sf = btag_sfs->getSF(flavor, jet_pt, jet_abseta);
                        double sf_up = btag_sfs->getSFUp(flavor, jet_pt, jet_abseta);
                        double sf_dn = btag_sfs->getSFDn(flavor, jet_pt, jet_abseta);
                        double eff = btag_sfs->getEff(flavor, jet_pt, jet_abseta);
                        btag_sf *= (1 - sf*eff)/(1 - eff);
                        btag_sf_up *= (1 - sf_up*eff)/(1 - eff);
                        btag_sf_dn *= (1 - sf_dn*eff)/(1 - eff);
                    }
                }
                if (deepflav_btag > gconf.WP_DeepFlav_loose)
                {
                    n_loose_b_jets++;
                }
            }
            // Save additional jet info
            n_jets++;
            ht += jet_p4.pt();
            good_jet_p4s.push_back(jet_p4);
            good_jet_idxs.push_back(jet_i);
        }
        double met = std::sqrt(std::pow(met_x, 2) + std::pow(met_y, 2));
        double met_up = std::sqrt(
            std::pow(met_x + nt.MET_MetUnclustEnUpDeltaX(), 2)
            + std::pow(met_y + nt.MET_MetUnclustEnUpDeltaY(), 2)
        );
        double met_dn = std::sqrt(
            std::pow(met_x - nt.MET_MetUnclustEnUpDeltaX(), 2)
            + std::pow(met_y - nt.MET_MetUnclustEnUpDeltaY(), 2)
        );
        arbol.setLeaf<double>("MET", met);
        arbol.setLeaf<double>("MET_up", met_up);
        arbol.setLeaf<double>("MET_dn", met_dn);

        globals.setVal<LorentzVectors>("good_jet_p4s", good_jet_p4s);
        globals.setVal<Integers>("good_jet_idxs", good_jet_idxs);

        arbol.setLeaf<double>("HT", ht);
        arbol.setLeaf<int>("n_loose_b_jets", n_loose_b_jets);
        arbol.setLeaf<int>("n_medium_b_jets", n_medium_b_jets);
        arbol.setLeaf<int>("n_tight_b_jets", n_tight_b_jets);
        arbol.setLeaf<int>("n_jets", n_jets);
        if (!nt.isData() && btag_sfs != nullptr)
        {
            arbol.setLeaf<double>("btag_sf", btag_sf);
            arbol.setLeaf<double>("btag_sf_up", btag_sf_up);
            arbol.setLeaf<double>("btag_sf_dn", btag_sf_dn);
        }
        else
        {
            arbol.setLeaf<double>("btag_sf", 1.);
            arbol.setLeaf<double>("btag_sf_up", 1.);
            arbol.setLeaf<double>("btag_sf_dn", 1.);
        }

        return true;
    };

    double weight()
    {
        return arbol.getLeaf<double>("btag_sf");
    };
};

class SelectFatJets : public AnalysisCut
{
public:
    SelectFatJets(std::string name, Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool isGoodFatJet(int fatjet_i)
    {
        if (nt.FatJet_pt().at(fatjet_i) <= 250) { return false; }
        if (nt.FatJet_mass().at(fatjet_i) <= 50) { return false; }
        if (nt.FatJet_msoftdrop().at(fatjet_i) <= 40) { return false; }
        if (fabs(nt.FatJet_eta().at(fatjet_i)) >= 2.5) { return false; }
        return true;
    }

    bool evaluate()
    {
        LorentzVectors good_fatjet_p4s;
        Integers good_fatjet_idxs;
        Doubles good_fatjet_hbbtags;
        Doubles good_fatjet_xbbtags;
        Doubles good_fatjet_masses;
        Doubles good_fatjet_msoftdrops;
        LorentzVectors veto_lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
        for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); ++fatjet_i)
        {
            // Basic requirements
            if (!isGoodFatJet(fatjet_i)) { continue; }
            // Remove lepton overlap
            LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
            bool is_overlap = false;
            for (auto& lep_p4 : veto_lep_p4s)
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

        arbol.setLeaf<int>("n_fatjets", good_fatjet_p4s.size());

        return true;
    };
};

class SelectVBSJetsMaxE : public AnalysisCut
{
public:
    SelectVBSJetsMaxE(std::string name, Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        // Select VBS jet candidates
        Integers vbsjet_cand_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            if (good_jet_p4s.at(jet_i).pt() >= 30.) { vbsjet_cand_idxs.push_back(jet_i); }
        }
        if (vbsjet_cand_idxs.size() < 2) { return false; }
        // Sort candidates by pt
        std::sort(
            vbsjet_cand_idxs.begin(), vbsjet_cand_idxs.end(),
            [&](int i, int j) -> bool { return good_jet_p4s.at(i).pt() > good_jet_p4s.at(j).pt(); }
        );
        // Process candidates
        std::pair<int, int> vbsjet_idxs;
        if (vbsjet_cand_idxs.size() == 2)
        {
            vbsjet_idxs = std::make_pair(vbsjet_cand_idxs.at(0), vbsjet_cand_idxs.at(1));
        }
        else
        {
            // Collect jets in pos/neg eta hemispheres
            Integers vbs_pos_eta_jet_idxs;
            Integers vbs_neg_eta_jet_idxs;
            for (auto& jet_i : vbsjet_cand_idxs)
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
                vbsjet_idxs = std::make_pair(vbs_neg_eta_jet_idxs.at(0), vbs_neg_eta_jet_idxs.at(1));
            }
            else if (vbs_neg_eta_jet_idxs.size() == 0)
            {
                // All candidates are in the +eta hemisphere
                vbsjet_idxs = std::make_pair(vbs_pos_eta_jet_idxs.at(0), vbs_pos_eta_jet_idxs.at(1));
            }
            else
            {
                // Take the leading candidate (in P) from each hemisphere
                vbsjet_idxs = std::make_pair(vbs_pos_eta_jet_idxs.at(0), vbs_neg_eta_jet_idxs.at(0));
            }
        }
        // Separate the two VBS jets into leading/trailing
        int ld_vbsjet_idx;
        int tr_vbsjet_idx;
        if (good_jet_p4s.at(vbsjet_idxs.first).pt() > good_jet_p4s.at(vbsjet_idxs.first).pt())
        {
            ld_vbsjet_idx = vbsjet_idxs.first;
            tr_vbsjet_idx = vbsjet_idxs.second;
        }
        else
        {
            ld_vbsjet_idx = vbsjet_idxs.second;
            tr_vbsjet_idx = vbsjet_idxs.first;
        }
        LorentzVector ld_vbsjet_p4 = good_jet_p4s.at(ld_vbsjet_idx);
        LorentzVector tr_vbsjet_p4 = good_jet_p4s.at(tr_vbsjet_idx);
        // Save VBS jet info
        globals.setVal<LorentzVector>("ld_vbsjet_p4", ld_vbsjet_p4);
        globals.setVal<int>("ld_vbsjet_idx", ld_vbsjet_idx);
        globals.setVal<LorentzVector>("tr_vbsjet_p4", tr_vbsjet_p4);
        globals.setVal<int>("tr_vbsjet_idx", tr_vbsjet_idx);

        arbol.setLeaf<double>("ld_vbsjet_pt", ld_vbsjet_p4.pt());
        arbol.setLeaf<double>("tr_vbsjet_pt", tr_vbsjet_p4.pt());
        arbol.setLeaf<double>("ld_vbsjet_eta", ld_vbsjet_p4.eta());
        arbol.setLeaf<double>("tr_vbsjet_eta", tr_vbsjet_p4.eta());
        arbol.setLeaf<double>("ld_vbsjet_phi", ld_vbsjet_p4.phi());
        arbol.setLeaf<double>("tr_vbsjet_phi", tr_vbsjet_p4.phi());
        arbol.setLeaf<double>("M_jj", (ld_vbsjet_p4 + tr_vbsjet_p4).M());
        arbol.setLeaf<double>("deta_jj", ld_vbsjet_p4.eta() - tr_vbsjet_p4.eta());
        arbol.setLeaf<double>("abs_deta_jj", fabs(ld_vbsjet_p4.eta() - tr_vbsjet_p4.eta()));
        arbol.setLeaf<double>("dR_jj", ROOT::Math::VectorUtil::DeltaR(ld_vbsjet_p4, tr_vbsjet_p4));

        return true;
    };
};

class SaveLHEScaleWeights : public AnalysisCut
{
public:
    SaveLHEScaleWeights(std::string name, Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        /* From Events->GetListOfBranches()->ls("LHEScaleWeight"):
           OBJ: TBranch   LHEScaleWeight  LHE scale variation weights (w_var / w_nominal); 
            [0] is MUF="0.5" MUR="0.5"; 
            [1] is MUF="1.0" MUR="0.5"; 
            [2] is MUF="2.0" MUR="0.5"; 
            [3] is MUF="0.5" MUR="1.0"; 
            [4] is MUF="1.0" MUR="1.0"; 
            [5] is MUF="2.0" MUR="1.0"; 
            [6] is MUF="0.5" MUR="2.0"; 
            [7] is MUF="1.0" MUR="2.0"; 
            [8] is MUF="2.0" MUR="2.0"
        */
        if (nt.isData() || !cli.is_signal) { return true; }
        if (nt.nLHEScaleWeight() == 9)
        {
            std::vector<float> scale_weights = nt.LHEScaleWeight();
            arbol.setLeaf<float>("lhe_muF0p5_muR0p5", scale_weights.at(0)); // MUF=0.5 MUR=0.5
            arbol.setLeaf<float>("lhe_muF1p0_muR0p5", scale_weights.at(1)); // MUF=1.0 MUR=0.5
            arbol.setLeaf<float>("lhe_muF2p0_muR0p5", scale_weights.at(2)); // MUF=2.0 MUR=0.5
            arbol.setLeaf<float>("lhe_muF0p5_muR1p0", scale_weights.at(3)); // MUF=0.5 MUR=1.0
            arbol.setLeaf<float>("lhe_muF1p0_muR1p0", scale_weights.at(4)); // MUF=1.0 MUR=1.0
            arbol.setLeaf<float>("lhe_muF2p0_muR1p0", scale_weights.at(5)); // MUF=2.0 MUR=1.0
            arbol.setLeaf<float>("lhe_muF0p5_muR2p0", scale_weights.at(6)); // MUF=0.5 MUR=2.0
            arbol.setLeaf<float>("lhe_muF1p0_muR2p0", scale_weights.at(7)); // MUF=1.0 MUR=2.0
            arbol.setLeaf<float>("lhe_muF2p0_muR2p0", scale_weights.at(8)); // MUF=2.0 MUR=2.0
        }
        else
        {
            // A handful of events are missing these, so we just set to 1
            arbol.setLeaf<float>("lhe_muF0p5_muR0p5", 1.);
            arbol.setLeaf<float>("lhe_muF1p0_muR0p5", 1.);
            arbol.setLeaf<float>("lhe_muF2p0_muR0p5", 1.);
            arbol.setLeaf<float>("lhe_muF0p5_muR1p0", 1.);
            arbol.setLeaf<float>("lhe_muF1p0_muR1p0", 1.);
            arbol.setLeaf<float>("lhe_muF2p0_muR1p0", 1.);
            arbol.setLeaf<float>("lhe_muF0p5_muR2p0", 1.);
            arbol.setLeaf<float>("lhe_muF1p0_muR2p0", 1.);
            arbol.setLeaf<float>("lhe_muF2p0_muR2p0", 1.);
        }
        return true;
    };
};

}; // End namespace Core

#endif
