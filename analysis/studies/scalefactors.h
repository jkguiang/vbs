#ifndef SCALEFACTORS_H
#define SCALEFACTORS_H

// ROOT
#include "TString.h"
#include "TH1.h"
#include "TRandom3.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "Tools/goodrun.h"
#include "Tools/btagsf/BTagCalibrationStandalone.h"
#include "Tools/btagsf/BTagCalibrationStandalone_v2.h"
#include "Tools/jetcorr/JetCorrectionUncertainty.h"
#include "Tools/jetcorr/JetResolutionUncertainty.h"
// Other
#include "tools/TauIDSFTool.h"

struct SFHist
{
    TFile* tfile;
    TH1* hist;
    
    SFHist(TString input_root_file, TString hist_name)
    {
        tfile = new TFile(input_root_file);
        hist = (TH1*) tfile->Get(hist_name);
    };
    
    double clip (double val, double val_max)
    {
        return (val >= val_max) ? std::nextafter(val_max, 0.0f) : val;
    };

    double getSF(double x) 
    { 
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        return hist->GetBinContent(hist->FindBin(x)); 
    };
    double getSF(double x, double y) 
    { 
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        double y_max = hist->GetYaxis()->GetXmax();
        y = clip(y, y_max);
        return hist->GetBinContent(hist->FindBin(x, y)); 
    };
    double getSF(double x, double y, double z) 
    { 
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        double y_max = hist->GetYaxis()->GetXmax();
        y = clip(y, y_max);
        double z_max = hist->GetZaxis()->GetXmax();
        y = clip(z, z_max);
        return hist->GetBinContent(hist->FindBin(x, y, z)); 
    };

    double getErr(double x) 
    { 
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        return hist->GetBinError(hist->FindBin(x)); 
    };
    double getErr(double x, double y) 
    { 
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        double y_max = hist->GetYaxis()->GetXmax();
        y = clip(y, y_max);
        return hist->GetBinError(hist->FindBin(x, y)); 
    };
    double getErr(double x, double y, double z) 
    { 
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        double y_max = hist->GetYaxis()->GetXmax();
        y = clip(y, y_max);
        double z_max = hist->GetZaxis()->GetXmax();
        y = clip(z, z_max);
        return hist->GetBinError(hist->FindBin(x, y, z)); 
    };
};

enum NanoCampaignUL
{
    RunIISummer20UL16APV,
    RunIISummer20UL16,
    RunIISummer20UL17,
    RunIISummer20UL18
};

struct NanoScaleFactorsUL
{
    NanoCampaignUL campaign;
    int year;

    SFHist* el_reco;
    SFHist* el_iso_loose;
    SFHist* el_tth_tight;
    SFHist* mu_pog_loose;
    SFHist* mu_iso_loose;
    SFHist* mu_tth_tight;

    TauIDSFTool* tau_vs_jet;
    TauIDSFTool* tau_vs_mu;
    TauIDSFTool* tau_vs_el;

    JetCorrectionUncertainty* jec_unc;
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

    NanoScaleFactorsUL(int jec_var = 1, int jer_var = 1) : jec_var(jec_var), jer_var(jer_var)
    {
        random_num = TRandom3(12345);
    };

    void init(TString file_name, bool taus = false)
    {
        if (file_name.Contains("RunIISummer20UL16"))
        {
            year = 2016;
            if (file_name.Contains("16APV")) { campaign = RunIISummer20UL16APV; }
            else { campaign = RunIISummer20UL16; }
        }
        else if (file_name.Contains("RunIISummer20UL17"))
        {
            year = 2017;
            campaign = RunIISummer20UL17;
        }
        else if (file_name.Contains("RunIISummer20UL18"))
        {
            year = 2018;
            campaign = RunIISummer20UL18;
        }
        else
        {
            return;
        }

        // Init Jet Energy Correction (JEC) uncertainty scale factors
        // NOTE: must download them first!
        jec_unc = new JetCorrectionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jecEraMC+"/"+gconf.jecEraMC+"_Uncertainty_AK4PFchs.txt"
        );

        // Init Jet Energy Resolution (JER) uncertainty scale factors
        // NOTE: must download them first!
        jer_unc = new JetResolutionUncertainty(
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jerEra+"/"+gconf.jerEra+"_PtResolution_AK4PFchs.txt",
            "NanoTools/NanoCORE/Tools/jetcorr/data/"+gconf.jerEra+"/"+gconf.jerEra+"_SF_AK4PFchs.txt"
        );

