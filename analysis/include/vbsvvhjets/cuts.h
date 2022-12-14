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
#include "vbswh/collections.h"
#include "vbswh/cuts.h"

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
    SelectVVHFatJets(std::string name, Core::Analysis& analysis) 
    : Core::AnalysisCut(name, analysis) 
    {
        // Do nothing
    };

    bool evaluate()
    {
    };
};

class SelectJetsNoFatJetOverlap : public Core::SelectJets
{
public:
    // LorentzVector hbbjet_p4;

    SelectJetsNoFatJetOverlap(std::string name, Core::Analysis& analysis, JetEnergyScales* jes = nullptr, 
                              BTagSFs* btag_sfs = nullptr) 
    : Core::SelectJets(name, analysis, jes, btag_sfs) 
    {
        // Do nothing
    };

    void loadOverlapVars()
    {
        // veto_lep_p4s = globals.getVal<LorentzVectors>("veto_lep_p4s");
        // veto_lep_jet_idxs = globals.getVal<Integers>("veto_lep_jet_idxs");
        // hbbjet_p4 = globals.getVal<LorentzVector>("hbbjet_p4");
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

} // End namespace VBSVVHJets;

#endif
