#ifndef SFS_H
#define SFS_H

// VBS
#include "pku.h"
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
            if (file_name.Contains("NanoAODAPV") || file_name.Contains("UL16APV")) 
            { 
                campaign = RunIISummer20UL16APV; 
            }
            else 
            { 
                campaign = RunIISummer20UL16; 
            }
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
        eta = std::min(fabs(eta), 2.4999);  // clip eta at < 2.5; take abs
        sf *= el_reco->getSF(eta, pt);      // event --> reco
        sf *= el_iso_loose->getSF(eta, pt); // reco --> loose ttH ID
        sf *= el_tth_tight->getSF(eta, pt); // loose ttH ID --> tight ttH ID
        return sf;
    };

    double getElecErrUp(double pt, double eta)
    {
        double err_up = 0.;
        eta = std::min(fabs(eta), 2.4999);                    // clip eta at < 2.5; take abs
        err_up += std::pow(el_reco->getErr(eta, pt), 2);      // event --> reco
        err_up += std::pow(el_iso_loose->getErr(eta, pt), 2); // reco --> loose ttH ID
        err_up += std::pow(el_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_up);
    };

    double getElecErrDn(double pt, double eta)
    {
        double err_dn = 0.;
        eta = std::min(fabs(eta), 2.4999);                    // clip eta at < 2.5; take abs
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
private:
    double getEGM(std::string variation, double pt, double eta) 
    { 
        pt = std::max(pt, 10.);
        if (id_level == PKU::IDveto) 
        { 
            return elec_sfs->evaluate({year_str, variation, "Veto", eta, pt});
        }
        else if (id_level == PKU::IDtight) 
        { 
            return elec_sfs->evaluate({year_str, variation, "Tight", eta, pt});
        }
        else
        {
            return 1.;
        }
    };

    double getMUO(std::string variation, double pt, double eta) 
    { 
        eta = std::min(fabs(eta), 2.3999);
        double id_val = muon_id_sfs->evaluate({year_str+"_UL", eta, pt, variation});
        double iso_val = muon_iso_sfs->evaluate({year_str+"_UL", eta, pt, variation});
        if (variation == "sf")
        {
            return id_val*iso_val;
        }
        else if (variation == "systup" || variation == "systdown")
        {
            return std::sqrt(std::pow(id_val, 2) + std::pow(iso_val, 2));
        }
        else
        {
            return 1.;
        }
    };
public:
    correction::Correction::Ref elec_sfs;
    correction::Correction::Ref muon_id_sfs;
    correction::Correction::Ref muon_iso_sfs;
    std::string year_str;
    PKU::IDLevel id_level;

    LeptonSFsPKU(PKU::IDLevel id_level) { this->id_level = id_level; };

    void init(TString file_name, bool taus = false)
    {
        NanoSFsUL::init(file_name);

        // Note: the gzipped JSONs in cvmfs can only be read by correctionlib v2.1.x
        std::string elec_json_path = "data/pog_jsons/EGM";
        std::string muon_json_path = "data/pog_jsons/MUO";
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            elec_json_path += "/2016preVFP_UL/electron.json";
            muon_json_path += "/2016preVFP_UL/muon_Z.json";
            year_str = "2016preVFP";
            break;
        case (RunIISummer20UL16):
            elec_json_path += "/2016postVFP_UL/electron.json";
            muon_json_path += "/2016postVFP_UL/muon_Z.json";
            year_str = "2016postVFP";
            break;
        case (RunIISummer20UL17):
            elec_json_path += "/2017_UL/electron.json";
            muon_json_path += "/2017_UL/muon_Z.json";
            year_str = "2017";
            break;
        case (RunIISummer20UL18):
            elec_json_path += "/2018_UL/electron.json";
            muon_json_path += "/2018_UL/muon_Z.json";
            year_str = "2018";
            break;
        };
        auto elec_cset = correction::CorrectionSet::from_file(elec_json_path);
        auto muon_cset = correction::CorrectionSet::from_file(muon_json_path);
        elec_sfs = elec_cset->at("UL-Electron-ID-SF");
        // Note: we only use the tight Muon POG ID in both the PKU veto and tight muon IDs
        //       currently, this may change in the future
        muon_id_sfs = muon_cset->at("NUM_TightID_DEN_TrackerMuons");
        muon_iso_sfs = muon_cset->at("NUM_TightRelIso_DEN_TightIDandIPCut");
    };

    double getElecSF(double pt, double eta) 
    { 
        return getEGM("sf", pt, eta); 
    };

    double getElecErrUp(double pt, double eta) 
    { 
        return getEGM("sfup", pt, eta); 
    };

    double getElecErrDn(double pt, double eta) 
    { 
        return getEGM("sfdown", pt, eta); 
    };

    double getMuonSF(double pt, double eta) 
    { 
        return getMUO("sf", pt, eta); 
    };

    double getMuonErrUp(double pt, double eta) 
    { 
        return getMUO("systup", pt, eta); 
    };

    double getMuonErrDn(double pt, double eta) 
    { 
        return getMUO("systdown", pt, eta); 
    };
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
    SFHist* eff_b;
    SFHist* eff_c;
    SFHist* eff_light;
    correction::Correction::Ref sfs_bc;
    correction::Correction::Ref sfs_light;
    std::string name;
    std::string wp;

    BTagSFs(std::string name, std::string wp) 
    { 
        this->name = name;
        this->wp = wp;
    };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        // Note: the gzipped JSONs in cvmfs can only be read by correctionlib v2.1.x
        std::string json_path = "data/pog_jsons/BTV";
        std::string root_path = "studies/btageff/output_vbswh";
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            json_path += "/2016preVFP_UL/btagging.json";
            root_path += "/2016preVFP/"+name+".root";
            break;
        case (RunIISummer20UL16):
            json_path += "/2016postVFP_UL/btagging.json";
            root_path += "/2016postVFP/"+name+".root";
            break;
        case (RunIISummer20UL17):
            json_path += "/2017_UL/btagging.json";
            root_path += "/2017/"+name+".root";
            break;
        case (RunIISummer20UL18):
            json_path += "/2018_UL/btagging.json";
            root_path += "/2018/"+name+".root";
            break;
        };
        auto cset = correction::CorrectionSet::from_file(json_path);
        sfs_bc = cset->at("deepJet_comb");
        sfs_light = cset->at("deepJet_incl");

        if (wp == "L")
        {
            eff_b = new SFHist(root_path, "deepjet_eff_b_loose");
            eff_c = new SFHist(root_path, "deepjet_eff_c_loose");
            eff_light = new SFHist(root_path, "deepjet_eff_light_loose");
        }
        else if (wp == "M")
        {
            eff_b = new SFHist(root_path, "deepjet_eff_b_medium");
            eff_c = new SFHist(root_path, "deepjet_eff_c_medium");
            eff_light = new SFHist(root_path, "deepjet_eff_light_medium");
        }
        else if (wp == "T")
        {
            eff_b = new SFHist(root_path, "deepjet_eff_b_tight");
            eff_c = new SFHist(root_path, "deepjet_eff_c_tight");
            eff_light = new SFHist(root_path, "deepjet_eff_light_tight");
        }
    };

    double getSF(int flavor, double pt, double eta) 
    { 
        return get("central", wp, flavor, pt, eta); 
    };

    double getSFUp(int flavor, double pt, double eta)
    { 
        return get("up_correlated", wp, flavor, pt, eta); 
    };

    double getSFDn(int flavor, double pt, double eta)
    { 
        return get("down_correlated", wp, flavor, pt, eta); 
    };

    double getEff(int flavor, double pt, double eta) 
    { 
        switch (flavor)
        {
        case 0:
            return eff_light->getSF(pt, eta);
            break;
        case 4:
            return eff_c->getSF(pt, eta);
            break;
        case 5:
            return eff_b->getSF(pt, eta);
            break;
        default:
            return 1.;
            break;
        }
    };
};

#endif
