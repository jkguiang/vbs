#ifndef JETS_H
#define JETS_H

// STL
#include <string>
// ROOT
#include "TRandom3.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "Tools/jetcorr/JetCorrectionUncertainty.h"
#include "Tools/jetcorr/JetResolutionUncertainty.h"

struct JetEnergyScales
{

    std::vector<JetCorrectionUncertainty*> jec_uncs;
    JetCorrectionUncertainty* ak8_jec_unc;
    JetResolutionUncertainty* jer_unc;
    // Note: jec_var == 1 means the nominal value is applied, 
    //       +/-2 means a variation is applied, 
    //       anything else means JECs are not applied.
    int jec_var;
    int jec_type;
    // Note: jer_var == 1 means the nominal value is applied, 
    //       +/-2 means a variation is applied, 
    //       anything else means JERs are not applied.
    int jer_var;
    TRandom3 random_num;
    std::string year_str;

    JetEnergyScales(std::string variation)
    {
        jec_type = 0;
        jer_var = 1;

        if (variation.substr(0, 4) == "jec_")
        {
            if (variation.substr(variation.size() - 3) == "_up") { jec_var = 2; }
            else if (variation.substr(variation.size() - 3) == "_dn") { jec_var = -2; }

            std::string jec_N = variation.substr(0, variation.size() - 3);
            if (jec_N == "jec") { jec_type = 0; }
            else if (jec_N == "jec_11") { jec_type = 11; }
            else if (jec_N == "jec_10") { jec_type = 10; }
            else if (jec_N == "jec_9")  { jec_type = 9; }
            else if (jec_N == "jec_8")  { jec_type = 8; }
            else if (jec_N == "jec_7")  { jec_type = 7; }
            else if (jec_N == "jec_6")  { jec_type = 6; }
            else if (jec_N == "jec_5")  { jec_type = 5; }
            else if (jec_N == "jec_4")  { jec_type = 4; }
            else if (jec_N == "jec_3")  { jec_type = 3; }
            else if (jec_N == "jec_2")  { jec_type = 2; }
            else if (jec_N == "jec_1")  { jec_type = 1; }

        }
        else if (variation.substr(0, 4) == "jer_")
        {
            if (variation.substr(variation.size() - 3) == "_up") { jer_var = 2; }
            else if (variation.substr(variation.size() - 3) == "_dn") { jer_var = -2; }
        }

        random_num = TRandom3(12345);
    };

    void init(TString file_name)
    {
        if (file_name.Contains("RunIISummer20UL16")) { year_str = "2016"; }
        else if (file_name.Contains("RunIISummer20UL17")) { year_str = "2017"; }
        else if (file_name.Contains("RunIISummer20UL18")) { year_str = "2018"; }

        // Init Jet Energy Correction (JEC) tool for "simple" JEC systematics
        // NOTE: must download them first!
        jec_uncs.push_back(new JetCorrectionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/"+gconf.jecEraMC+"_Uncertainty_AK4PFchs.txt"
        ));
        // Init JEC tool for "simple" AK8 JEC systematics
        ak8_jec_unc = new JetCorrectionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/"+gconf.jecEraMC+"_Uncertainty_AK8PFchs.txt"
        );
        // Init Jet Energy Resolution (JER) uncertainty scale factors
        // NOTE: must download them first!
        jer_unc = new JetResolutionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jerEra+"/"+gconf.jerEra+"_PtResolution_AK4PFchs.txt",
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jerEra+"/"+gconf.jerEra+"_SF_AK4PFchs.txt"
        );

        // No "Ungrouped" ("Regrouped" JEC txt files manually split by group) for data
        if (nt.isData())
        {
            return;
        }

        // Init the 11 JEC tools for "not simple" JEC systematics
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 1
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_Absolute.txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 2
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_Absolute_"+year_str+".txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 3
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_BBEC1_"+year_str+".txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 4
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_BBEC1.txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 5
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_EC2_"+year_str+".txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 6
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_EC2.txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 7
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_FlavorQCD.txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 8
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_HF_"+year_str+".txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 9
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_HF.txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 10
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_RelativeBal.txt"
        ));
        jec_uncs.push_back(new JetCorrectionUncertainty(
            // jec_type = 11
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/UngroupedV2_"+gconf.jecEraMC+"_UncertaintySources_AK4PFchs_RelativeSample_"+year_str+".txt"
        ));
    };

    LorentzVector applyAK4JEC(LorentzVector jet_p4)
    {
        JetCorrectionUncertainty* ak4_jec_unc = jec_uncs.at(jec_type);
        if (abs(jec_var) != 2) { return jet_p4; }
        ak4_jec_unc->setJetEta(jet_p4.eta());
        ak4_jec_unc->setJetPt(jet_p4.pt());
        float jec_err = fabs(ak4_jec_unc->getUncertainty(jec_var == 2))*jec_var/2;
        return jet_p4*(1. + jec_err);
    };

    LorentzVector applyAK8JEC(LorentzVector fatjet_p4)
    {
        // if (abs(jec_var) != 2) { return fatjet_p4; }
        // ak8_jec_unc->setJetEta(fatjet_p4.eta());
        // ak8_jec_unc->setJetPt(fatjet_p4.pt());
        // float jec_err = fabs(ak8_jec_unc->getUncertainty(jec_var == 2))*jec_var/2;
        // return fatjet_p4*(1. + jec_err);
        return applyAK4JEC(fatjet_p4); // per Giacomo's recommendation
    };

    LorentzVector applyJER(int seed, LorentzVector jet_p4, float rho, std::vector<LorentzVector> gen_jet_p4s)
    {
        random_num.SetSeed(seed);
        jer_unc->setJetEta(jet_p4.eta());
        jer_unc->setJetPt(jet_p4.pt());
        jer_unc->setRho(rho);
        jer_unc->applyJER(jet_p4, jer_var, gen_jet_p4s, random_num);
        return jet_p4;
    };
};

#endif
