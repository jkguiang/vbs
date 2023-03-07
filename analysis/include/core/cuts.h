#ifndef CORE_CUTS_H
#define CORE_CUTS_H

// RAPIDO
#include "arbol.h"
#include "arbusto.h"
#include "cutflow.h"
#include "hepcli.h"
#include "utilities.h"          // Utilities::Variables
// VBS
#include "core/collections.h"   // Core::Core::Analysis, Core::Skimmer
#include "core/pku.h"           // PKU::IDLevel, PKU::passesElecID, PKU::passesMuonID
#include "corrections/all.h"    // PileUpSFs, LeptonSFsTTH/PKU, BTagSFs, JetEnergyScales
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"    // DeltaR
// NanoCORE
#include "Nano.h"
#include "Config.h"             // gconf
#include "ElectronSelections.h" // ttH_UL::electronID
#include "MuonSelections.h"     // ttH_UL::muonID
#include "Tools/goodrun.h"

namespace Core
{

class SkimmerCut : public Cut
{
public:
    Arbusto& arbusto;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;

    SkimmerCut(std::string new_name, Core::Skimmer& s) 
    : Cut(new_name), arbusto(s.arbusto), nt(s.nt), cli(s.cli), globals(s.cutflow.globals)
    {
        // Do nothing
    };
};

class AnalysisCut : public Cut
{
public:
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;

    AnalysisCut(std::string new_name, Core::Analysis& a) 
    : Cut(new_name), arbol(a.arbol), nt(a.nt), cli(a.cli), globals(a.cutflow.globals)
    {
        // Do nothing
    };
};

class Bookkeeping : public AnalysisCut
{
public:
    PileUpSFs* pu_sfs;

    Bookkeeping(std::string name, Core::Analysis& analysis, PileUpSFs* pu_sfs = nullptr) 
    : AnalysisCut(name, analysis) 
    {
        this->pu_sfs = pu_sfs;
    };

    bool evaluate()
    {
        arbol.setLeaf<int>("event", nt.event());
        arbol.setLeaf<double>("xsec_sf", (nt.isData()) ? 1. : cli.scale_factor*nt.genWeight());
        arbol.setLeaf<double>("prefire_sf", (nt.isData()) ? 1. : nt.L1PreFiringWeight_Nom());
        arbol.setLeaf<double>("prefire_sf_up", (nt.isData()) ? 1. : nt.L1PreFiringWeight_Up());
        arbol.setLeaf<double>("prefire_sf_dn", (nt.isData()) ? 1. : nt.L1PreFiringWeight_Dn());
        arbol.setLeaf<int>("year", (nt.year() == 2016 && gconf.isAPV) ? -nt.year() : nt.year());
        if (!nt.isData() && pu_sfs != nullptr)
        {
            arbol.setLeaf<double>("pu_sf", pu_sfs->getSF(nt.Pileup_nTrueInt()));
            arbol.setLeaf<double>("pu_sf_up", pu_sfs->getSFUp(nt.Pileup_nTrueInt()));
            arbol.setLeaf<double>("pu_sf_dn", pu_sfs->getSFDn(nt.Pileup_nTrueInt()));
        }
        else
        {
            arbol.setLeaf<double>("pu_sf", 1.);
            arbol.setLeaf<double>("pu_sf_up", 1.);
            arbol.setLeaf<double>("pu_sf_dn", 1.);
        }
        return (nt.isData()) ? goodrun(nt.run(), nt.luminosityBlock()) : true;
    };

    double weight()
    {
        if (nt.isData()) 
        {
            return 1.;
        }
        else
        {
            return (
                arbol.getLeaf<double>("xsec_sf")
                *arbol.getLeaf<double>("pu_sf")
                *arbol.getLeaf<double>("prefire_sf")
            );
        }
    };
};

class SelectLeptons : public AnalysisCut
{
public:
    SelectLeptons(std::string name, Core::Analysis& analysis) : AnalysisCut(name, analysis)
    {
        // Do nothing
    };