        // Init ttH lepton ID scale factors
        // NOTE: ttH uses VVVLoose tau ID, but sfs only available up to VVLoose
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            el_reco = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2016APV_recoToloose_EGM2D.root",
                "EGamma_SF2D"
            );
            el_iso_loose = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2016APV_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            el_tth_tight = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2016APV_2lss_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_pog_loose = new SFHist(
                "data/lepton_sfs/muon/Efficiencies_muon_generalTracks_Z_Run2016_UL_HIPM_ID.root",
                "NUM_LooseID_DEN_TrackerMuons_abseta_pt"
            );
            mu_iso_loose = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2016APV_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_tth_tight = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2016APV_EGM2D.root",
                "EGamma_SF2D"
            );
            if (!taus) { break; }
            tau_vs_jet = new TauIDSFTool("UL2016_preVFP", "DeepTau2017v2p1VSjet", "Medium", false, false);
            tau_vs_mu = new TauIDSFTool("2016Legacy", "DeepTau2017v2p1VSmu", "Loose", false, false); // FIXME: needs to be updated
            tau_vs_el = new TauIDSFTool("UL2016_preVFP", "DeepTau2017v2p1VSe", "VVLoose", false, false);
            break;
        case (RunIISummer20UL16):
            el_reco = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2016_recoToloose_EGM2D.root",
                "EGamma_SF2D"
            );
            el_iso_loose = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2016_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            el_tth_tight = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2016_2lss_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_pog_loose = new SFHist(
                "data/lepton_sfs/muon/Efficiencies_muon_generalTracks_Z_Run2016_UL_ID.root",
                "NUM_LooseID_DEN_TrackerMuons_abseta_pt"
            );
            mu_iso_loose = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2016_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_tth_tight = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2016_EGM2D.root",
                "EGamma_SF2D"
            );
            if (!taus) { break; }
            tau_vs_jet = new TauIDSFTool("UL2016_postVFP", "DeepTau2017v2p1VSjet", "Medium", false, false);
            tau_vs_mu = new TauIDSFTool("2016Legacy", "DeepTau2017v2p1VSmu", "Loose", false, false); // FIXME: needs to be updated
            tau_vs_el = new TauIDSFTool("UL2016_postVFP", "DeepTau2017v2p1VSe", "VVLoose", false, false);
            break;
        case (RunIISummer20UL17):
            el_reco = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2017_recoToloose_EGM2D.root",
                "EGamma_SF2D"
            );
            el_iso_loose = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2017_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            el_tth_tight = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2017_2lss_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_pog_loose = new SFHist(
                "data/lepton_sfs/muon/Efficiencies_muon_generalTracks_Z_Run2017_UL_ID.root",
                "NUM_LooseID_DEN_TrackerMuons_abseta_pt"
            );
            mu_iso_loose = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2017_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_tth_tight = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2017_EGM2D.root",
                "EGamma_SF2D"
            );
            if (!taus) { break; }
            tau_vs_jet = new TauIDSFTool("UL2017", "DeepTau2017v2p1VSjet", "Medium", false, false);
            tau_vs_mu = new TauIDSFTool("2017ReReco", "DeepTau2017v2p1VSmu", "Loose", false, false); // FIXME: needs to be updated
            tau_vs_el = new TauIDSFTool("UL2017", "DeepTau2017v2p1VSe", "VVLoose", false, false);
            break;
        case (RunIISummer20UL18):
            el_reco = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2018_recoToloose_EGM2D.root",
                "EGamma_SF2D"
            );
            el_iso_loose = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2018_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            el_tth_tight = new SFHist(
                "data/lepton_sfs/elec/egammaEffi2018_2lss_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_pog_loose = new SFHist(
                "data/lepton_sfs/muon/Efficiencies_muon_generalTracks_Z_Run2018_UL_ID.root",
                "NUM_LooseID_DEN_TrackerMuons_abseta_pt"
            );
            mu_iso_loose = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2018_iso_EGM2D.root",
                "EGamma_SF2D"
            );
            mu_tth_tight = new SFHist(
                "data/lepton_sfs/muon/egammaEffi2018_EGM2D.root",
                "EGamma_SF2D"
            );
            if (!taus) { break; }
            tau_vs_jet = new TauIDSFTool("UL2018", "DeepTau2017v2p1VSjet", "Medium", false, false);
            tau_vs_mu = new TauIDSFTool("2018ReReco", "DeepTau2017v2p1VSmu", "Loose", false, false); // FIXME: needs to be updated
            tau_vs_el = new TauIDSFTool("UL2018", "DeepTau2017v2p1VSe", "VVLoose", false, false);
            break;
        }
    };

    LorentzVector applyJEC(LorentzVector jet_p4)
    {
        if (abs(jec_var) != 2) { return jet_p4; }
        jec_unc->setJetEta(jet_p4.eta());
        jec_unc->setJetPt(jet_p4.pt());
        float jec_err = fabs(jec_unc->getUncertainty(jec_var == 2))*jec_var/2;
        return jet_p4*(1. + jec_err);
    };

    LorentzVector applyJER(int seed, LorentzVector jet_p4, float rho, 
                           std::vector<LorentzVector> gen_jet_p4s)
    {
        /* FIXME: GenJet_* branches missing in current skim
        random_num.SetSeed(seed);
        jer_unc->setJetEta(jet_p4.eta());
        jer_unc->setJetPt(jet_p4.pt());
        jer_unc->setRho(rho);
        jer_unc->applyJER(jet_p4, jer_var, gen_jet_p4s, random_num); 
        */
        return jet_p4;
    };
};

#endif
