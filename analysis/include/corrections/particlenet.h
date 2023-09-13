#ifndef PARTICLENET_H
#define PARTICLENET_H

// VBS
#include "corrections/sfs.h"       // NanoSFsUL, SFHist
// ROOT
#include "TString.h"

struct ParticleNetXbbSFs : NanoSFsUL
{
private:
    double boohftCalib(std::string year, double pt, std::string var = "nominal")
    {
        if (year == "2018")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_Htobb_dipoleRecoilOn_bb_ULNanoV9_PNetXbbVsQCD_ak8_2018/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.000** [-0.024/+0.024] | **1.029** [-0.023/+0.034] | **1.026** [-0.016/+0.025] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.000; }
                else if (var == "up") { return 1.000+0.024; }
                else if (var == "dn") { return 1.000-0.024; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.029; }
                else if (var == "up") { return 1.029+0.034; }
                else if (var == "dn") { return 1.029-0.023; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.026; }
                else if (var == "up") { return 1.026+0.025; }
                else if (var == "dn") { return 1.026-0.016; }
            }
        }
        if (year == "2017")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_Htobb_dipoleRecoilOn_bb_ULNanoV9_PNetXbbVsQCD_ak8_2017/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.021** [-0.024/+0.028] | **1.018** [-0.022/+0.031] | **1.010** [-0.018/+0.019] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.021; }
                else if (var == "up") { return 1.021+0.028; }
                else if (var == "dn") { return 1.021-0.024; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.018; }
                else if (var == "up") { return 1.018+0.031; }
                else if (var == "dn") { return 1.018-0.022; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.010; }
                else if (var == "up") { return 1.010+0.019; }
                else if (var == "dn") { return 1.010-0.018; }
            }
        }
        if (year == "2016postVFP")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_Htobb_dipoleRecoilOn_bb_ULNanoV9_PNetXbbVsQCD_ak8_2016/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.044** [-0.040/+0.052] | **1.081** [-0.065/+0.080] | **0.996** [-0.039/+0.035] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.044; }
                else if (var == "up") { return 1.044+0.052; }
                else if (var == "dn") { return 1.044-0.040; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.081; }
                else if (var == "up") { return 1.081+0.080; }
                else if (var == "dn") { return 1.081-0.065; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 0.996; }
                else if (var == "up") { return 0.996+0.035; }
                else if (var == "dn") { return 0.996-0.039; }
            }
        }
        if (year == "2016preVFP")
        {
            /* Markdown table from web/VBSWH_mkW_Mjj100toInf_Htobb_dipoleRecoilOn_bb_ULNanoV9_PNetXbbVsQCD_ak8_2016APV/4_fit/index.html:
               |       | pT [250, 500) | pT [500, 700) | pT [700, +inf) |
               | :---: | :---: | :---: | :---: |
               | **High Purity** WP | **1.008** [-0.063/+0.064] | **1.027** [-0.055/+0.070] | **1.014** [-0.055/+0.057] |
            */
            if (pt >= 250 && pt < 500)
            {
                if (var == "nominal") { return 1.008; }
                else if (var == "up") { return 1.008+0.064; }
                else if (var == "dn") { return 1.008-0.063; }
            }
            else if (pt >= 500 && pt < 700)
            {
                if (var == "nominal") { return 1.027; }
                else if (var == "up") { return 1.027+0.070; }
                else if (var == "dn") { return 1.027-0.055; }
            }
            else if (pt >= 700)
            {
                if (var == "nominal") { return 1.014; }
                else if (var == "up") { return 1.014+0.057; }
                else if (var == "dn") { return 1.014-0.055; }
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