    virtual bool passesVetoElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDveto, nt.year());
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDveto, nt.year());
    };

    bool evaluate()
    {
        LorentzVectors veto_lep_p4s;
        Integers veto_lep_pdgIDs;
        Integers veto_lep_idxs;
        Integers veto_lep_jet_idxs;
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
        }

        globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
        globals.setVal<Integers>("veto_lep_pdgIDs", veto_lep_pdgIDs);
        globals.setVal<Integers>("veto_lep_idxs", veto_lep_idxs);
        globals.setVal<Integers>("veto_lep_jet_idxs", veto_lep_jet_idxs);

        return true;
    };
};

class SelectLeptonsPKU : public SelectLeptons
{
public:
    SelectLeptonsPKU(std::string name, Core::Analysis& analysis) 
    : Core::SelectLeptons(name, analysis) 
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
    PileUpJetIDSFs* puid_sfs;
    LorentzVectors veto_lep_p4s;
    Integers veto_lep_jet_idxs;

    SelectJets(std::string name, Core::Analysis& analysis, JetEnergyScales* jes = nullptr, BTagSFs* btag_sfs = nullptr,
               PileUpJetIDSFs* puid_sfs = nullptr) 
    : AnalysisCut(name, analysis)
    {
        this->jes = jes;
        this->btag_sfs = btag_sfs;
        this->puid_sfs = puid_sfs;
    };

    virtual bool isGoodJet(int jet_i, LorentzVector jet_p4)
    {
        if (jet_p4.pt() <= 20) { return false; }
        int jet_id = nt.Jet_jetId().at(jet_i);
        if (nt.year() == 2016 && jet_id < 1) { return false; }
        if (nt.year() > 2016 && jet_id < 2) { return false; }
        if (jet_p4.pt() < 50)
        {
            if (nt.Jet_puId().at(jet_i) == 0) { return false; }
        }
        return true;
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
        double puid_sf = 1.;
        double puid_sf_up = 1.;
        double puid_sf_dn = 1.;
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
            // Apply HEM prescription
            if (!nt.isData()
                && nt.year() == 2018
                && nt.event() % 1961 < 1286 
                && jet_p4.phi() > -1.57 && jet_p4.phi() < -0.87)
            {
                double jet_eta = jet_p4.eta();
                if (jet_eta > -2.5 && jet_eta < -1.3)
                {
                    jet_p4 *= 0.8;
                }
                else if (jet_eta > -3.0 && jet_eta < -2.5)
                {
                    jet_p4 *= 0.65;
                }
            }
            // Apply JECs
            if (jes != nullptr)
            {
                jet_p4 = jes->applyAK4JEC(jet_p4);
            }
            // Add corrected jet pt
            met_x += jet_p4.px();
            met_y += jet_p4.py();
            // Apply JERs
            if (!nt.isData() && jes != nullptr)
            {
                jet_p4 = jes->applyJER(
                    jer_seed, 
                    jet_p4, 
                    nt.fixedGridRhoFastjetAll(), 
                    nt.GenJet_p4()
                );
            }
            // Select good jets
            if (!isGoodJet(jet_i, jet_p4)) { continue; }
            if (isOverlap(jet_i, jet_p4)) { continue; }
            // Apply PU jet ID scale factors
            double jet_pt = jet_p4.pt();
            double jet_eta = jet_p4.eta();
            if (!nt.isData() && puid_sfs != nullptr && jet_pt < 50 && nt.Jet_puId().at(jet_i) > 0)
            {
                for (auto genjet_p4 : nt.GenJet_p4())
                {
                    if (ROOT::Math::VectorUtil::DeltaR(jet_p4, genjet_p4) < 0.4)
                    {
                        puid_sf *= puid_sfs->getSF(jet_pt, jet_eta, "L");
                        puid_sf_up *= puid_sfs->getSFUp(jet_pt, jet_eta, "L");
                        puid_sf_dn *= puid_sfs->getSFDn(jet_pt, jet_eta, "L");
                        break;
                    }
                }
            }
            // Perform b-tagging (for b-veto); only possible for pt > 20 GeV and |eta| < 2.4
            if (fabs(jet_p4.eta()) < 2.4 && jet_p4.pt() > 20) 
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
                    // Apply DeepJet b-tagging scale factor (for a VETO using the medium WP)
                    if (!nt.isData() && btag_sfs != nullptr)
                    {
                        int flavor = nt.Jet_hadronFlavour().at(jet_i);
                        double jet_abseta = fabs(jet_eta);
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
        if (!nt.isData())
        {
            arbol.setLeaf<double>("btag_sf", btag_sf);
            arbol.setLeaf<double>("btag_sf_up", btag_sf_up);
            arbol.setLeaf<double>("btag_sf_dn", btag_sf_dn);
            arbol.setLeaf<double>("puid_sf", puid_sf);
            arbol.setLeaf<double>("puid_sf_up", puid_sf_up);
            arbol.setLeaf<double>("puid_sf_dn", puid_sf_dn);
        }
        else
        {
            arbol.setLeaf<double>("btag_sf", 1.);
            arbol.setLeaf<double>("btag_sf_up", 1.);
            arbol.setLeaf<double>("btag_sf_dn", 1.);
            arbol.setLeaf<double>("puid_sf", 1.);
            arbol.setLeaf<double>("puid_sf_up", 1.);
            arbol.setLeaf<double>("puid_sf_dn", 1.);
        }

        return true;
    };

    double weight()
    {
        return arbol.getLeaf<double>("btag_sf")*arbol.getLeaf<double>("puid_sf");
    };
};

class SelectFatJets : public AnalysisCut
{
public:
    JetEnergyScales* jes;

