#ifndef SFS_H
#define SFS_H

// STL
#include <filesystem>
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
private:
    TString input_file;

    void assertHist()
    {
        if (hist == nullptr)
        {
            throw std::runtime_error("SFHist - "+input_file+" not found");
        }
    }
public:
    TFile* tfile;
    TH1* hist;
    
    SFHist(TString input_root_file, TString hist_name)
    {
        if (!std::filesystem::exists(input_root_file.Data()))
        {
            tfile = nullptr;
            hist = nullptr;
            input_file = input_root_file;
        }
        else
        {
            tfile = new TFile(input_root_file);
            hist = (TH1*) tfile->Get(hist_name);
        }
    };
    
    double clip (double val, double val_max)
    {
        return (val >= val_max) ? std::nextafter(val_max, 0.0f) : val;
    };

    double getSF(double x) 
    { 
        assertHist();
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        return hist->GetBinContent(hist->FindBin(x)); 
    };
    double getSF(double x, double y) 
    { 
        assertHist();
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        double y_max = hist->GetYaxis()->GetXmax();
        y = clip(y, y_max);
        return hist->GetBinContent(hist->FindBin(x, y)); 
    };
    double getSF(double x, double y, double z) 
    { 
        assertHist();
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
        assertHist();
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        return hist->GetBinError(hist->FindBin(x)); 
    };
    double getErr(double x, double y) 
    { 
        assertHist();
        double x_max = hist->GetXaxis()->GetXmax();
        x = clip(x, x_max);
        double y_max = hist->GetYaxis()->GetXmax();
        y = clip(y, y_max);
        return hist->GetBinError(hist->FindBin(x, y)); 
    };
    double getErr(double x, double y, double z) 
    { 
        assertHist();
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
            year = -1; // Note: data files will land here!
        }
    };

    void assertYear()
    {
        if (year == -1)
        {
            throw std::runtime_error(
                "NanoSFsUL::assertYear - no scale factors loaded; campaign not in file name or file not found"
            );
        }
    }
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
        default:
            return;
            break;
        }
    };

    double getElecSF(double pt, double eta)
    {
        NanoSFsUL::assertYear();
        double sf = 1.;
        eta = std::min(fabs(eta), 2.4999);  // clip eta at < 2.5; take abs
        sf *= el_reco->getSF(eta, pt);      // event --> reco
        sf *= el_iso_loose->getSF(eta, pt); // reco --> loose ttH ID
        sf *= el_tth_tight->getSF(eta, pt); // loose ttH ID --> tight ttH ID
        return sf;
    };

    double getElecErrUp(double pt, double eta)
    {
        NanoSFsUL::assertYear();
        double err_up = 0.;
        eta = std::min(fabs(eta), 2.4999);                    // clip eta at < 2.5; take abs
        err_up += std::pow(el_reco->getErr(eta, pt), 2);      // event --> reco
        err_up += std::pow(el_iso_loose->getErr(eta, pt), 2); // reco --> loose ttH ID
        err_up += std::pow(el_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_up);
    };

    double getElecErrDn(double pt, double eta)
    {
        NanoSFsUL::assertYear();
        double err_dn = 0.;
        eta = std::min(fabs(eta), 2.4999);                    // clip eta at < 2.5; take abs
        err_dn += std::pow(el_reco->getErr(eta, pt), 2);      // event --> reco
        err_dn += std::pow(el_iso_loose->getErr(eta, pt), 2); // reco --> loose ttH ID
        err_dn += std::pow(el_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_dn);
    };

    double getMuonSF(double pt, double eta)
    {
        NanoSFsUL::assertYear();
        double sf = 1.;
        eta = fabs(eta);
        sf *= mu_pog_loose->getSF(eta, pt); // event --> loose POG ID
        sf *= mu_iso_loose->getSF(eta, pt); // loose POG ID --> loose ttH ID
        sf *= mu_tth_tight->getSF(eta, pt); // loose ttH ID --> tight ttH ID
        return sf;
    };

    double getMuonErrUp(double pt, double eta)
    {
        NanoSFsUL::assertYear();
        double err_up = 0.;
        eta = fabs(eta);
        err_up += std::pow(mu_pog_loose->getErr(eta, pt), 2); // event --> loose POG ID
        err_up += std::pow(mu_iso_loose->getErr(eta, pt), 2); // loose POG ID --> loose ttH ID
        err_up += std::pow(mu_tth_tight->getErr(eta, pt), 2); // loose ttH ID --> tight ttH ID
        return std::sqrt(err_up);
    };

    double getMuonErrDn(double pt, double eta)
    {
        NanoSFsUL::assertYear();
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
    double getEGM(std::string variation, double pt, double eta, std::string wp = "") 
    { 
        NanoSFsUL::assertYear();
        pt = std::max(pt, 10.);

        std::string working_point;
        switch (id_level)
        {
        case (PKU::IDveto):
            working_point = "Veto";
            break;
        case (PKU::IDtight):
            working_point = "Tight";
            break;
        default:
            return 1.;
            break;
        }

        if (wp != "") { working_point = wp; }

        return elec_sfs->evaluate({year_str, variation, working_point, eta, pt});
    };

    double getMUO(std::string variation, double pt, double eta, bool iso = false) 
    { 
        NanoSFsUL::assertYear();
        eta = std::min(fabs(eta), 2.3999);

        correction::Correction::Ref muon_sfs;
        switch (id_level)
        {
        case (PKU::IDveto):
            muon_sfs = (iso) ? muon_loose_iso_sfs : muon_tight_id_sfs;
            break;
        case (PKU::IDtight):
            muon_sfs = (iso) ? muon_tight_iso_sfs : muon_tight_id_sfs;
            break;
        default:
            return 1.;
            break;
        }

        return muon_sfs->evaluate({year_str+"_UL", eta, pt, variation});
    };
public:
    correction::Correction::Ref elec_sfs;
    correction::Correction::Ref muon_tight_id_sfs;
    correction::Correction::Ref muon_tight_iso_sfs;
    correction::Correction::Ref muon_loose_iso_sfs;
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
        default:
            return;
            break;
        };
        auto elec_cset = correction::CorrectionSet::from_file(elec_json_path);
        auto muon_cset = correction::CorrectionSet::from_file(muon_json_path);
        elec_sfs = elec_cset->at("UL-Electron-ID-SF");
        // Note: we only use the tight Muon POG ID in both the PKU veto and tight muon IDs
        //       currently, this may change in the future
        muon_tight_id_sfs = muon_cset->at("NUM_TightID_DEN_TrackerMuons");
        muon_tight_iso_sfs = muon_cset->at("NUM_TightRelIso_DEN_TightIDandIPCut");
        muon_loose_iso_sfs = muon_cset->at("NUM_LooseRelIso_DEN_TightIDandIPCut");
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

    double getElecRecoSF(double pt, double eta) 
    { 
        return getEGM("sf", pt, eta, "RecoAbove20"); 
    };

    double getElecRecoErrUp(double pt, double eta) 
    { 
        return getEGM("sfup", pt, eta, "RecoAbove20"); 
    };

    double getElecRecoErrDn(double pt, double eta) 
    { 
        return getEGM("sfdown", pt, eta, "RecoAbove20"); 
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

    double getMuonIsoSF(double pt, double eta) 
    { 
        return getMUO("sf", pt, eta, true); 
    };

    double getMuonIsoErrUp(double pt, double eta) 
    { 
        return getMUO("systup", pt, eta, true); 
    };

    double getMuonIsoErrDn(double pt, double eta) 
    { 
        return getMUO("systdown", pt, eta, true); 
    };
};

