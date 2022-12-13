#ifndef TRIGGERS_H
#define TRIGGERS_H

// VBS
#include "corrections/sfs.h"       // NanoSFsUL, SFHist
#include "corrections/leptons.h"   // LeptonSFs
// ROOT
#include "TString.h"
// CMSSW
#include "correction.h"

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

#endif