    SelectFatJets(std::string name, Core::Analysis& analysis, JetEnergyScales* jes = nullptr) 
    : AnalysisCut(name, analysis) 
    {
        this->jes = jes;
    };

    virtual bool isGoodFatJet(int fatjet_i, LorentzVector fatjet_p4)
    {
        if (fatjet_p4.pt() <= 300) { return false; }
        if (fabs(fatjet_p4.eta()) >= 2.5) { return false; }
        if (fatjet_p4.mass() <= 50) { return false; }
        if (nt.FatJet_msoftdrop().at(fatjet_i) <= 40) { return false; }
        if (nt.FatJet_jetId().at(fatjet_i) <= 0) { return false; }
        return true;
    };

    bool evaluate()
    {
        LorentzVectors good_fatjet_p4s;
        Integers good_fatjet_idxs;
        Doubles good_fatjet_wqqtags;
        Doubles good_fatjet_zqqtags;
        Doubles good_fatjet_hbbtags;
        Doubles good_fatjet_xbbtags;
        Doubles good_fatjet_xqqtags;
        Doubles good_fatjet_xcctags;
        Doubles good_fatjet_xwqqtags;
        Doubles good_fatjet_xvqqtags;
        Doubles good_fatjet_masses;
        Doubles good_fatjet_msoftdrops;
        LorentzVectors veto_lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
        for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); ++fatjet_i)
        {
            LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
            // Apply HEM prescription
            if (!nt.isData()
                && nt.year() == 2018
                && nt.event() % 1961 < 1286 
                && fatjet_p4.phi() > -1.57 && fatjet_p4.phi() < -0.87)
            {
                double fatjet_eta = fatjet_p4.eta();
                if (fatjet_eta > -2.5 && fatjet_eta < -1.3)
                {
                    fatjet_p4 *= 0.8;
                }
                else if (fatjet_eta > -3.0 && fatjet_eta < -2.5)
                {
                    fatjet_p4 *= 0.65;
                }
            }
            // Apply jet energy corrections
            if (jes != nullptr)
            {
                fatjet_p4 = jes->applyAK8JEC(fatjet_p4);
            }

            // Basic requirements
            if (!isGoodFatJet(fatjet_i, fatjet_p4)) { continue; }

            // Remove lepton overlap
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

            double pnet_xbb = nt.FatJet_particleNetMD_Xbb().at(fatjet_i);
            double pnet_xqq = nt.FatJet_particleNetMD_Xqq().at(fatjet_i);
            double pnet_xcc = nt.FatJet_particleNetMD_Xcc().at(fatjet_i);
            double pnet_qcd = nt.FatJet_particleNetMD_QCD().at(fatjet_i);

            // Store good fat jets
            good_fatjet_p4s.push_back(fatjet_p4);
            good_fatjet_idxs.push_back(fatjet_i);
            good_fatjet_wqqtags.push_back(nt.FatJet_particleNet_WvsQCD().at(fatjet_i));
            good_fatjet_zqqtags.push_back(nt.FatJet_particleNet_ZvsQCD().at(fatjet_i));
            good_fatjet_hbbtags.push_back(nt.FatJet_particleNet_HbbvsQCD().at(fatjet_i));
            good_fatjet_xbbtags.push_back(pnet_xbb/(pnet_xbb + pnet_qcd));
            good_fatjet_xqqtags.push_back(pnet_xqq/(pnet_xqq + pnet_qcd));
            good_fatjet_xcctags.push_back(pnet_xcc/(pnet_xcc + pnet_qcd));
            good_fatjet_xwqqtags.push_back((pnet_xcc + pnet_xqq)/(pnet_xcc + pnet_xqq + pnet_qcd));
            good_fatjet_xvqqtags.push_back((pnet_xbb + pnet_xcc + pnet_xqq)/(pnet_xbb + pnet_xcc + pnet_xqq + pnet_qcd));
            good_fatjet_masses.push_back(nt.FatJet_particleNet_mass().at(fatjet_i));
            good_fatjet_msoftdrops.push_back(nt.FatJet_msoftdrop().at(fatjet_i));
        }
        globals.setVal<LorentzVectors>("good_fatjet_p4s", good_fatjet_p4s);
        globals.setVal<Integers>("good_fatjet_idxs", good_fatjet_idxs);
        globals.setVal<Doubles>("good_fatjet_wqqtags", good_fatjet_wqqtags);
        globals.setVal<Doubles>("good_fatjet_zqqtags", good_fatjet_zqqtags);
        globals.setVal<Doubles>("good_fatjet_hbbtags", good_fatjet_hbbtags);
        globals.setVal<Doubles>("good_fatjet_xbbtags", good_fatjet_xbbtags);
        globals.setVal<Doubles>("good_fatjet_xqqtags", good_fatjet_xqqtags);
        globals.setVal<Doubles>("good_fatjet_xcctags", good_fatjet_xcctags);
        globals.setVal<Doubles>("good_fatjet_xwqqtags", good_fatjet_xwqqtags);
        globals.setVal<Doubles>("good_fatjet_xvqqtags", good_fatjet_xvqqtags);
        globals.setVal<Doubles>("good_fatjet_masses", good_fatjet_masses);
        globals.setVal<Doubles>("good_fatjet_msoftdrops", good_fatjet_msoftdrops);