struct BTagSFs : NanoSFsUL
{
private:
    double get(std::string variation, int flavor, double pt, double eta) 
    { 
        NanoSFsUL::assertYear();
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
    std::string working_point;

    BTagSFs(std::string name, std::string working_point) 
    { 
        this->name = name;
        this->working_point = working_point;
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
        default:
            return;
            break;
        };
        auto cset = correction::CorrectionSet::from_file(json_path);
        sfs_bc = cset->at("deepJet_comb");
        sfs_light = cset->at("deepJet_incl");

        if (working_point == "L")
        {
            eff_b = new SFHist(root_path, "deepjet_eff_b_loose");
            eff_c = new SFHist(root_path, "deepjet_eff_c_loose");
            eff_light = new SFHist(root_path, "deepjet_eff_light_loose");
        }
        else if (working_point == "M")
        {
            eff_b = new SFHist(root_path, "deepjet_eff_b_medium");
            eff_c = new SFHist(root_path, "deepjet_eff_c_medium");
            eff_light = new SFHist(root_path, "deepjet_eff_light_medium");
        }
        else if (working_point == "T")
        {
            eff_b = new SFHist(root_path, "deepjet_eff_b_tight");
            eff_c = new SFHist(root_path, "deepjet_eff_c_tight");
            eff_light = new SFHist(root_path, "deepjet_eff_light_tight");
        }
    };

