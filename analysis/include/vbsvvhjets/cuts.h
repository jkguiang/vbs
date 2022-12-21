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
#include "vbsvvhjets/enums.h"
#include "corrections/all.h"

namespace VBSVVHJets
{

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
        Doubles good_fatjet_xwqqtags = globals.getVal<Doubles>("good_fatjet_xwqqtags");
        Doubles good_fatjet_msoftdrops = globals.getVal<Doubles>("good_fatjet_msoftdrops");
        Doubles good_fatjet_masses = globals.getVal<Doubles>("good_fatjet_masses");

        // Select Hbb fat jet candidate first
        int best_xbb_i = (
            std::max_element(good_fatjet_xbbtags.begin(), good_fatjet_xbbtags.end()) - good_fatjet_xbbtags.begin()
        );
        LorentzVector hbbfatjet_p4 = good_fatjet_p4s.at(best_xbb_i);
        globals.setVal<LorentzVector>("hbbfatjet_p4", hbbfatjet_p4);
        arbol.setLeaf<double>("hbbfatjet_score", good_fatjet_xbbtags.at(best_xbb_i));
        arbol.setLeaf<double>("hbbfatjet_pt", hbbfatjet_p4.pt());
        arbol.setLeaf<double>("hbbfatjet_eta", hbbfatjet_p4.eta());
        arbol.setLeaf<double>("hbbfatjet_phi", hbbfatjet_p4.phi());
        arbol.setLeaf<double>("hbbfatjet_mass", good_fatjet_masses.at(best_xbb_i));
        arbol.setLeaf<double>("hbbfatjet_msoftdrop", good_fatjet_msoftdrops.at(best_xbb_i));
        // Remove Hbb fat jet candidate from consideration
        good_fatjet_p4s.erase(good_fatjet_p4s.begin() + best_xbb_i);
        good_fatjet_xbbtags.erase(good_fatjet_xbbtags.begin() + best_xbb_i);
        good_fatjet_xwqqtags.erase(good_fatjet_xwqqtags.begin() + best_xbb_i);
        good_fatjet_msoftdrops.erase(good_fatjet_msoftdrops.begin() + best_xbb_i);
        good_fatjet_masses.erase(good_fatjet_masses.begin() + best_xbb_i);

        // Select W/Z candidate(s) last
        if (channel == AllMerged)
        {
            unsigned int ld_fatjet_i;
            unsigned int tr_fatjet_i;
            if (good_fatjet_p4s.at(0).pt() > good_fatjet_p4s.at(1).pt())
            {
                ld_fatjet_i = 0;
                tr_fatjet_i = 1;
            }
            else
            {
                ld_fatjet_i = 1;
                tr_fatjet_i = 0;
            }
            LorentzVector ld_vqqfatjet_p4 = good_fatjet_p4s.at(ld_fatjet_i);
            LorentzVector tr_vqqfatjet_p4 = good_fatjet_p4s.at(tr_fatjet_i);
            globals.setVal<LorentzVector>("ld_vqqfatjet_p4", ld_vqqfatjet_p4);
            arbol.setLeaf<double>("ld_vqqfatjet_score", good_fatjet_xbbtags.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_pt", ld_vqqfatjet_p4.pt());
            arbol.setLeaf<double>("ld_vqqfatjet_eta", ld_vqqfatjet_p4.eta());
            arbol.setLeaf<double>("ld_vqqfatjet_phi", ld_vqqfatjet_p4.phi());
            arbol.setLeaf<double>("ld_vqqfatjet_mass", good_fatjet_masses.at(ld_fatjet_i));
            arbol.setLeaf<double>("ld_vqqfatjet_msoftdrop", good_fatjet_msoftdrops.at(ld_fatjet_i));
            globals.setVal<LorentzVector>("tr_vqqfatjet_p4", tr_vqqfatjet_p4);
            arbol.setLeaf<double>("tr_vqqfatjet_score", good_fatjet_xbbtags.at(tr_fatjet_i));
            arbol.setLeaf<double>("tr_vqqfatjet_pt", ld_vqqfatjet_p4.pt());
            arbol.setLeaf<double>("tr_vqqfatjet_eta", ld_vqqfatjet_p4.eta());
            arbol.setLeaf<double>("tr_vqqfatjet_phi", ld_vqqfatjet_p4.phi());
            arbol.setLeaf<double>("tr_vqqfatjet_mass", good_fatjet_masses.at(tr_fatjet_i));
            arbol.setLeaf<double>("tr_vqqfatjet_msoftdrop", good_fatjet_msoftdrops.at(tr_fatjet_i));
        }
        else if (channel == SemiMerged)
        {
            LorentzVector vqqfatjet_p4 = good_fatjet_p4s.at(0);
            arbol.setLeaf<double>("ld_vqqfatjet_score", good_fatjet_xbbtags.at(0));
            arbol.setLeaf<double>("ld_vqqfatjet_pt", vqqfatjet_p4.pt());
            arbol.setLeaf<double>("ld_vqqfatjet_eta", vqqfatjet_p4.eta());
            arbol.setLeaf<double>("ld_vqqfatjet_phi", vqqfatjet_p4.phi());
            arbol.setLeaf<double>("ld_vqqfatjet_mass", good_fatjet_masses.at(0));
            arbol.setLeaf<double>("ld_vqqfatjet_msoftdrop", good_fatjet_msoftdrops.at(0));
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
                              JetEnergyScales* jes = nullptr, BTagSFs* btag_sfs = nullptr) 
    : Core::SelectJets(name, analysis, jes, btag_sfs) 
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
        if (channel == SemiMerged)
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
        arbol.setLeaf<bool>("passes_bveto", arbol.getLeaf<int>("n_medium_b_jets") == 0);
        if (channel == AllMerged)
        {
            arbol.setLeaf<double>(
                "ST",
                arbol.getLeaf<double>("hbbfatjet_pt")
                + arbol.getLeaf<double>("ld_vqqfatjet_pt")
                + arbol.getLeaf<double>("tr_vqqfatjet_pt")
            );
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
        }
        return true;
    };
};

} // End namespace VBSVVHJets;

#endif
