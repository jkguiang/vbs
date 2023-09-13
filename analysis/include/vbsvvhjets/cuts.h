#ifndef VBSVVHJETS_CUTS_H
#define VBSVVHJETS_CUTS_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// VBS
#include "core/collections.h"
#include "core/cuts.h"
#include "vbswh/cuts.h"
#include "vbsvvhjets/enums.h"
#include "corrections/all.h"

namespace VBSVVHJets
{

class FindLeptonsTTHNoUL : public VBSWH::FindLeptons
{
public:
    FindLeptonsTTHNoUL(std::string name, Core::Skimmer& skimmer) : FindLeptons(name, skimmer) 
    {
        // Do nothing
    };

    virtual bool passesVetoElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDveto, nt.year()); // same as ttH_UL
    };

    virtual bool passesTightElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDtight, nt.year());   // not the same as ttH_UL
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDveto, nt.year());     // same as ttH_UL
    };

    virtual bool passesTightMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDtight, nt.year());       // not the same as ttH_UL
    };
};

class PassesTriggers : public Core::AnalysisCut
{
public:
    PassesTriggers(std::string name, Core::Analysis& analysis) : Core::AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        bool passed = false;
        switch (nt.year())
        {
        case (2016):
            try { passed = (passed || nt.HLT_PFHT800()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_PFHT900()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFHT650_TrimR0p1PT0p03Mass50()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFHT700_TrimR0p1PT0p03Mass50()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet450()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet360_TrimMass30()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8DiPFJet280_200_TrimMass30()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p20()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        case (2017):
            try { passed = (passed || nt.HLT_PFHT1050()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFHT800_TrimMass50()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_PFJet320()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_PFJet500()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet320()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet500()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet400_TrimMass30()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet420_TrimMass30()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        case (2018):
            try { passed = (passed || nt.HLT_PFHT1050()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFHT800_TrimMass50()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_PFJet500()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet500()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet400_TrimMass30()); }
            catch (const runtime_error& error) { /* do nothing */ }
            try { passed = (passed || nt.HLT_AK8PFJet420_TrimMass30()); }
            catch (const runtime_error& error) { /* do nothing */ }
            break;
        }
        if (!nt.isData() && passed)
        {
            // TODO: set/implement HT HLT sfs
            arbol.setLeaf<double>("trig_sf", 1.);
            arbol.setLeaf<double>("trig_sf_up", 1.);
            arbol.setLeaf<double>("trig_sf_dn", 1.);
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

class SelectVVHFatJets : public Core::AnalysisCut
{
public:
    Channel channel;

    SelectVVHFatJets(std::string name, Core::Analysis& analysis, Channel channel) 
    : Core::AnalysisCut(name, analysis) 
    {
        this->channel = channel;
    };

    bool evaluate()
    {
        LorentzVectors good_fatjet_p4s = globals.getVal<LorentzVectors>("good_fatjet_p4s");
        Doubles good_fatjet_xbbtags = globals.getVal<Doubles>("good_fatjet_xbbtags");
        Doubles good_fatjet_xvqqtags = globals.getVal<Doubles>("good_fatjet_xvqqtags");
        Doubles good_fatjet_xwqqtags = globals.getVal<Doubles>("good_fatjet_xwqqtags");
        Doubles good_fatjet_msoftdrops = globals.getVal<Doubles>("good_fatjet_msoftdrops");
        Doubles good_fatjet_masses = globals.getVal<Doubles>("good_fatjet_masses");

        // Select Hbb fat jet candidate first
        unsigned int best_xbb_i = (
            std::max_element(good_fatjet_xbbtags.begin(), good_fatjet_xbbtags.end()) - good_fatjet_xbbtags.begin()
        );
        LorentzVector hbbfatjet_p4 = good_fatjet_p4s.at(best_xbb_i);
        globals.setVal<LorentzVector>("hbbfatjet_p4", hbbfatjet_p4);
        globals.setVal<unsigned int>("hbbfatjet_gidx", best_xbb_i);
        arbol.setLeaf<double>("hbbfatjet_xbb", good_fatjet_xbbtags.at(best_xbb_i));
        arbol.setLeaf<double>("hbbfatjet_pt", hbbfatjet_p4.pt());
        arbol.setLeaf<double>("hbbfatjet_eta", hbbfatjet_p4.eta());
        arbol.setLeaf<double>("hbbfatjet_phi", hbbfatjet_p4.phi());
        arbol.setLeaf<double>("hbbfatjet_mass", good_fatjet_masses.at(best_xbb_i));
        arbol.setLeaf<double>("hbbfatjet_msoftdrop", good_fatjet_msoftdrops.at(best_xbb_i));

        // Get the two leading fatjets in pT
        int ld_fatjet_i = -999;
        int tr_fatjet_i = -999;
        double ld_fatjet_pt = -999.;
        double tr_fatjet_pt = -999.;
        for (unsigned int fatjet_i = 0; fatjet_i < good_fatjet_p4s.size(); ++fatjet_i)
        {
            if (fatjet_i == best_xbb_i)
            {
                continue;
            }
            double fatjet_pt = good_fatjet_p4s.at(fatjet_i).pt();
            if (fatjet_pt > ld_fatjet_pt)
            {
                if (ld_fatjet_pt != -999.)
                {
                    tr_fatjet_pt = ld_fatjet_pt;
                    tr_fatjet_i = ld_fatjet_i;
                }
                ld_fatjet_pt = fatjet_pt;
                ld_fatjet_i = fatjet_i;
            }
            else if (fatjet_pt > tr_fatjet_pt)
            {
                tr_fatjet_pt = fatjet_pt;
                tr_fatjet_i = fatjet_i;
            }
        }

        // Select W/Z candidate(s) last
        if (channel == AllMerged)
        {
            LorentzVector ld_vqqfatjet_p4 = good_fatjet_p4s.at(ld_fatjet_i);
            LorentzVector tr_vqqfatjet_p4 = good_fatjet_p4s.at(tr_fatjet_i);
            globals.setVal<LorentzVector>("ld_vqqfatjet_p4", ld_vqqfatjet_p4);
            globals.setVal<unsigned int>("ld_vqqfatjet_gidx", ld_fatjet_i);
            arbol.setLeaf<double>("ld_vqqfatjet_xvqq", good_fatjet_xvqqtags.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_xwqq", good_fatjet_xwqqtags.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_pt", ld_vqqfatjet_p4.pt());
            arbol.setLeaf<double>("ld_vqqfatjet_eta", ld_vqqfatjet_p4.eta());
            arbol.setLeaf<double>("ld_vqqfatjet_phi", ld_vqqfatjet_p4.phi());
            arbol.setLeaf<double>("ld_vqqfatjet_mass", good_fatjet_masses.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_msoftdrop", good_fatjet_msoftdrops.at(ld_fatjet_i));
            globals.setVal<LorentzVector>("tr_vqqfatjet_p4", tr_vqqfatjet_p4);
            globals.setVal<unsigned int>("tr_vqqfatjet_gidx", tr_fatjet_i);
            arbol.setLeaf<double>("tr_vqqfatjet_xvqq", good_fatjet_xvqqtags.at(tr_fatjet_i));
            arbol.setLeaf<double>("tr_vqqfatjet_xwqq", good_fatjet_xwqqtags.at(tr_fatjet_i));
            arbol.setLeaf<double>("tr_vqqfatjet_pt", tr_vqqfatjet_p4.pt());
            arbol.setLeaf<double>("tr_vqqfatjet_eta", tr_vqqfatjet_p4.eta());
            arbol.setLeaf<double>("tr_vqqfatjet_phi", tr_vqqfatjet_p4.phi());
            arbol.setLeaf<double>("tr_vqqfatjet_mass", good_fatjet_masses.at(tr_fatjet_i));
            arbol.setLeaf<double>("tr_vqqfatjet_msoftdrop", good_fatjet_msoftdrops.at(tr_fatjet_i));
            arbol.setLeaf<double>("M_VVH", (hbbfatjet_p4 + ld_vqqfatjet_p4 + tr_vqqfatjet_p4).M());
            arbol.setLeaf<double>("VVH_pt", (hbbfatjet_p4 + ld_vqqfatjet_p4 + tr_vqqfatjet_p4).pt());
            arbol.setLeaf<double>("VVH_eta", (hbbfatjet_p4 + ld_vqqfatjet_p4 + tr_vqqfatjet_p4).eta());
            arbol.setLeaf<double>("VVH_phi", (hbbfatjet_p4 + ld_vqqfatjet_p4 + tr_vqqfatjet_p4).phi());
        }
        else if (channel == SemiMerged)
        {
            LorentzVector vqqfatjet_p4 = good_fatjet_p4s.at(ld_fatjet_i);
            globals.setVal<LorentzVector>("ld_vqqfatjet_p4", vqqfatjet_p4);
            globals.setVal<unsigned int>("ld_vqqfatjet_gidx", ld_fatjet_i);
            arbol.setLeaf<double>("ld_vqqfatjet_xvqq", good_fatjet_xvqqtags.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_xwqq", good_fatjet_xwqqtags.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_pt", vqqfatjet_p4.pt());
            arbol.setLeaf<double>("ld_vqqfatjet_eta", vqqfatjet_p4.eta());
            arbol.setLeaf<double>("ld_vqqfatjet_phi", vqqfatjet_p4.phi());
            arbol.setLeaf<double>("ld_vqqfatjet_mass", good_fatjet_masses.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_msoftdrop", good_fatjet_msoftdrops.at(ld_fatjet_i));
        }
        return true;
    };
};

class SelectJetsNoFatJetOverlap : public Core::SelectJets
{
public:
    Channel channel;
    LorentzVector hbbfatjet_p4;
    LorentzVector ld_vqqfatjet_p4;
    LorentzVector tr_vqqfatjet_p4;

    SelectJetsNoFatJetOverlap(std::string name, Core::Analysis& analysis, Channel channel, 
                              JetEnergyScales* jes = nullptr, BTagSFs* btag_sfs = nullptr,
                              PileUpJetIDSFs* puid_sfs = nullptr) 
    : Core::SelectJets(name, analysis, jes, btag_sfs, puid_sfs) 
    {
        this->channel = channel;
    };

    void loadOverlapVars()
    {
        hbbfatjet_p4 = globals.getVal<LorentzVector>("hbbfatjet_p4");
        ld_vqqfatjet_p4 = globals.getVal<LorentzVector>("ld_vqqfatjet_p4");
        tr_vqqfatjet_p4 = globals.getVal<LorentzVector>("tr_vqqfatjet_p4");
    };

    bool isOverlap(int jet_i, LorentzVector jet_p4)
    {
        bool hbb_overlap = ROOT::Math::VectorUtil::DeltaR(hbbfatjet_p4, jet_p4) < 0.8;
        bool vqq_overlap = ROOT::Math::VectorUtil::DeltaR(ld_vqqfatjet_p4, jet_p4) < 0.8;
        if (channel == AllMerged)
        {
            vqq_overlap = vqq_overlap || ROOT::Math::VectorUtil::DeltaR(tr_vqqfatjet_p4, jet_p4) < 0.8;
        }
        return hbb_overlap || vqq_overlap;
    };
};

class SelectVJets : public Core::AnalysisCut
{
public:
    SelectVJets(std::string name, Core::Analysis& analysis) 
    : Core::AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
        int ld_vbsjet_idx = globals.getVal<int>("ld_vbsjet_idx");
        int tr_vbsjet_idx = globals.getVal<int>("tr_vbsjet_idx");

        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        Integers good_jet_idxs = globals.getVal<Integers>("good_jet_idxs");
        if (good_jet_idxs.size() < 4) { return false; }

        double min_dR = 99999;
        std::pair<unsigned int, unsigned int> vqqjet_idxs;
        for (unsigned int jet_i = 0; jet_i < good_jet_p4s.size(); ++jet_i)
        {
            int jet_idx = good_jet_idxs.at(jet_i);
            // Skip VBS jet candidates
            if (jet_idx == ld_vbsjet_idx || jet_idx == tr_vbsjet_idx) { continue; }
            // Iterate over all pairs
            for (unsigned int jet_j = jet_i + 1; jet_j < good_jet_p4s.size(); ++jet_j)
            {
                LorentzVector jet1_p4 = good_jet_p4s.at(jet_i);
                LorentzVector jet2_p4 = good_jet_p4s.at(jet_j);
                double dR = ROOT::Math::VectorUtil::DeltaR(jet1_p4, jet2_p4);
                if (dR < min_dR)
                {
                    min_dR = dR;
                    vqqjet_idxs = std::make_pair(jet_i, jet_j);
                }
            }
        }

        // Sort the two VBS jets into leading/trailing
        int ld_vqqjet_idx;
        int tr_vqqjet_idx;
        if (good_jet_p4s.at(vqqjet_idxs.first).pt() > good_jet_p4s.at(vqqjet_idxs.first).pt())
        {
            ld_vqqjet_idx = vqqjet_idxs.first;
            tr_vqqjet_idx = vqqjet_idxs.second;
        }
        else
        {
            ld_vqqjet_idx = vqqjet_idxs.second;
            tr_vqqjet_idx = vqqjet_idxs.first;
        }
        LorentzVector ld_vqqjet_p4 = good_jet_p4s.at(ld_vqqjet_idx);
        LorentzVector tr_vqqjet_p4 = good_jet_p4s.at(tr_vqqjet_idx);
        int ld_vqqjet_nanoidx = good_jet_idxs.at(ld_vqqjet_idx);
        int tr_vqqjet_nanoidx = good_jet_idxs.at(tr_vqqjet_idx);

        globals.setVal<LorentzVector>("ld_vqqjet_p4", ld_vqqjet_p4);
        globals.setVal<LorentzVector>("tr_vqqjet_p4", tr_vqqjet_p4);
        arbol.setLeaf<double>("ld_vqqjet_qgl", nt.Jet_qgl().at(ld_vqqjet_nanoidx));
        arbol.setLeaf<double>("ld_vqqjet_pt", ld_vqqjet_p4.pt());
        arbol.setLeaf<double>("ld_vqqjet_eta", ld_vqqjet_p4.eta());
        arbol.setLeaf<double>("ld_vqqjet_phi", ld_vqqjet_p4.phi());
        arbol.setLeaf<double>("ld_vqqjet_mass", ld_vqqjet_p4.M());
        arbol.setLeaf<double>("tr_vqqjet_qgl", nt.Jet_qgl().at(tr_vqqjet_nanoidx));
        arbol.setLeaf<double>("tr_vqqjet_pt", tr_vqqjet_p4.pt());
        arbol.setLeaf<double>("tr_vqqjet_eta", tr_vqqjet_p4.eta());
        arbol.setLeaf<double>("tr_vqqjet_phi", tr_vqqjet_p4.phi());
        arbol.setLeaf<double>("tr_vqqjet_mass", tr_vqqjet_p4.M());
        arbol.setLeaf<double>("vqqjets_Mjj", (ld_vqqjet_p4 + tr_vqqjet_p4).M());
        arbol.setLeaf<double>("vqqjets_dR", min_dR);
        return true;
    };
};

class SaveVariables : public Core::AnalysisCut
{
public:
    Channel channel;

    SaveVariables(std::string name, Core::Analysis& analysis, Channel channel) 
    : Core::AnalysisCut(name, analysis) 
    {
        this->channel = channel;
    };

    bool evaluate()
    {
        // Save b-veto
        arbol.setLeaf<bool>("passes_bveto", arbol.getLeaf<int>("n_medium_b_jets") == 0);
        if (channel == AllMerged)
        {
            arbol.setLeaf<double>(
                "ST",
                arbol.getLeaf<double>("hbbfatjet_pt")
                + arbol.getLeaf<double>("ld_vqqfatjet_pt")
                + arbol.getLeaf<double>("tr_vqqfatjet_pt")
            );
            arbol.setLeaf<bool>("is_allmerged", true);
        }
        else if (channel == SemiMerged)
        {
            arbol.setLeaf<double>(
                "ST",
                arbol.getLeaf<double>("hbbfatjet_pt")
                + arbol.getLeaf<double>("ld_vqqfatjet_pt")
                + arbol.getLeaf<double>("ld_vqqjet_pt")
                + arbol.getLeaf<double>("tr_vqqjet_pt")
            );
            arbol.setLeaf<bool>("is_semimerged", true);
        }

        // Save alphaS variations
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

} // End namespace VBSVVHJets;

#endif