    double getSF(int flavor, double pt, double eta) 
    { 
        return get("central", flavor, pt, eta); 
    };

    double getSFUp(int flavor, double pt, double eta)
    { 
        return get("up_correlated", flavor, pt, eta); 
    };

    double getSFDn(int flavor, double pt, double eta)
    { 
        return get("down_correlated", flavor, pt, eta); 
    };

    double getEff(int flavor, double pt, double eta) 
    { 
        NanoSFsUL::assertYear();
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

struct PileUpSFs : NanoSFsUL
{
private:
    double get(std::string variation, float n_true_interactions) 
    { 
        NanoSFsUL::assertYear();
        n_true_interactions = std::min(n_true_interactions, 98.f);

        return sfs->evaluate({n_true_interactions, variation});
    };
public:
    correction::Correction::Ref sfs;

    PileUpSFs() { /* Do nothing */ };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        // Note: the gzipped JSONs in cvmfs can only be read by correctionlib v2.1.x
        std::string json_path = "data/pog_jsons/LUM";
        std::string sfs_name;
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            json_path += "/2016preVFP_UL/puWeights.json";
            sfs_name = "Collisions16_UltraLegacy_goldenJSON";
            break;
        case (RunIISummer20UL16):
            json_path += "/2016postVFP_UL/puWeights.json";
            sfs_name = "Collisions16_UltraLegacy_goldenJSON";
            break;
        case (RunIISummer20UL17):
            json_path += "/2017_UL/puWeights.json";
            sfs_name = "Collisions17_UltraLegacy_goldenJSON";
            break;
        case (RunIISummer20UL18):
            json_path += "/2018_UL/puWeights.json";
            sfs_name = "Collisions18_UltraLegacy_goldenJSON";
            break;
        default:
            return;
            break;
        };
        auto cset = correction::CorrectionSet::from_file(json_path);
        sfs = cset->at(sfs_name);
    };

    double getSF(float n_true_interactions) 
    { 
        return get("nominal", n_true_interactions); 
    };

    double getSFUp(float n_true_interactions) 
    { 
        return get("up", n_true_interactions); 
    };

    double getSFDn(float n_true_interactions) 
    { 
        return get("down", n_true_interactions); 
    };
};

struct HLT1LepSFs : LeptonSFs
{
private:
    double getMUO(std::string variation, double pt, double eta) 
    { 
        NanoSFsUL::assertYear();
        eta = std::min(fabs(eta), 2.3999);
        pt = std::max(pt, 26.);

        return muon_sfs->evaluate({year_str+"_UL", eta, pt, variation});
    };
public:
    SFHist* elec_sfs;
    correction::Correction::Ref muon_sfs;
    std::string year_str;

    HLT1LepSFs() { /* Do nothing */ };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        // Note: the gzipped JSONs in cvmfs can only be read by correctionlib v2.1.x
        std::string json_path = "data/pog_jsons/MUO";
        std::string root_path = "data/hlt_sfs";
        std::string sfs_name;
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            json_path += "/2016preVFP_UL/muon_Z.json";
            root_path += "/electron_hlt_sfs_2016.root";
            sfs_name = "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight";
            year_str = "2016preVFP";
            break;
        case (RunIISummer20UL16):
            json_path += "/2016postVFP_UL/muon_Z.json";
            root_path += "/electron_hlt_sfs_2016.root";
            sfs_name = "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight";
            year_str = "2016postVFP";
            break;
        case (RunIISummer20UL17):
            json_path += "/2017_UL/muon_Z.json";
            root_path += "/electron_hlt_sfs_2017.root";
            sfs_name = "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight";
            year_str = "2017";
            break;
        case (RunIISummer20UL18):
            json_path += "/2018_UL/muon_Z.json";
            root_path += "/electron_hlt_sfs_2018.root";
            sfs_name = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight";
            year_str = "2018";
            break;
        default:
            return;
            break;
        };
        auto cset = correction::CorrectionSet::from_file(json_path);

        elec_sfs = new SFHist(root_path, "EGamma_SF2D");
        muon_sfs = cset->at(sfs_name);
    };

    double getElecSF(double pt, double eta) { return elec_sfs->getSF(eta, pt); };

    double getElecErrUp(double pt, double eta) { return getElecSF(pt, eta) + elec_sfs->getErr(eta, pt); };

    double getElecErrDn(double pt, double eta) { return getElecSF(pt, eta) - elec_sfs->getErr(eta, pt); };

    double getMuonSF(double pt, double eta) { return getMUO("sf", pt, eta); };

    double getMuonErrUp(double pt, double eta) { return getMUO("systup", pt, eta); };

    double getMuonErrDn(double pt, double eta) { return getMUO("systdown", pt, eta); };
};

