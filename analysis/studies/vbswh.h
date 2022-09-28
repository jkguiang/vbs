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
    Passes1LepTriggers(std::string name, Core::Analysis& analysis) : Core::AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool passesMuonTriggers()
    {
        bool passed = false;
        switch (nt.year())
        {
        case (2016):
            try { passed = (passed || nt.HLT_IsoMu24()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_IsoTkMu24()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        case (2017):
            try { passed = (passed || nt.HLT_IsoMu27()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        case (2018):
            try { passed = (passed || nt.HLT_IsoMu24()); }
            catch (const runtime_error& error) { /* do nothing */ }
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
            catch (const runtime_error& error) { /* do nothing */ }
            passed = (passed || passesMuonTriggers());
            break;
        case (2017):
            try { passed = (passed || nt.HLT_Ele32_WPTight_Gsf_L1DoubleEG()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        case (2018):
            try { passed = (passed || nt.HLT_Ele32_WPTight_Gsf()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        }
        return passed;
    };

    bool passesLepTriggers(unsigned int abs_lep_pdg_id)
    {
        if (!nt.isData()) 
        { 
            switch (abs_lep_pdg_id)
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
        return passesLepTriggers(abs(arbol.getLeaf<int>("lep_pdgID")));
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
        arbol.setLeaf<double>("ST", (arbol.getLeaf<double>("LT") + best_hbbjet_p4.pt()));

        return true;
    };
};

class SelectJetsNoHbbOverlap : public Core::SelectJets
{
public:
    LorentzVector hbbjet_p4;

    SelectJetsNoHbbOverlap(std::string name, Core::Analysis& analysis, JetEnergyScales* jes, BTagSFs* btag_sfs) 
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
        return overlapsLepton(jet_i, jet_p4) or overlapsHbbJet(jet_p4);
    };
};

class Has1Lep : public Core::AnalysisCut
{
public:
    Has1Lep(std::string name, Core::Analysis& analysis) : Core::AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    virtual bool passesVetoElecID(int elec_i)
    {
        return ttH_UL::electronID(elec_i, ttH::IDfakable, nt.year());
    };

    virtual bool passesTightElecID(int elec_i)
    {
        return ttH_UL::electronID(elec_i, ttH::IDtight, nt.year());
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return ttH_UL::muonID(muon_i, ttH::IDfakable, nt.year());
    };

    virtual bool passesTightMuonID(int muon_i)
    {
        return ttH_UL::muonID(muon_i, ttH::IDtight, nt.year());
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
            // Count loose and tight leptons
            unsigned int lep_i = veto_lep_idxs.at(veto_lep_i);
            int lep_pdgID = veto_lep_pdgIDs.at(veto_lep_i);
            if (abs(lep_pdgID) == 11)
            {
                if (passesTightElecID(lep_i)) 
                {
                    n_tight_leps++;
                    tight_lep_idx = veto_lep_i;
                }
            }
            else if (abs(lep_pdgID) == 13)
            {
                if (passesTightMuonID(lep_i)) 
                {
                    n_tight_leps++;
                    tight_lep_idx = veto_lep_i;
                }
            }
        }
        // Require 1 and only 1 lepton (no additional >= veto leptons)
        if (n_tight_leps != 1 || veto_lep_idxs.size() != 1) { return false; }
        LorentzVector lep_p4 = veto_lep_p4s.at(tight_lep_idx);
        globals.setVal<LorentzVector>("lep_p4", lep_p4);

        arbol.setLeaf<int>("lep_pdgID", veto_lep_pdgIDs.at(tight_lep_idx));
        arbol.setLeaf<double>("lep_pt", lep_p4.pt());
        arbol.setLeaf<double>("lep_eta", lep_p4.eta());
        arbol.setLeaf<double>("lep_phi", lep_p4.phi());
        arbol.setLeaf<double>("LT", (lep_p4.pt() + arbol.getLeaf<double>("MET")));

        return true;
    };

    double weight()
    {
        return arbol.getLeaf<double>("lep_sf");
    };
};

class Has1LepPKU : public Has1Lep
{
public:
    Has1LepPKU(std::string name, Core::Analysis& analysis) : Has1Lep(name, analysis) 
    {
        // Do nothing
    };

    bool passesVetoElecID(int elec_i)
    {
        return PKU::passesElecID(elec_i, PKU::IDveto);
    };

    bool passesTightElecID(int elec_i)
    {
        return PKU::passesElecID(elec_i, PKU::IDtight);
    };

    bool passesVetoMuonID(int muon_i)
    {
        return PKU::passesMuonID(muon_i, PKU::IDveto);
    };

    bool passesTightMuonID(int muon_i)
    {
        return PKU::passesMuonID(muon_i, PKU::IDtight);
    };
};

class SaveBJetVeto : public Core::AnalysisCut
{
public:
    SaveBJetVeto(std::string name, Core::Analysis& analysis) : AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        bool passes_bveto = true;
        for (auto& btag : globals.getVal<Doubles>("good_jet_btags"))
        {
            if (btag > gconf.WP_DeepFlav_medium) 
            { 
                passes_bveto = false;
                break;
            }
        }
        arbol.setLeaf<bool>("passes_bveto", passes_bveto);
        return true;
    };
};

struct Analysis : Core::Analysis
{
    JetEnergyScales* jes;
    LeptonSFsPKU* lep_sfs;
    BTagSFs* btag_sfs;
    // LeptonSFsTTH* lep_sfs;

    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Analysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Lepton globals
        cutflow.globals.newVar<LorentzVector>("lep_p4");
        // Hbb jet globals
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
    };

    virtual void initBranches()
    {
        Core::Analysis::initBranches();
        // Lepton branches
        arbol.newBranch<double>("lep_sf", -999);
        arbol.newBranch<double>("lep_sf_up", -999);
        arbol.newBranch<double>("lep_sf_dn", -999);
        arbol.newBranch<int>("lep_pdgID", -999);
        arbol.newBranch<double>("lep_pt", -999);
        arbol.newBranch<double>("lep_eta", -999);
        arbol.newBranch<double>("lep_phi", -999);
        arbol.newBranch<double>("LT", -999);
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
        arbol.newBranch<bool>("passes_bveto", false);
    };

    virtual void initCutflow()
    {
        // Initialize scale factors
        jes = new JetEnergyScales(cli.variation);
        lep_sfs = new LeptonSFsPKU();
        // lep_sfs = new LeptonSFsTTH();
        btag_sfs = new BTagSFs();

        // Bookkeeping
        Cut* bookkeeping = new Core::Bookkeeping("Bookkeeping", *this);
        cutflow.setRoot(bookkeeping);

        // Event filters
        Cut* event_filters = new PassesEventFilters("PassesEventFilters", *this);
        cutflow.insert(bookkeeping, event_filters, Right);

        // Lepton selection
        Cut* select_leps = new Core::SelectLeptonsPKU("SelectLeptons", *this, lep_sfs);
        // Cut* select_leps = new Core::SelectLeptons("SelectLeptons", *this, lep_sfs);
        cutflow.insert(event_filters, select_leps, Right);

        // == 1 lepton selection
        Cut* has_1lep = new Has1LepPKU("Has1TightLep", *this);
        // Cut* has_1lep = new Has1Lep("Has1TightLep", *this);
        cutflow.insert(select_leps, has_1lep, Right);

        // Lepton has pT > 40
        Cut* lep_pt_gt40 = new LambdaCut(
            "LepPtGt40", [&]() { return arbol.getLeaf<double>("lep_pt") >= 40; }
        );
        cutflow.insert(has_1lep, lep_pt_gt40, Right);

        // Single-lepton triggers
        Cut* lep_triggers = new Passes1LepTriggers("Passes1LepTriggers", *this);
        cutflow.insert(lep_pt_gt40, lep_triggers, Right);

        // Fat jet selection
        Cut* select_fatjets = new Core::SelectFatJets("SelectFatJets", *this);
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

        // Global AK4 b-veto
        Cut* save_ak4bveto = new SaveBJetVeto("SaveAk4GlobalBVeto", *this);
        cutflow.insert(select_jets, save_ak4bveto, Right);

        // VBS jet selection
        Cut* select_vbsjets_maxE = new Core::SelectVBSJetsMaxE("SelectVBSJetsMaxE", *this);
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

    void init()
    {
        Core::Analysis::init();
        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        jes->init();
        lep_sfs->init(file_name);
    };
};

} // End namespace VBSWH;

#endif
