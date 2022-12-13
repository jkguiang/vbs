#ifndef LEPTONS_H
#define LEPTONS_H

// VBS
#include "corrections/sfs.h"       // NanoSFsUL, SFHist
#include "core/pku.h"              // PKU::IDLevel, PKU::passesElecID, PKU::passesMuonID
#include "tools/TauIDSFTool.h"     // TauIDSFTool
// ROOT
#include "TString.h"
// NanoCORE
#include "Nano.h"
// CMSSW
#include "correction.h"

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

#endif
