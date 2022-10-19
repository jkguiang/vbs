#ifndef VBSWH_H
#define VBSWH_H

// RAPIDO
#include "arbol.h"
#include "cutflow.h"
#include "utilities.h"
// VBS
#include "core.h"

namespace VBSWH
{

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
        );
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
            passed = (passed || passesMuonTriggers());
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
        return passed;
    };

    bool passesLepTriggers(unsigned int abs_lep_pdgID)
    {
        if (!nt.isData()) 
        { 
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
                return passesElecTriggers();
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
            double hlt_sf = 1.;
            switch (abs_lep_pdgID)
            {
            case (11):
                hlt_sf = hlt_sfs->getElecSF(pt, eta);
                arbol.setLeaf<double>("trig_sf", hlt_sf);
                arbol.setLeaf<double>("trig_sf_up", hlt_sf + hlt_sfs->getElecErrUp(pt, eta));
                arbol.setLeaf<double>("trig_sf_dn", hlt_sf - hlt_sfs->getElecErrDn(pt, eta));
                break;
            case (13):
                hlt_sf = hlt_sfs->getMuonSF(pt, eta);
                arbol.setLeaf<double>("trig_sf", hlt_sf);
                arbol.setLeaf<double>("trig_sf_up", hlt_sf + hlt_sfs->getMuonErrUp(pt, eta));
                arbol.setLeaf<double>("trig_sf_dn", hlt_sf - hlt_sfs->getMuonErrDn(pt, eta));
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
    SaveVariables(std::string name, Core::Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
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
        return true;
    };
};

struct Analysis : Core::Analysis
{
    JetEnergyScales* jes;
    LeptonSFsPKU* lep_sfs;
    HLT1LepSFs* hlt_sfs;
    BTagSFs* btag_sfs;
    PileUpSFs* pu_sfs;
    bool all_corrections;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Lepton globals
        cutflow.globals.newVar<LorentzVector>("lep_p4");
        // Hbb jet globals
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
        // Scale factors
        jes = nullptr;
        lep_sfs = nullptr;
        hlt_sfs = nullptr;
        btag_sfs = nullptr;
        pu_sfs = nullptr;
        all_corrections = false;
    };

    virtual void initBranches()
    {
        Core::Analysis::initBranches();
        // Lepton branches
        arbol.newBranch<double>("lep_id_sf", -999);
        arbol.newBranch<double>("lep_id_sf_up", -999);
        arbol.newBranch<double>("lep_id_sf_dn", -999);
        arbol.newBranch<double>("elec_reco_sf", -999);
        arbol.newBranch<double>("elec_reco_sf_up", -999);
        arbol.newBranch<double>("elec_reco_sf_dn", -999);
        arbol.newBranch<double>("muon_iso_sf", -999);
        arbol.newBranch<double>("muon_iso_sf_up", -999);
        arbol.newBranch<double>("muon_iso_sf_dn", -999);
        arbol.newBranch<int>("lep_pdgID", -999);
        arbol.newBranch<double>("lep_pt", -999);
        arbol.newBranch<double>("lep_eta", -999);
        arbol.newBranch<double>("lep_phi", -999);
        arbol.newBranch<double>("LT", -999);
        arbol.newBranch<double>("LT_up", -999);
        arbol.newBranch<double>("LT_dn", -999);
        // Hbb jet branches
        arbol.newBranch<int>("n_hbbjet_genbquarks", -999);
        arbol.newBranch<double>("hbbjet_score", -999);
        arbol.newBranch<double>("hbbjet_pt", -999);
        arbol.newBranch<double>("hbbjet_eta", -999);
        arbol.newBranch<double>("hbbjet_phi", -999);
        arbol.newBranch<double>("hbbjet_mass", -999);
        arbol.newBranch<double>("hbbjet_msoftdrop", -999);
        // Other branches
        arbol.newBranch<double>("ST", -999);
        arbol.newBranch<double>("ST_up", -999);
        arbol.newBranch<double>("ST_dn", -999);
        arbol.newBranch<bool>("passes_bveto", false);
        arbol.newBranch<double>("trig_sf", -999);
        arbol.newBranch<double>("trig_sf_up", -999);
        arbol.newBranch<double>("trig_sf_dn", -999);
        arbol.newBranch<double>("alphaS_up", -999);
        arbol.newBranch<double>("alphaS_dn", -999);
    };

    virtual void initCorrections()
    {
        jes = new JetEnergyScales(cli.variation);
        lep_sfs = new LeptonSFsPKU(PKU::IDtight);
        hlt_sfs = new HLT1LepSFs();
        btag_sfs = new BTagSFs(cli.output_name, "M");
        pu_sfs = new PileUpSFs();
        all_corrections = true;
    };

    virtual void initCutflow()
    {
        // Bookkeeping
        Cut* bookkeeping = new Core::Bookkeeping("Bookkeeping", *this, pu_sfs);
        cutflow.setRoot(bookkeeping);

        // Event filters
        Cut* event_filters = new PassesEventFilters("PassesEventFilters", *this);
        cutflow.insert(bookkeeping, event_filters, Right);

        // Lepton selection
        Cut* select_leps = new Core::SelectLeptonsPKU("SelectLeptons", *this);
        cutflow.insert(event_filters, select_leps, Right);

        // == 1 lepton selection
        Cut* has_1lep = new Has1LepPKU("Has1TightLep", *this, lep_sfs);
        cutflow.insert(select_leps, has_1lep, Right);

        // Lepton has pT > 40
        Cut* lep_pt_gt40 = new LambdaCut(
            "LepPtGt40", [&]() { return arbol.getLeaf<double>("lep_pt") >= 40.; }
        );
        cutflow.insert(has_1lep, lep_pt_gt40, Right);

        // Single-lepton triggers
        Cut* lep_triggers = new Passes1LepTriggers("Passes1LepTriggers", *this, hlt_sfs);
        cutflow.insert(lep_pt_gt40, lep_triggers, Right);

        // Fat jet selection
        Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", *this, jes);
        cutflow.insert(lep_triggers, select_fatjets, Right);

        // Geq1FatJet
        Cut* geq1fatjet = new LambdaCut(
            "Geq1FatJet", [&]() { return arbol.getLeaf<int>("n_fatjets") >= 1; }
        );
        cutflow.insert(select_fatjets, geq1fatjet, Right);

        // Hbb selection
        Cut* select_hbbjet = new SelectHbbFatJet("SelectHbbFatJet", *this, true);
        cutflow.insert(geq1fatjet, select_hbbjet, Right);

        // Jet selection
        Cut* select_jets = new SelectJetsNoHbbOverlap("SelectJetsNoHbbOverlap", *this, jes, btag_sfs);
        cutflow.insert(select_hbbjet, select_jets, Right);

        // VBS jet selection
        Cut* select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SelectVBSJetsMaxE", *this);
        cutflow.insert(select_jets, select_vbsjets_maxE, Right);

        // Save LHE mu_R and mu_F scale weights
        Cut* save_lhe = new Core::SaveSystWeights("SaveSystWeights", *this);
        cutflow.insert(select_vbsjets_maxE, save_lhe, Right);

        // Save analysis variables
        Cut* save_vars = new SaveVariables("SaveVariables", *this);
        cutflow.insert(save_lhe, save_vars, Right);

        // Basic VBS jet requirements
        Cut* vbsjets_presel = new LambdaCut(
            "MjjGt500_detajjGt3", 
            [&]()
            {
                return arbol.getLeaf<double>("M_jj") > 500 && fabs(arbol.getLeaf<double>("deta_jj")) > 3;
            }
        );
        cutflow.insert(save_vars, vbsjets_presel, Right);

        Cut* xbb_presel = new LambdaCut(
            "XbbGt0p3", [&]() { return arbol.getLeaf<double>("hbbjet_score") > 0.3; }
        );
        cutflow.insert(vbsjets_presel, xbb_presel, Right);

        Cut* apply_ak4bveto = new LambdaCut(
            "ApplyAk4GlobalBVeto", [&]() { return arbol.getLeaf<bool>("passes_bveto"); }
        );
        cutflow.insert(xbb_presel, apply_ak4bveto, Right);
        
        Cut* SR1_vbs_cuts = new LambdaCut(
            "MjjGt600_detajjGt4", 
            [&]() 
            { 
                return arbol.getLeaf<double>("M_jj") > 600 && fabs(arbol.getLeaf<double>("deta_jj")) > 4;
            }
        );
        cutflow.insert(apply_ak4bveto, SR1_vbs_cuts, Right);

        Cut* SR1_ST_cut = new LambdaCut(
            "STGt900", [&]() { return arbol.getLeaf<double>("ST") > 900; }
        );
        cutflow.insert(SR1_vbs_cuts, SR1_ST_cut, Right);

        Cut* SR1_hbb_cut = new LambdaCut(
            "XbbGt0p9_MSDLt150", 
            [&]() 
            { 
                return (
                    arbol.getLeaf<double>("hbbjet_score") > 0.9 
                    && arbol.getLeaf<double>("hbbjet_msoftdrop") < 150
                ); 
            }
        );
        cutflow.insert(SR1_ST_cut, SR1_hbb_cut, Right);

        Cut* SR2 = new LambdaCut(
            "STGt1500", [&]() { return arbol.getLeaf<double>("ST") > 1500; }
        );
        cutflow.insert(SR1_hbb_cut, SR2, Right);
    };

    virtual void init()
    {
        Core::Analysis::init();
        if (all_corrections)
        {
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
            jes->init();
            lep_sfs->init(file_name);
            hlt_sfs->init(file_name);
            btag_sfs->init(file_name);
            pu_sfs->init(file_name);
        }
    };
};

} // End namespace VBSWH;

#endif
