#ifndef VBSWH_CUTS_H
#define VBSWH_CUTS_H

// RAPIDO
#include "arbol.h"
#include "cutflow.h"
#include "utilities.h"
// VBS
#include "core/collections.h"
#include "core/cuts.h"
#include "core/pku.h"
#include "corrections/all.h"

namespace VBSWH
{

class FindLeptons : public Core::SkimmerCut
{
public:
    FindLeptons(std::string name, Core::Skimmer& skimmer) : Core::SkimmerCut(name, skimmer) 
    {
        // Do nothing
    };

    virtual bool passesVetoElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDveto, nt.year()); // same as ttH_UL
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDveto, nt.year());     // same as ttH_UL
    };

    virtual bool passesTightElecID(int elec_i)
    {
        return ttH_UL::electronID(elec_i, ttH::IDtight, nt.year());
    };

    virtual bool passesTightMuonID(int muon_i)
    {
        return ttH_UL::muonID(muon_i, ttH::IDtight, nt.year());
    };

    bool evaluate()
    {
        LorentzVectors veto_lep_p4s;
        LorentzVectors tight_lep_p4s;
        Integers veto_lep_pdgIDs;
        Integers tight_lep_pdgIDs;
        for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
        {
            LorentzVector lep_p4 = nt.Electron_p4().at(elec_i);
            if (passesVetoElecID(elec_i)) 
            { 
                veto_lep_p4s.push_back(lep_p4); 
                veto_lep_pdgIDs.push_back(-nt.Electron_charge().at(elec_i)*11);
            }
            if (passesTightElecID(elec_i)) 
            { 
                tight_lep_p4s.push_back(lep_p4); 
                tight_lep_pdgIDs.push_back(-nt.Electron_charge().at(elec_i)*11);
            }
        }
        for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
        {
            LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
            if (passesVetoMuonID(muon_i)) 
            { 
                veto_lep_p4s.push_back(lep_p4); 
                veto_lep_pdgIDs.push_back(-nt.Muon_charge().at(muon_i)*13); 
            }
            if (passesTightMuonID(muon_i)) 
            { 
                tight_lep_p4s.push_back(lep_p4); 
                tight_lep_pdgIDs.push_back(-nt.Muon_charge().at(muon_i)*13); 
            }
        }
        globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
        globals.setVal<LorentzVectors>("tight_lep_p4s", tight_lep_p4s);
        globals.setVal<Integers>("veto_lep_pdgIDs", veto_lep_pdgIDs);
        globals.setVal<Integers>("tight_lep_pdgIDs", tight_lep_pdgIDs);
        return true;
    };
};

class FindLeptonsPKU : public FindLeptons
{
public:
    FindLeptonsPKU(std::string name, Core::Skimmer& skimmer) : FindLeptons(name, skimmer) 
    {
        // Do nothing
    };

    virtual bool passesVetoElecID(int elec_i)
    {
        return PKU::passesElecID(elec_i, PKU::IDveto);
    };

    virtual bool passesTightElecID(int elec_i)
    {
        return PKU::passesElecID(elec_i, PKU::IDtight);
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return PKU::passesMuonID(muon_i, PKU::IDveto);
    };

    virtual bool passesTightMuonID(int muon_i)
    {
        return PKU::passesMuonID(muon_i, PKU::IDtight);
    };
};

class Geq2Jets : public Core::SkimmerCut
{
public:
    Geq2Jets(std::string name, Core::Skimmer& skimmer) : Core::SkimmerCut(name, skimmer) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
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
    };
};

class Geq1FatJetLoose : public Core::SkimmerCut
{
public:
    Geq1FatJetLoose(std::string name, Core::Skimmer& skimmer) : Core::SkimmerCut(name, skimmer) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
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
    };
};

class Exactly1Lepton : public Core::SkimmerCut
{
public:
    Exactly1Lepton(std::string name, Core::Skimmer& skimmer) : Core::SkimmerCut(name, skimmer) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        return (
            globals.getVal<LorentzVectors>("veto_lep_p4s").size() == 1 
            && globals.getVal<LorentzVectors>("tight_lep_p4s").size() == 1
        );
    };
};

