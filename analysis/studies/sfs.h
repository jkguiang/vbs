#ifndef SFS_H
#define SFS_H

// VBS
#include "tools/TauIDSFTool.h"
// ROOT
#include "TString.h"
#include "TH1.h"
// NanoCORE
#include "Nano.h"
// CMSSW
#include "correction.h"

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

struct NanoSFsUL
{
    NanoCampaignUL campaign;
    int year;

    NanoSFsUL() { /* Do nothing */ };

    virtual void init(TString file_name)
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
            year = -1;
        }
    };
};

struct LeptonSFs : NanoSFsUL
{
    virtual double getElecSF(double pt, double eta) { return 1.; };
    virtual double getElecErrUp(double pt, double eta) { return 0.; };
    virtual double getElecErrDn(double pt, double eta) { return 0.; };

    virtual double getMuonSF(double pt, double eta) { return 1.; };
    virtual double getMuonErrUp(double pt, double eta) { return 0.; };
    virtual double getMuonErrDn(double pt, double eta) { return 0.; };
};

struct LeptonSFsTTH : LeptonSFs
{
    SFHist* el_reco;
    SFHist* el_iso_loose;
    SFHist* el_tth_tight;
    SFHist* mu_pog_loose;
    SFHist* mu_iso_loose;
    SFHist* mu_tth_tight;

    TauIDSFTool* tau_vs_jet;
    TauIDSFTool* tau_vs_mu;
    TauIDSFTool* tau_vs_el;

    LeptonSFsTTH() { /* Do nothing */ };

    void init(TString file_name, bool taus = false)
    {
        NanoSFsUL::init(file_name);

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

    double getElecSF(double pt, double eta)
    {
        double sf = 1.;
        eta = std::max(fabs(eta), 2.4999);  // clip eta at < 2.5; take abs
        sf *= el_reco->getSF(eta, pt);      // event --> reco
        sf *= el_iso_loose->getSF(eta, pt); // reco --> loose ttH ID
        sf *= el_tth_tight->getSF(eta, pt); // loose ttH ID --> tight ttH ID
        return sf;
    };

    double getElecErrUp(double pt, double eta)
    {
        double err_up = 0.;
        eta = std::max(fabs(eta), 2.4999);                    // clip eta at < 2.5; take abs
        err_up += std::pow(el_reco->getErr(eta, pt), 2);      // event --> reco
        err_up += std::pow(el_iso_loose->getErr(eta, pt), 2); // reco --> loose ttH ID
        err_up += std::pow(el_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_up);
    };

    double getElecErrDn(double pt, double eta)
    {
        double err_dn = 0.;
        eta = std::max(fabs(eta), 2.4999);                    // clip eta at < 2.5; take abs
        err_dn += std::pow(el_reco->getErr(eta, pt), 2);      // event --> reco
        err_dn += std::pow(el_iso_loose->getErr(eta, pt), 2); // reco --> loose ttH ID
        err_dn += std::pow(el_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_dn);
    };

    double getMuonSF(double pt, double eta)
    {
        double sf = 1.;
        eta = fabs(eta);
        sf *= mu_pog_loose->getSF(eta, pt); // event --> loose POG ID
        sf *= mu_iso_loose->getSF(eta, pt); // loose POG ID --> loose ttH ID
        sf *= mu_tth_tight->getSF(eta, pt); // loose ttH ID --> tight ttH ID
        return sf;
    };

    double getMuonErrUp(double pt, double eta)
    {
        double err_up = 0.;
        eta = fabs(eta);
        err_up += std::pow(mu_pog_loose->getErr(eta, pt), 2); // event --> loose POG ID
        err_up += std::pow(mu_iso_loose->getErr(eta, pt), 2); // loose POG ID --> loose ttH ID
        err_up += std::pow(mu_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_up);
    };

    double getMuonErrDn(double pt, double eta)
    {
        double err_dn = 0.;
        eta = fabs(eta);
        err_dn += std::pow(mu_pog_loose->getErr(eta, pt), 2); // event --> loose POG ID
        err_dn += std::pow(mu_iso_loose->getErr(eta, pt), 2); // loose POG ID --> loose ttH ID
        err_dn += std::pow(mu_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_dn);
    };
};

struct LeptonSFsPKU : LeptonSFs
{
    correction::Correction::Ref lep_sfs;

