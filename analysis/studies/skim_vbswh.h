#ifndef SKIM_VBSWH_H
#define SKIM_VBSWH_H

#include "core.h"
// RAPIDO
#include "arbusto.h"
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
        return ttH_UL::electronID(elec_i, ttH::IDfakable, nt.year());
    };

    virtual bool passesVetoMuonID(int muon_i)
    {
        return ttH_UL::muonID(muon_i, ttH::IDfakable, nt.year());
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
        for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
        {
            LorentzVector lep_p4 = nt.Electron_p4().at(elec_i);
            if (passesVetoElecID(elec_i)) { veto_lep_p4s.push_back(lep_p4); }
            if (passesTightElecID(elec_i)) { tight_lep_p4s.push_back(lep_p4); }
        }
        for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
        {
            LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
            if (passesVetoMuonID(muon_i)) { veto_lep_p4s.push_back(lep_p4); }
            if (passesTightMuonID(muon_i)) { tight_lep_p4s.push_back(lep_p4); }
        }
        globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
        globals.setVal<LorentzVectors>("tight_lep_p4s", tight_lep_p4s);
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

struct Skimmer : Core::Skimmer
{
    Skimmer(Arbusto& arbusto_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Core::Skimmer(arbusto_ref, nt_ref, cli_ref, cutflow_ref)
    {
        gconf.nanoAOD_ver = 9;

        cutflow.globals.newVar<LorentzVector>("lep_p4");
        cutflow.globals.newVar<LorentzVector>("hbbjet_p4");
        cutflow.globals.newVar<double>("ST", -999);
        cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s", {});
        cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s", {});
    };

    virtual void initCutflow()
    {
        Cut* base = new LambdaCut("Base", [&]() { return true; });
        cutflow.setRoot(base);

        Cut* find_leps = new FindLeptons("FindLeptons", *this);
        cutflow.insert(base, find_leps, Right);

        Cut* geq1_veto_lep = new LambdaCut(
            "Geq1VetoLep", 
            [&]()
            {
                return (cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1);
            }
        );
        cutflow.insert(find_leps, geq1_veto_lep, Right);

        Cut* geq2_jets = new Geq2Jets("Geq2Jets", *this);
        cutflow.insert(geq1_veto_lep, geq2_jets, Right);

        Cut* geq1_fatjet_loose = new Geq1FatJetLoose("Geq1FatJetLoose", *this);
        cutflow.insert(geq2_jets, geq1_fatjet_loose, Right);

        Cut* exactly1_lep = new Exactly1Lepton("Exactly1Lepton", *this);
        cutflow.insert(geq1_fatjet_loose, exactly1_lep, Right);

        Cut* geq1_fatjet_tight = new Geq1FatJetTight("Geq1FatJetTight", *this);
        cutflow.insert(exactly1_lep, geq1_fatjet_tight, Right);

        Cut* STgt800 = new LambdaCut(
            "STgt800", 
            [&]() { return (cutflow.globals.getVal<double>("ST") > 800); }
        );
        cutflow.insert(geq1_fatjet_tight, STgt800, Right);
    };
};

struct SkimmerPKU : Skimmer
{
    SkimmerPKU(Arbusto& arbusto_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : Skimmer(arbusto_ref, nt_ref, cli_ref, cutflow_ref)
    {
        // Do nothing
    };

    void initCutflow()
    {
        Skimmer::initCutflow();
        
        Cut* find_leps = new FindLeptonsPKU("FindLeptonsPKU", *this);
        cutflow.replace("FindLeptons", find_leps);
    };
};

}; // End namespace VBSWH

#endif