class Geq1FatJetTight : public Core::SkimmerCut
{
public:
    Geq1FatJetTight(std::string name, Core::Skimmer& skimmer) : Core::SkimmerCut(name, skimmer) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVector lep_p4 = globals.getVal<LorentzVectors>("tight_lep_p4s").at(0);
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

            // Find candidate with highest Xbb score
            double pnet_xbb = nt.FatJet_particleNetMD_Xbb().at(fatjet_i);
            double pnet_qcd = nt.FatJet_particleNetMD_QCD().at(fatjet_i);
            double xbb_score = pnet_xbb/(pnet_xbb + pnet_qcd);
            if (xbb_score > hbbjet_score)
            {
                hbbjet_score = xbb_score;
                hbbjet_p4 = fatjet_p4;
            }
        }
        if (n_fatjets >= 1)
        {
            globals.setVal<LorentzVector>("hbbjet_p4", hbbjet_p4);
            globals.setVal<double>("ST", hbbjet_p4.pt() + lep_p4.pt() + nt.MET_pt());
            return true;
        }
        else
        {
            return false;
        }
    };
};

class PassesEventFilters : public Core::AnalysisCut
{
public:
    PassesEventFilters(std::string name, Core::Analysis& analysis) : Core::AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        bool passed = (
            nt.Flag_goodVertices()
            && nt.Flag_HBHENoiseFilter()
            && nt.Flag_HBHENoiseIsoFilter()
            && nt.Flag_EcalDeadCellTriggerPrimitiveFilter()
            && nt.Flag_BadPFMuonFilter()
            && nt.Flag_BadPFMuonDzFilter()
            && nt.Flag_hfNoisyHitsFilter()
            && nt.Flag_eeBadScFilter()
        );
        if (nt.year() > 2016)
        {
            passed = passed && nt.Flag_ecalBadCalibFilter();
        }
        if (nt.isData())
        {
            passed = passed && nt.Flag_globalSuperTightHalo2016Filter();
        }
        return passed;
    };
};

class Passes1LepTriggers : public Core::AnalysisCut
{
public:
    HLT1LepSFs* hlt_sfs;

    Passes1LepTriggers(std::string name, Core::Analysis& analysis, HLT1LepSFs* hlt_sfs = nullptr) 
    : Core::AnalysisCut(name, analysis) 
    {
        this->hlt_sfs = hlt_sfs;
    };