    LeptonSFsPKU() { /* Do nothing */ };

    void init(TString file_name, bool taus = false)
    {
        NanoSFsUL::init(file_name);

        /* FIXME: not able to read JSON for some reason...
        std::cout << "HELLO THERE" << std::endl;
        // std::unique_ptr<correction::CorrectionSet> cset;
        std::string json_path = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM";
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            json_path += "/2016preVFP_UL/electron.json.gz";
            break;
        case (RunIISummer20UL16):
            json_path += "/2016postVFP_UL/electron.json.gz";
            break;
        case (RunIISummer20UL17):
            json_path += "/2017_UL/electron.json.gz";
            break;
        case (RunIISummer20UL18):
            json_path += "/2018_UL/electron.json.gz";
            std::cout << "DEBUG " << json_path << std::endl;
            break;
        };
        auto cset = correction::CorrectionSet::from_file(json_path);
        std::cout << "I'M OK" << std::endl;
        */
    };

    double getElecSF(double pt, double eta) { return 1.; };    // FIXME
    double getElecErrUp(double pt, double eta) { return 0.; }; // FIXME
    double getElecErrDn(double pt, double eta) { return 0.; }; // FIXME

    double getMuonSF(double pt, double eta) { return 1.; };    // FIXME
    double getMuonErrUp(double pt, double eta) { return 0.; }; // FIXME
    double getMuonErrDn(double pt, double eta) { return 0.; }; // FIXME
};

struct BTagSFs : NanoSFsUL
{
private:
    double get(std::string variation, std::string working_point, int flavor, double pt, double eta) 
    { 
        eta = fabs(eta);
        switch (flavor)
        {
        case 0:
            return sfs_light->evaluate({variation, working_point, flavor, eta, pt});
            break;
        case 4:
        case 5:
            return sfs_bc->evaluate({variation, working_point, flavor, eta, pt});
            break;
        default:
            throw std::runtime_error("BTagSFs::getBTagSF - invalid hadron flavor (0, 4, 5 allowed)");
            break;
        }
    };
public:
    correction::Correction::Ref sfs_bc;
    correction::Correction::Ref sfs_light;

    BTagSFs() { /* Do nothing */ };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        std::cout << "HELLO THERE" << std::endl;
        std::unique_ptr<correction::CorrectionSet> cset;
        std::string json_base = "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV";
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            cset = correction::CorrectionSet::from_file(json_base+"/2016preVFP_UL/btagging.json.gz");
            break;
        case (RunIISummer20UL16):
            cset = correction::CorrectionSet::from_file(json_base+"/2016postVFP_UL/btagging.json.gz");
            break;
        case (RunIISummer20UL17):
            cset = correction::CorrectionSet::from_file(json_base+"/2017_UL/btagging.json.gz");
            break;
        case (RunIISummer20UL18):
            std::cout << "DEBUG " << json_base+"/2018_UL/btagging.json.gz" << std::endl;
            cset = correction::CorrectionSet::from_file(json_base+"/2018_UL/btagging.json.gz");
            break;
        }
        sfs_bc = cset->at("deepJet_incl");
        sfs_light = cset->at("deepJet_comb");
    };

    double getSF(std::string wp, int flavor, double pt, double eta) 
    { 
        return get("central", wp, flavor, pt, eta); 
    };
    double getSFUp(std::string wp, int flavor, double pt, double eta)
    { 
        return get("up_correlated", wp, flavor, pt, eta); 
    };
    double getSFDn(std::string wp, int flavor, double pt, double eta)
    { 
        return get("down_correlated", wp, flavor, pt, eta); 
    };
};

#endif