struct ParticleNetXbbSFs : NanoSFsUL
{
private:
    double boohftCalib(std::string year, double pt, std::string var = "nominal")
    {
        if (year == "2018")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_bb_ULNanoV9_PNetXbbVsQCD_ak8_2018/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **0.990** [-0.031/+0.027] | **1.040** [-0.034/+0.038] | **1.069** [-0.038/+0.056] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 0.990; }
                else if (var == "up") { return 0.990+0.027; }
                else if (var == "dn") { return 0.990-0.031; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.040; }
                else if (var == "up") { return 1.040+0.038; }
                else if (var == "dn") { return 1.040-0.034; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.069; }
                else if (var == "up") { return 1.069+0.056; }
                else if (var == "dn") { return 1.069-0.038; }
            }
        }
        if (year == "2017")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_bb_ULNanoV9_PNetXbbVsQCD_ak8_2017/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.020** [-0.025/+0.027] | **1.049** [-0.031/+0.041] | **1.030** [-0.030/+0.041] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.020; }
                else if (var == "up") { return 1.020+0.027; }
                else if (var == "dn") { return 1.020-0.025; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.049; }
                else if (var == "up") { return 1.049+0.041; }
                else if (var == "dn") { return 1.049-0.031; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.030; }
                else if (var == "up") { return 1.030+0.041; }
                else if (var == "dn") { return 1.030-0.030; }
            }
        }
        if (year == "2016postVFP")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_bb_ULNanoV9_PNetXbbVsQCD_ak8_2016/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.028** [-0.047/+0.046] | **1.090** [-0.098/+0.104] | **1.045** [-0.087/+0.102] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.028; }
                else if (var == "up") { return 1.028+0.046; }
                else if (var == "dn") { return 1.028-0.047; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.090; }
                else if (var == "up") { return 1.090+0.104; }
                else if (var == "dn") { return 1.090-0.098; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.045; }
                else if (var == "up") { return 1.045+0.102; }
                else if (var == "dn") { return 1.045-0.087; }
            }
        }
        if (year == "2016preVFP")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_bb_ULNanoV9_PNetXbbVsQCD_ak8_2016APV/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.038** [-0.115/+0.116] | **1.084** [-0.132/+0.137] | **1.027** [-0.142/+0.145] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.038; }
                else if (var == "up") { return 1.038+0.116; }
                else if (var == "dn") { return 1.038-0.115; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.084; }
                else if (var == "up") { return 1.084+0.137; }
                else if (var == "dn") { return 1.084-0.132; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.027; }
                else if (var == "up") { return 1.027+0.145; }
                else if (var == "dn") { return 1.027-0.142; }
            }
        }
        return 1.;
    };
public:
    std::string year_str;

    ParticleNetXbbSFs() { /* Do nothing */ };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            year_str = "2016preVFP";
            break;
        case (RunIISummer20UL16):
            year_str = "2016postVFP";
            break;
        case (RunIISummer20UL17):
            year_str = "2017";
            break;
        case (RunIISummer20UL18):
            year_str = "2018";
            break;
        default:
            return;
            break;
        };
    };

    double getSF(double pt) 
    { 
        return boohftCalib(year_str, pt);
    };

    double getSFUp(double pt) 
    { 
        return boohftCalib(year_str, pt, "up");
    };

    double getSFDn(double pt) 
    { 
        return boohftCalib(year_str, pt, "dn");
    };
};

#endif