        arbol.setLeaf<int>("n_fatjets", good_fatjet_p4s.size());

        return true;
    };
};

class SelectVBSJets : public AnalysisCut
{
public:
    SelectVBSJets(std::string name, Core::Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    virtual std::vector<unsigned int> getVBSCandidates()
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        std::vector<unsigned int> vbsjet_cand_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            LorentzVector jet_p4 = good_jet_p4s.at(jet_i);
            if (jet_p4.pt() >= 30. && fabs(jet_p4.eta()) < 4.7) 
            {
                vbsjet_cand_idxs.push_back(jet_i); 
            }
        }
        return vbsjet_cand_idxs;
    };

    virtual std::pair<unsigned int, unsigned int> getVBSPair(std::vector<unsigned int> vbsjet_cand_idxs)
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        double max_detajj = -999;
        std::pair<unsigned int, unsigned int> vbsjet_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            for (unsigned int jet_j = jet_i + 1; jet_j < good_jet_p4s.size(); ++jet_j)
            {
                LorentzVector jet1_p4 = good_jet_p4s.at(jet_i);
                LorentzVector jet2_p4 = good_jet_p4s.at(jet_j);
                double detajj = fabs(jet1_p4.eta() - jet2_p4.eta());
                if (detajj > max_detajj)
                {
                    max_detajj = detajj;
                    vbsjet_idxs = std::make_pair(jet_i, jet_j);
                }
            }
        }
        return vbsjet_idxs;
    };

    bool evaluate()
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");

        // Get VBS jet candidates
        std::vector<unsigned int> vbsjet_cand_idxs = getVBSCandidates();
        if (vbsjet_cand_idxs.size() < 2) { return false; }

        // Select final VBS jet pair
        std::pair<unsigned int, unsigned int> vbsjet_idxs = getVBSPair(vbsjet_cand_idxs);

        // Sort the two VBS jets into leading/trailing
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

        // Save VBS jet globals
        globals.setVal<LorentzVector>("ld_vbsjet_p4", ld_vbsjet_p4);
        globals.setVal<int>("ld_vbsjet_idx", ld_vbsjet_idx);
        globals.setVal<LorentzVector>("tr_vbsjet_p4", tr_vbsjet_p4);
        globals.setVal<int>("tr_vbsjet_idx", tr_vbsjet_idx);
        // Set VBS jet leaves
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

