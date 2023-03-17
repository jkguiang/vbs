#ifndef QCD_H
#define QCD_H

// VBS
#include "corrections/sfs.h"       // NanoSFsUL, SFHist
// ROOT
#include "TString.h"
// CMSSW
#include "correction.h"

struct QCDPNetXbbSFs : NanoSFsUL
{
    SFHist* sfs;
    TString hist_name;

    QCDPNetXbbSFs() 
    { 
        // hist_name = "RatioDataMC__SemiMerged_SelectVBSJets__hbbfatjet_xbbscore3D";
        hist_name = "RatioDataMC__SemiMerged_SelectVBSJets__hbbfatjet_xbbscore2D";
    };

    void init(TString file_name)
    {
        NanoSFsUL::init(file_name);

        // TODO: might want to switch to yearly sfs
        // switch (campaign)
        // {
        // case (RunIISummer20UL16APV):
        //     root_path += "/2016preVFP/qcd_sfs.root";
        //     break;
        // case (RunIISummer20UL16):
        //     root_path += "/2016postVFP/qcd_sfs.root";
        //     break;
        // case (RunIISummer20UL17):
        //     root_path += "/2017/qcd_sfs.root";
        //     break;
        // case (RunIISummer20UL18):
        //     root_path += "/2018/qcd_sfs.root";
        //     break;
        // default:
        //     return;
        //     break;
        // };
        std::string root_path = "data/vbsvvhjets_sfs/qcd_pnet_sfs.root";

        sfs = new SFHist(root_path, hist_name);
    };

    // double getSF(double pt, double eta, double score) 
    double getSF(double pt, double score) 
    { 
        // return sfs->getSF(pt, fabs(eta), score); 
        return sfs->getSF(pt, score); 
    };

    // double getSFUp(double pt, double eta, double score)
    double getSFUp(double pt, double score)
    { 
        // eta = fabs(eta);
        // return getSF(pt, eta, score) + sfs->getErr(pt, eta, score); 
        return getSF(pt, score) + sfs->getErr(pt, score); 
    };

    // double getSFDn(double pt, double eta, double score)
    double getSFDn(double pt, double score)
    { 
        // eta = fabs(eta);
        // return getSF(pt, eta, score) - sfs->getErr(pt, eta, score); 
        return getSF(pt, score) - sfs->getErr(pt, score); 
    };
};

struct QCDPNetXVqqSFs : QCDPNetXbbSFs
{
    SFHist* sfs;
    TString hist_name;

    QCDPNetXVqqSFs() 
    { 
        // hist_name = "RatioDataMC__SemiMerged_ApplyXbbReweighting__ld_vqqfatjet_xwqqscore3D";
        hist_name = "RatioDataMC__SemiMerged_ApplyXbbReweighting__ld_vqqfatjet_xwqqscore2D";
    };
};

#endif