    bool passesMuonTriggers()
    {
        bool passed = false;
        switch (nt.year())
        {
        case (2016):
            try { passed = (passed || nt.HLT_IsoMu24()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            try { passed = (passed || nt.HLT_IsoTkMu24()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            break;
        case (2017):
            try { passed = (passed || nt.HLT_IsoMu27()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            break;
        case (2018):
            try { passed = (passed || nt.HLT_IsoMu24()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            break;
        }
        return passed;
    };

    bool passesElecTriggers()
    {
        bool passed = false;
        switch (nt.year())
        {
        case (2016):
            try { passed = (passed || nt.HLT_Ele27_WPTight_Gsf()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            break;
        case (2017):
            try { passed = (passed || nt.HLT_Ele32_WPTight_Gsf_L1DoubleEG()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            break;
        case (2018):
            try { passed = (passed || nt.HLT_Ele32_WPTight_Gsf()); }
            catch (const runtime_error& error) { /* Do nothing */ }
            break;
        }
        return (passed && !passesMuonTriggers());
    };

    bool passesLepTriggers(unsigned int abs_lep_pdgID)
    {
        if (!nt.isData()) 
        { 
            /* Below is what was done for SS, but PKU does what is currently implemented
            switch (abs_lep_pdgID)
            {
            case (11):
                return passesElecTriggers();
                break;
            case (13):
                return passesMuonTriggers();
                break;
            default:
                return true;
                break;
            }
            */
            return (passesMuonTriggers() || passesElecTriggers());
        }
        else
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            if (file_name.Contains("SingleMuon"))
            {
                return passesMuonTriggers();
            }
            else if (file_name.Contains("SingleElectron") || file_name.Contains("EGamma"))
            {
                return (passesElecTriggers() && !passesMuonTriggers());
            }
            else
            {
                return (passesMuonTriggers() || passesElecTriggers());
            }
        }
    };

    bool evaluate()
    {
        unsigned int abs_lep_pdgID = abs(arbol.getLeaf<int>("lep_pdgID"));
        bool passed = passesLepTriggers(abs_lep_pdgID);
        if (!nt.isData() && hlt_sfs != nullptr && passed)
        {
            double pt = arbol.getLeaf<double>("lep_pt");
            double eta = arbol.getLeaf<double>("lep_eta");
            switch (abs_lep_pdgID)
            {
            case (11):
                arbol.setLeaf<double>("trig_sf", hlt_sfs->getElecSF(pt, eta));
                arbol.setLeaf<double>("trig_sf_up", hlt_sfs->getElecErrUp(pt, eta));
                arbol.setLeaf<double>("trig_sf_dn", hlt_sfs->getElecErrDn(pt, eta));
                break;
            case (13):
                arbol.setLeaf<double>("trig_sf", hlt_sfs->getMuonSF(pt, eta));
                arbol.setLeaf<double>("trig_sf_up", hlt_sfs->getMuonErrUp(pt, eta));
                arbol.setLeaf<double>("trig_sf_dn", hlt_sfs->getMuonErrDn(pt, eta));
                break;
            default:
                break;
            }
        }
        else
        {
            arbol.setLeaf<double>("trig_sf", 1.);
            arbol.setLeaf<double>("trig_sf_up", 1.);
            arbol.setLeaf<double>("trig_sf_dn", 1.);
        }
        return passed;
    };

    double weight()
    {
        return arbol.getLeaf<double>("trig_sf");
    };
};

class SelectHbbFatJet : public Core::AnalysisCut
{
public:
    bool use_md;

    SelectHbbFatJet(std::string name, Core::Analysis& analysis, bool md = false) 
    : Core::AnalysisCut(name, analysis) 
    {
        use_md = md;
    };

    bool evaluate()
    {
        // Select fatjet with best (highest) ParticleNet score
        int best_hbbjet_i = -1;
        double best_hbbjet_score = -999.;
        if (use_md)
        {
            Doubles xbbtags = globals.getVal<Doubles>("good_fatjet_xbbtags");
            best_hbbjet_i = std::distance(
                xbbtags.begin(), 
                std::max_element(xbbtags.begin(), xbbtags.end())
            );
            best_hbbjet_score = xbbtags.at(best_hbbjet_i);
        }
        else
        {
            Doubles hbbtags = globals.getVal<Doubles>("good_fatjet_hbbtags");
            best_hbbjet_i = std::distance(
                hbbtags.begin(), 
                std::max_element(hbbtags.begin(), hbbtags.end())
            );
            best_hbbjet_score = hbbtags.at(best_hbbjet_i);
        }
        if (best_hbbjet_i < 0) { return false; }
        // Find number of gen-level b quarks in Hbb jet cone
        LorentzVector best_hbbjet_p4 = globals.getVal<LorentzVectors>("good_fatjet_p4s").at(best_hbbjet_i);
        int n_hbbjet_genbquarks = 0;
        if (!nt.isData())
        {
            for (unsigned int gen_i = 0; gen_i < nt.nGenPart(); ++gen_i)
            {
                if (abs(nt.GenPart_pdgId().at(gen_i)) != 5) { continue; }
                if (nt.GenPart_status().at(gen_i) != 23) { continue; }
                if (abs(nt.GenPart_pdgId().at(nt.GenPart_genPartIdxMother().at(gen_i))) == 5) { continue; }

                LorentzVector genpart_p4 = nt.GenPart_p4().at(gen_i);
                if (ROOT::Math::VectorUtil::DeltaR(best_hbbjet_p4, genpart_p4) < 0.8)
                {
                    n_hbbjet_genbquarks++;
                }
            }
        }

        // Store the fatjet
        globals.setVal<LorentzVector>("hbbjet_p4", best_hbbjet_p4);
        arbol.setLeaf<int>("n_hbbjet_genbquarks", n_hbbjet_genbquarks);
        arbol.setLeaf<double>("hbbjet_score", best_hbbjet_score);
        arbol.setLeaf<double>("hbbjet_pt", best_hbbjet_p4.pt());
        arbol.setLeaf<double>("hbbjet_eta", best_hbbjet_p4.eta());
        arbol.setLeaf<double>("hbbjet_phi", best_hbbjet_p4.phi());
        arbol.setLeaf<double>("hbbjet_mass", globals.getVal<Doubles>("good_fatjet_masses").at(best_hbbjet_i));
        arbol.setLeaf<double>("hbbjet_msoftdrop", globals.getVal<Doubles>("good_fatjet_msoftdrops").at(best_hbbjet_i));

        return true;
    };
};

class SelectJetsNoHbbOverlap : public Core::SelectJets
{
public:
    LorentzVector hbbjet_p4;

    SelectJetsNoHbbOverlap(std::string name, Core::Analysis& analysis, JetEnergyScales* jes = nullptr, 
                           BTagSFs* btag_sfs = nullptr) 
    : Core::SelectJets(name, analysis, jes, btag_sfs) 
    {
        // Do nothing
    };

    void loadOverlapVars()
    {
        veto_lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
        veto_lep_jet_idxs = globals.getVal<Integers>("veto_lep_jet_idxs");
        hbbjet_p4 = globals.getVal<LorentzVector>("hbbjet_p4");
    };

    bool overlapsHbbJet(LorentzVector jet_p4)
    {
        return ROOT::Math::VectorUtil::DeltaR(hbbjet_p4, jet_p4) < 0.8;
    };

    bool isOverlap(int jet_i, LorentzVector jet_p4)
    {
        return overlapsLepton(jet_i, jet_p4) || overlapsHbbJet(jet_p4);
    };
};

class Has1Lep : public Core::AnalysisCut
{
public:
    LeptonSFs* lep_sfs;

    Has1Lep(std::string name, Core::Analysis& analysis, LeptonSFs* lep_sfs = nullptr) 
    : Core::AnalysisCut(name, analysis) 
    {
        this->lep_sfs = lep_sfs;
    };

    virtual bool passesTightElecID(int elec_i)
    {
        return ttH_UL::electronID(elec_i, ttH::IDtight, nt.year());
    };

    virtual bool passesTightMuonID(int muon_i)
    {
        return ttH_UL::muonID(muon_i, ttH::IDtight, nt.year());
    };

    virtual void applySFs(unsigned int abs_lep_pdgID, double lep_pt, double lep_eta)
    {
        double lep_sf = 1.;
        double lep_sf_up = 1.;
        double lep_sf_dn = 1.;
        if (abs_lep_pdgID == 11)
        {
            lep_sf = lep_sfs->getElecSF(lep_pt, lep_eta);
            lep_sf_up = lep_sf*(1. + lep_sfs->getElecErrUp(lep_pt, lep_eta));
            lep_sf_dn = lep_sf*(1. - lep_sfs->getElecErrDn(lep_pt, lep_eta));
        }
        else if (abs_lep_pdgID == 13)
        {
            lep_sf = lep_sfs->getMuonSF(lep_pt, lep_eta);
            lep_sf_up = lep_sf*(1. + lep_sfs->getMuonErrUp(lep_pt, lep_eta));
            lep_sf_dn = lep_sf*(1. - lep_sfs->getMuonErrDn(lep_pt, lep_eta));
        }
        arbol.setLeaf<double>("lep_id_sf", lep_sf);
        arbol.setLeaf<double>("lep_id_sf_up", lep_sf_up);
        arbol.setLeaf<double>("lep_id_sf_dn", lep_sf_dn);
    };

    virtual bool evaluate()
    {
        LorentzVectors veto_lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
        Integers veto_lep_pdgIDs = globals.getVal<Integers>("veto_lep_pdgIDs");
        Integers veto_lep_idxs = globals.getVal<Integers>("veto_lep_idxs");
        int n_tight_leps = 0;
        int tight_lep_idx = -999;
        for (unsigned int veto_lep_i = 0; veto_lep_i < veto_lep_p4s.size(); ++veto_lep_i)
        {
            unsigned int lep_i = veto_lep_idxs.at(veto_lep_i);
            int lep_pdgID = veto_lep_pdgIDs.at(veto_lep_i);
            if (abs(lep_pdgID) == 11 && passesTightElecID(lep_i))
            {
                // Count tight electrons
                n_tight_leps++;
                tight_lep_idx = veto_lep_i;
            }
            else if (abs(lep_pdgID) == 13 && passesTightMuonID(lep_i))
            {
                // Count tight muons
                n_tight_leps++;
                tight_lep_idx = veto_lep_i;
            }
        }

        // Require 1 and only 1 lepton (no additional >= veto leptons)
        if (n_tight_leps != 1 || veto_lep_idxs.size() != 1) { return false; }

        LorentzVector lep_p4 = veto_lep_p4s.at(tight_lep_idx);
        int lep_pdgID = veto_lep_pdgIDs.at(tight_lep_idx);
        globals.setVal<LorentzVector>("lep_p4", lep_p4);

        if (!nt.isData() && lep_sfs != nullptr) 
        { 
            applySFs(abs(lep_pdgID), lep_p4.pt(), lep_p4.eta()); 
        }
        else
        {
            arbol.setLeaf<double>("lep_id_sf", 1.);
            arbol.setLeaf<double>("lep_id_sf_up", 1.);
            arbol.setLeaf<double>("lep_id_sf_dn", 1.);
            arbol.setLeaf<double>("elec_reco_sf", 1.);
            arbol.setLeaf<double>("elec_reco_sf_up", 1.);
            arbol.setLeaf<double>("elec_reco_sf_dn", 1.);
            arbol.setLeaf<double>("muon_iso_sf", 1.);
            arbol.setLeaf<double>("muon_iso_sf_up", 1.);
            arbol.setLeaf<double>("muon_iso_sf_dn", 1.);
        }

        arbol.setLeaf<int>("lep_pdgID", lep_pdgID);
        arbol.setLeaf<double>("lep_pt", lep_p4.pt());
        arbol.setLeaf<double>("lep_eta", lep_p4.eta());
        arbol.setLeaf<double>("lep_phi", lep_p4.phi());

        return true;
    };

    double weight()
    {
        return arbol.getLeaf<double>("lep_id_sf");
    };
};

class Has1LepPKU : public Has1Lep
{
public:
    LeptonSFsPKU* lep_sfs;

    Has1LepPKU(std::string name, Core::Analysis& analysis, LeptonSFsPKU* lep_sfs = nullptr) 
    : Has1Lep(name, analysis, lep_sfs) 
    {
        this->lep_sfs = lep_sfs;
    };

    bool passesTightElecID(int elec_i)
    {
        return PKU::passesElecID(elec_i, PKU::IDtight);
    };

    bool passesTightMuonID(int muon_i)
    {
        return PKU::passesMuonID(muon_i, PKU::IDtight);
    };

    void applySFs(unsigned int abs_lep_pdgID, double lep_pt, double lep_eta)
    {
        double lep_id_sf = 1.;
        double lep_id_sf_up = 1.;
        double lep_id_sf_dn = 1.;
        double muon_iso_sf = 1.;
        double muon_iso_sf_up = 1.;
        double muon_iso_sf_dn = 1.;
        double elec_reco_sf = 1.;
        double elec_reco_sf_up = 1.;
        double elec_reco_sf_dn = 1.;
        if (abs_lep_pdgID == 11)
        {
            lep_id_sf = lep_sfs->getElecSF(lep_pt, lep_eta);
            lep_id_sf_up = lep_sfs->getElecErrUp(lep_pt, lep_eta);
            lep_id_sf_dn = lep_sfs->getElecErrDn(lep_pt, lep_eta);
            elec_reco_sf = lep_sfs->getElecRecoSF(lep_pt, lep_eta);
            elec_reco_sf_up = lep_sfs->getElecRecoErrUp(lep_pt, lep_eta);
            elec_reco_sf_dn = lep_sfs->getElecRecoErrDn(lep_pt, lep_eta);
        }
        else if (abs_lep_pdgID == 13)
        {
            lep_id_sf = lep_sfs->getMuonSF(lep_pt, lep_eta);
            lep_id_sf_up = lep_sfs->getMuonErrUp(lep_pt, lep_eta);
            lep_id_sf_dn = lep_sfs->getMuonErrDn(lep_pt, lep_eta);
            muon_iso_sf = lep_sfs->getMuonIsoSF(lep_pt, lep_eta);
            muon_iso_sf_up = lep_sfs->getMuonIsoErrUp(lep_pt, lep_eta);
            muon_iso_sf_dn = lep_sfs->getMuonIsoErrDn(lep_pt, lep_eta);
        }
        arbol.setLeaf<double>("lep_id_sf", lep_id_sf);
        arbol.setLeaf<double>("lep_id_sf_up", lep_id_sf_up);
        arbol.setLeaf<double>("lep_id_sf_dn", lep_id_sf_dn);
        arbol.setLeaf<double>("muon_iso_sf", muon_iso_sf);
        arbol.setLeaf<double>("muon_iso_sf_up", muon_iso_sf_up);
        arbol.setLeaf<double>("muon_iso_sf_dn", muon_iso_sf_dn);
        arbol.setLeaf<double>("elec_reco_sf", elec_reco_sf);
        arbol.setLeaf<double>("elec_reco_sf_up", elec_reco_sf_up);
        arbol.setLeaf<double>("elec_reco_sf_dn", elec_reco_sf_dn);
    };

    double weight()
    {
        return (
            arbol.getLeaf<double>("lep_id_sf")
            *arbol.getLeaf<double>("muon_iso_sf")
            *arbol.getLeaf<double>("elec_reco_sf")
        );
    };
};

class SaveVariables : public Core::AnalysisCut
{
public:
    ParticleNetXbbSFs* xbb_sfs;

    SaveVariables(std::string name, Core::Analysis& analysis, ParticleNetXbbSFs* xbb_sfs = nullptr) 
    : AnalysisCut(name, analysis) 
    {
        this->xbb_sfs = xbb_sfs;
    };

    bool evaluate()
    {
        arbol.setLeaf<bool>("passes_bveto", arbol.getLeaf<int>("n_medium_b_jets") == 0);
        arbol.setLeaf<double>("LT", arbol.getLeaf<double>("lep_pt") + arbol.getLeaf<double>("MET"));
        arbol.setLeaf<double>("ST", arbol.getLeaf<double>("LT") + arbol.getLeaf<double>("hbbjet_pt"));
        arbol.setLeaf<double>("LT_up", arbol.getLeaf<double>("lep_pt") + arbol.getLeaf<double>("MET_up"));
        arbol.setLeaf<double>("ST_up", arbol.getLeaf<double>("LT_up") + arbol.getLeaf<double>("hbbjet_pt"));
        arbol.setLeaf<double>("LT_dn", arbol.getLeaf<double>("lep_pt") + arbol.getLeaf<double>("MET_dn"));
        arbol.setLeaf<double>("ST_dn", arbol.getLeaf<double>("LT_dn") + arbol.getLeaf<double>("hbbjet_pt"));

        if (cli.is_signal && xbb_sfs != nullptr && arbol.getLeaf<double>("hbbjet_score") > 0.9)
        {
            double hbbjet_pt = arbol.getLeaf<double>("hbbjet_pt");
            arbol.setLeaf<double>("xbb_sf", xbb_sfs->getSF(hbbjet_pt));
            arbol.setLeaf<double>("xbb_sf_up", xbb_sfs->getSFUp(hbbjet_pt));
            arbol.setLeaf<double>("xbb_sf_dn", xbb_sfs->getSFDn(hbbjet_pt));
        }
        else
        {
            arbol.setLeaf<double>("xbb_sf", 1.);
            arbol.setLeaf<double>("xbb_sf_up", 1.);
            arbol.setLeaf<double>("xbb_sf_dn", 1.);
        }

        if (!nt.isData() && nt.nLHEPdfWeight() == 103) // PDF-dependent; this is fine for VBSWH signal
        {
            arbol.setLeaf<double>("alphaS_up", nt.LHEPdfWeight().at(101));
            arbol.setLeaf<double>("alphaS_dn", nt.LHEPdfWeight().at(102));
        }
        else
        {
            arbol.setLeaf<double>("alphaS_up", 1.);
            arbol.setLeaf<double>("alphaS_dn", 1.);
        }

        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        if (file_name.Contains("kWscan_kZscan") || file_name.Contains("kWkZscan"))
        {
            Doubles reweights;
            for (auto reweight : nt.LHEReweightingWeight())
            {
                reweights.push_back(reweight);
            }
            arbol.setLeaf<Doubles>("reweights", reweights);
        }
        return true;
    };
};

class FixEWKSamples : public Core::AnalysisCut
{
public:
    SFHist* ewk_fix;

    FixEWKSamples(std::string name, Core::Analysis& analysis) 
    : Core::AnalysisCut(name, analysis) 
    {
        ewk_fix = new SFHist("data/ewk_fix.root", "Wgt__pdgid5_quarks_pt_varbin");
    };

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
    };

    double getChargeQQ(int q1_pdgID, int q2_pdgID)
    {
        int q1_sign = (q1_pdgID > 0) - (q1_pdgID < 0);
        int q2_sign = (q2_pdgID > 0) - (q2_pdgID < 0);
        return (q1_sign*getChargeQx3(q1_pdgID) + q2_sign*getChargeQx3(q2_pdgID)) / 3.;
    };

    bool evaluate()
    {
        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        if (file_name.Contains("EWKWPlus") || file_name.Contains("EWKWMinus"))
        {
            int q1_pdgID = nt.LHEPart_pdgId().at(4);
            int q2_pdgID = nt.LHEPart_pdgId().at(5);
            LorentzVector q1_p4 = nt.LHEPart_p4().at(4);
            LorentzVector q2_p4 = nt.LHEPart_p4().at(5);
            double M_qq = (q1_p4 + q2_p4).M();
            LorentzVector lep_p4 = nt.LHEPart_p4().at(2);
            LorentzVector nu_p4 = nt.LHEPart_p4().at(3);
            double M_lnu = (lep_p4 + nu_p4).M();
            bool is_WW = (
                fabs(getChargeQQ(q1_pdgID, q2_pdgID)) == 1 
                && M_qq >= 70 && M_qq < 90 
                && M_lnu >= 70 && M_lnu < 90
            );
            if (is_WW)
            {
                // WW event
                arbol.setLeaf<double>("ewkfix_sf", 0.);
                return true;
            }
            else if (M_qq >= 95)
            {
                // VBS W event
                double bquark_pt = -999;
                if (abs(q1_pdgID) == 5 && abs(q2_pdgID) == 5)
                {
                    bquark_pt = (q1_p4.pt() > q2_p4.pt()) ? q1_p4.pt() : q2_p4.pt();
                }
                else if (abs(q1_pdgID) == 5)
                {
                    bquark_pt = q1_p4.pt();
                }
                else if (abs(q2_pdgID) == 5)
                {
                    bquark_pt = q2_p4.pt();
                }
                if (bquark_pt != -999)
                {
                    arbol.setLeaf<double>("ewkfix_sf", ewk_fix->getSF(bquark_pt));
                    return true;
                }
            }
        }
        arbol.setLeaf<double>("ewkfix_sf", 1.);
        return true;
    };

    double weight()
    {
        return arbol.getLeaf<double>("ewkfix_sf");
    };
};

} // End namespace VBSWH;

#endif
