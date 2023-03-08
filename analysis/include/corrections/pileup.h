#ifndef PILEUP_H
#define PILEUP_H

// VBS
#include "corrections/sfs.h"
// CMSSW
#include "correction.h"

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

struct PileUpJetIDSFs : NanoSFsUL
{
private:
    double get(std::string variation, std::string wp, double pt, double eta) 
    { 
        NanoSFsUL::assertYear();
        return sfs->evaluate({eta, pt, variation, wp});
    };
public:
    correction::Correction::Ref sfs;

    PileUpJetIDSFs() { /* Do nothing */ };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        // Note: the gzipped JSONs in cvmfs can only be read by correctionlib v2.1.x
        std::string json_path = "data/pog_jsons/JME";
        std::string sfs_name;
        switch (campaign)
        {
        case (RunIISummer20UL16APV):
            json_path += "/2016preVFP_UL/jmar.json";
            sfs_name = "PUJetID_eff";
            break;
        case (RunIISummer20UL16):
            json_path += "/2016postVFP_UL/jmar.json";
            sfs_name = "PUJetID_eff";
            break;
        case (RunIISummer20UL17):
            json_path += "/2017_UL/jmar.json";
            sfs_name = "PUJetID_eff";
            break;
        case (RunIISummer20UL18):
            json_path += "/2018_UL/jmar.json";
            sfs_name = "PUJetID_eff";
            break;
        default:
            return;
            break;
        };
        auto cset = correction::CorrectionSet::from_file(json_path);
        sfs = cset->at(sfs_name);
    };

    double getSF(double pt, double eta, std::string wp = "L") 
    { 
        return get("nom", wp, pt, eta); 
    };

    double getSFUp(double pt, double eta, std::string wp = "L") 
    { 
        return get("up", wp, pt, eta); 
    };

    double getSFDn(double pt, double eta, std::string wp = "L") 
    { 
        return get("down", wp, pt, eta); 
    };
};

#endif
