#ifndef BTAGS_H
#define BTAGS_H

// VBS
#include "corrections/sfs.h"       // NanoSFsUL, SFHist
// ROOT
#include "TString.h"
// CMSSW
#include "correction.h"

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

#endif
