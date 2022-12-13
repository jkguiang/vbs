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

    JetCorrectionUncertainty* ak4_jec_unc;
    JetCorrectionUncertainty* ak8_jec_unc;
    JetResolutionUncertainty* jer_unc;
    // Note: jec_var == 1 means the nominal value is applied, 
    //       +/-2 means a variation is applied, 
    //       anything else means JECs are not applied.
    int jec_var;
    // Note: jer_var == 1 means the nominal value is applied, 
    //       +/-2 means a variation is applied, 
    //       anything else means JERs are not applied.
    int jer_var;
    TRandom3 random_num;

    JetEnergyScales(std::string variation)
    {
        if (variation == "jec_up") 
        {
            jec_var = 2;
            jer_var = 1;
        }
        else if (variation == "jec_dn") 
        { 
            jec_var = -2;
            jer_var = 1;
        }
        else if (variation == "jer_up") 
        { 
            jec_var = 1;
            jer_var = 2;
        }
        else if (variation == "jer_dn") 
        { 
            jec_var = 1;
            jer_var = -2;
        }
        else
        {
            jec_var = 1;
            jer_var = 1;
        }
        random_num = TRandom3(12345);
    };

    void init()
    {
        // Init Jet Energy Correction (JEC) uncertainty scale factors
        // NOTE: must download them first!
        ak4_jec_unc = new JetCorrectionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/"+gconf.jecEraMC+"_Uncertainty_AK4PFchs.txt"
        );
        ak8_jec_unc = new JetCorrectionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/"+gconf.jecEraMC+"_Uncertainty_AK8PFchs.txt"
        );

        // Init Jet Energy Resolution (JER) uncertainty scale factors
        // NOTE: must download them first!
        jer_unc = new JetResolutionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jerEra+"/"+gconf.jerEra+"_PtResolution_AK4PFchs.txt",
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jerEra+"/"+gconf.jerEra+"_SF_AK4PFchs.txt"
        );
    };

    LorentzVector applyAK4JEC(LorentzVector jet_p4)
    {
        if (abs(jec_var) != 2) { return jet_p4; }
        ak4_jec_unc->setJetEta(jet_p4.eta());
        ak4_jec_unc->setJetPt(jet_p4.pt());
        float jec_err = fabs(ak4_jec_unc->getUncertainty(jec_var == 2))*jec_var/2;
        return jet_p4*(1. + jec_err);
    };

    LorentzVector applyAK8JEC(LorentzVector fatjet_p4)
    {
        if (abs(jec_var) != 2) { return fatjet_p4; }
        ak8_jec_unc->setJetEta(fatjet_p4.eta());
        ak8_jec_unc->setJetPt(fatjet_p4.pt());
        float jec_err = fabs(ak8_jec_unc->getUncertainty(jec_var == 2))*jec_var/2;
        return fatjet_p4*(1. + jec_err);
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
