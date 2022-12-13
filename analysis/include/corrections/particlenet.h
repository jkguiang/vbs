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