class SelectVBSJetsMaxMjj : public SelectVBSJets
{
public:
    SelectVBSJetsMaxMjj(std::string name, Core::Analysis& analysis) : SelectVBSJets(name, analysis) 
    {
        // Do nothing
    };

    std::pair<unsigned int, unsigned int> getVBSPair(std::vector<unsigned int> vbsjet_cand_idxs)
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        double max_Mjj = -999;
        std::pair<unsigned int, unsigned int> vbsjet_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            for (unsigned int jet_j = jet_i + 1; jet_j < good_jet_p4s.size(); ++jet_j)
            {
                LorentzVector jet1_p4 = good_jet_p4s.at(jet_i);
                LorentzVector jet2_p4 = good_jet_p4s.at(jet_j);
                double Mjj = (jet1_p4 + jet2_p4).M();
                if (Mjj > max_Mjj)
                {
                    max_Mjj = Mjj;
                    vbsjet_idxs = std::make_pair(jet_i, jet_j);
                }
            }
        }
        return vbsjet_idxs;
    };
};

class SelectVBSJetsMaxE : public SelectVBSJets
{
public:
    SelectVBSJetsMaxE(std::string name, Core::Analysis& analysis) : SelectVBSJets(name, analysis) 
    {
        // Do nothing
    };

    std::pair<unsigned int, unsigned int> getVBSPair(std::vector<unsigned int> vbsjet_cand_idxs)
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        // Sort candidates by pt
        std::sort(
            vbsjet_cand_idxs.begin(), vbsjet_cand_idxs.end(),
            [&](int i, int j) -> bool { return good_jet_p4s.at(i).pt() > good_jet_p4s.at(j).pt(); }
        );
        // Process candidates
        std::pair<unsigned int, unsigned int> vbsjet_idxs;
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
        return vbsjet_idxs;
    };

    /* Refactored
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
        // Save VBS jet globals
        globals.setVal<LorentzVector>("ld_vbsjet_p4", ld_vbsjet_p4);
        globals.setVal<int>("ld_vbsjet_idx", ld_vbsjet_idx);
        globals.setVal<LorentzVector>("tr_vbsjet_p4", tr_vbsjet_p4);
        globals.setVal<int>("tr_vbsjet_idx", tr_vbsjet_idx);
        // Set VBS jet leaves
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
    */
};

class SaveSystWeights : public AnalysisCut
{
public:
    SaveSystWeights(std::string name, Core::Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        if (nt.isData()) { return true; }

        /* From Events->GetListOfBranches()->ls("LHEScaleWeight*"):
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

        /* From Events->GetListOfBranches()->ls("PSWeight*"):
           OBJ: TBranch   PSWeight    PS weights (w_var / w_nominal);   
            [0] is ISR=2 FSR=1; 
            [1] is ISR=1 FSR=2;
            [2] is ISR=0.5 FSR=1; 
            [3] is ISR=1 FSR=0.5;
        */
        if (nt.nPSWeight() == 4)
        {
            std::vector<float> ps_weights = nt.PSWeight();
            arbol.setLeaf<float>("ps_isr2p0_fsr1p0", ps_weights.at(0)); // ISR=2 FSR=1
            arbol.setLeaf<float>("ps_isr1p0_fsr2p0", ps_weights.at(1)); // ISR=1 FSR=2
            arbol.setLeaf<float>("ps_isr0p5_fsr1p0", ps_weights.at(2)); // ISR=0.5 FSR=1
            arbol.setLeaf<float>("ps_isr1p0_fsr0p5", ps_weights.at(3)); // ISR=1 FSR=0.5
        }
        else
        {
            arbol.setLeaf<float>("ps_isr2p0_fsr1p0", 1.);
            arbol.setLeaf<float>("ps_isr1p0_fsr2p0", 1.);
            arbol.setLeaf<float>("ps_isr0p5_fsr1p0", 1.);
            arbol.setLeaf<float>("ps_isr1p0_fsr0p5", 1.);
        }
        return true;
    };
};

}; // End namespace Core

#endif
