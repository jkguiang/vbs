#ifndef VBSWH_H
#define VBSWH_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// VBS WH
#include "core.h"

// DEBUG
#include "MCTools.h"

namespace VBSWH
{

struct Analysis : Core::VBSAnalysis
{
    Analysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::VBSAnalysis(arbol_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Lepton globals
        cutflow.globals.newVar<LorentzVector>("lep_p4");
        // Hbb jet globals
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
    };

    virtual void initBranches()
    {
        Core::VBSAnalysis::initBranches();
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

        init_branches = true;
    };
};

class Passes1LepTriggers : public Core::DressedCut
{
public:
    Passes1LepTriggers(std::string name, Analysis& analysis) : Core::DressedCut(name, analysis) 
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
        if (!cli.is_data) 
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

class SelectHbbFatJet : public Core::DressedCut
{
public:
    bool use_md;

    SelectHbbFatJet(std::string name, Analysis& analysis, bool md = false) 
    : Core::DressedCut(name, analysis) 
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
        if (!cli.is_data)
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
        /* Uncomment this block for debugging
        if (n_hbbjet_genbquarks > 1)
        {
            std::cout << "------------------ START ------------------" << std::endl;
            dumpGenParticleInfos({});
            std::cout << "\n FOUND " << n_hbbjet_genbquarks << " MATCHES" << std::endl;
            std::cout << "------------------ END ------------------" << std::endl;
        }
        */

        // Store the fatjet
        globals.setVal<LorentzVector>("hbbjet_p4", best_hbbjet_p4);

        if (!init_branches) { return true; }

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

    SelectJetsNoHbbOverlap(std::string name, Analysis& analysis) : Core::SelectJets(name, analysis) 
    {
        // Do nothing
    };

    void loadOverlapVars()
    {
        good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        good_lep_jet_idxs = globals.getVal<Integers>("good_lep_jet_idxs");
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

class Has1Lep : public Core::DressedCut
{
public:
    Has1Lep(std::string name, Analysis& analysis) : Core::DressedCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        LorentzVectors good_lep_p4s = globals.getVal<LorentzVectors>("good_lep_p4s");
        Integers good_lep_pdgIDs = globals.getVal<Integers>("good_lep_pdgIDs");
        Integers good_lep_idxs = globals.getVal<Integers>("good_lep_idxs");
        int n_loose_leps = 0;
        int n_tight_leps = 0;
        int tight_lep_idx = -999;
        for (unsigned int good_lep_i = 0; good_lep_i < good_lep_p4s.size(); ++good_lep_i)
        {
            // Count loose and tight leptons
            unsigned int lep_i = good_lep_idxs.at(good_lep_i);
            int lep_pdgID = good_lep_pdgIDs.at(good_lep_i);
            if (abs(lep_pdgID) == 11)
            {
                if (ttH_UL::electronID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH_UL::electronID(lep_i, ttH::IDtight, nt.year())) 
                {
                    n_tight_leps++;
                    tight_lep_idx = good_lep_i;
                }
            }
            else if (abs(lep_pdgID) == 13)
            {
                if (ttH_UL::muonID(lep_i, ttH::IDfakable, nt.year())) { n_loose_leps++; }
                if (ttH_UL::muonID(lep_i, ttH::IDtight, nt.year())) 
                {
                    n_tight_leps++;
                    tight_lep_idx = good_lep_i;
                }
            }
        }
        // Require 1 and only 1 lepton (all passing tight ID)
        if (n_tight_leps != 1 || n_loose_leps != 1) { return false; }
        LorentzVector lep_p4 = good_lep_p4s.at(tight_lep_idx);
        globals.setVal<LorentzVector>("lep_p4", lep_p4);

        if (!init_branches) { return true; }

        arbol.setLeaf<int>("lep_pdgID", good_lep_pdgIDs.at(tight_lep_idx));
        arbol.setLeaf<double>("lep_pt", lep_p4.pt());
        arbol.setLeaf<double>("lep_eta", lep_p4.eta());
        arbol.setLeaf<double>("lep_phi", lep_p4.phi());
        arbol.setLeaf<double>("LT", (lep_p4.pt() + arbol.getLeaf<double>("MET")));

        return true;
    };

    double weight()
    {
        if (init_branches)
        {
            return arbol.getLeaf<double>("lep_sf");
        }
        else
        {
            return 1.;
        }
    };
};

} // End namespace VBSWH;

#endif
