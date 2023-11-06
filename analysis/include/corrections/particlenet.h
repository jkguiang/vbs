#ifndef PARTICLENET_H
#define PARTICLENET_H

// VBS
#include "corrections/sfs.h"       // NanoSFsUL, SFHist
// ROOT
#include "TString.h"

struct VBSWHXbbSFs : NanoSFsUL
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
        else if (year == "2017")
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
        else if (year == "2016postVFP")
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
        else if (year == "2016preVFP")
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

    VBSWHXbbSFs() { /* Do nothing */ };

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

struct VBSVVHXbbSFs : NanoSFsUL
{
private:
    // TODO: replace this with sfs from Lynn! These are just copied from VBSWH!!
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
        else if (year == "2017")
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
        else if (year == "2016postVFP")
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
        else if (year == "2016preVFP")
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

    VBSVVHXbbSFs() { /* Do nothing */ };

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

struct VBSVVHXWqqSFs : NanoSFsUL
{
private:
    double boohftCalib(std::string year, double pt, std::string wp, std::string var)
    {
        // From these slides: https://docs.google.com/presentation/d/1Y39avmA0bSe2hpPdqdM6RZr6s9N_h3iQEVLihOwebJY/edit?usp=sharing
        if (year == "2018")
        {
            if (pt >= 200 && pt < 300)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.91; }
                    else if (var == "up") { return 0.91+0.02; }
                    else if (var == "dn") { return 0.91-0.02; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.88; }
                    else if (var == "up") { return 0.88+0.02; }
                    else if (var == "dn") { return 0.88-0.02; }
                }
            }
            else if (pt >= 300 && pt < 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.90; }
                    else if (var == "up") { return 0.90+0.02; }
                    else if (var == "dn") { return 0.90-0.02; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.87; }
                    else if (var == "up") { return 0.87+0.02; }
                    else if (var == "dn") { return 0.87-0.02; }
                }
            }
            else if (pt >= 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.85; }
                    else if (var == "up") { return 0.85+0.04; }
                    else if (var == "dn") { return 0.85-0.04; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.82; }
                    else if (var == "up") { return 0.82+0.04; }
                    else if (var == "dn") { return 0.82-0.04; }
                }
            }
        }
        else if (year == "2017")
        {
            if (pt >= 200 && pt < 300)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.96; }
                    else if (var == "up") { return 0.96+0.02; }
                    else if (var == "dn") { return 0.96-0.02; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.91; }
                    else if (var == "up") { return 0.91+0.04; }
                    else if (var == "dn") { return 0.91-0.04; }
                }
            }
            else if (pt >= 300 && pt < 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.94; }
                    else if (var == "up") { return 0.94+0.02; }
                    else if (var == "dn") { return 0.94-0.02; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.91; }
                    else if (var == "up") { return 0.91+0.02; }
                    else if (var == "dn") { return 0.91-0.03; }
                }
            }
            else if (pt >= 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.93; }
                    else if (var == "up") { return 0.93+0.04; }
                    else if (var == "dn") { return 0.93-0.04; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.91; }
                    else if (var == "up") { return 0.91+0.05; }
                    else if (var == "dn") { return 0.91-0.04; }
                }
            }
        }
        else if (year == "2016postVFP")
        {
            if (pt >= 200 && pt < 300)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.96; }
                    else if (var == "up") { return 0.96+0.05; }
                    else if (var == "dn") { return 0.96-0.05; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.89; }
                    else if (var == "up") { return 0.89+0.06; }
                    else if (var == "dn") { return 0.89-0.05; }
                }
            }
            else if (pt >= 300 && pt < 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.89; }
                    else if (var == "up") { return 0.89+0.04; }
                    else if (var == "dn") { return 0.89-0.04; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.87; }
                    else if (var == "up") { return 0.87+0.04; }
                    else if (var == "dn") { return 0.87-0.04; }
                }
            }
            else if (pt >= 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.85; }
                    else if (var == "up") { return 0.85+0.07; }
                    else if (var == "dn") { return 0.85-0.06; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.85; }
                    else if (var == "up") { return 0.85+0.07; }
                    else if (var == "dn") { return 0.85-0.07; }
                }
            }
        }
        else if (year == "2016preVFP")
        {
            if (pt >= 200 && pt < 300)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.92; }
                    else if (var == "up") { return 0.92+0.04; }
                    else if (var == "dn") { return 0.92-0.04; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.87; }
                    else if (var == "up") { return 0.87+0.04; }
                    else if (var == "dn") { return 0.87-0.03; }
                }
            }
            else if (pt >= 300 && pt < 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.93; }
                    else if (var == "up") { return 0.93+0.04; }
                    else if (var == "dn") { return 0.93-0.03; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.86; }
                    else if (var == "up") { return 0.86+0.04; }
                    else if (var == "dn") { return 0.86-0.03; }
                }
            }
            else if (pt >= 400)
            {
                if (wp == "trail")
                {
                    // XWqq > 0.7 (for trailing Vqq fat jet)
                    if (var == "nominal") { return 0.94; }
                    else if (var == "up") { return 0.94+0.06; }
                    else if (var == "dn") { return 0.94-0.06; }
                }
                else if (wp == "lead")
                {
                    // XWqq > 0.8 (for leading Vqq fat jet)
                    if (var == "nominal") { return 0.91; }
                    else if (var == "up") { return 0.91+0.07; }
                    else if (var == "dn") { return 0.91-0.06; }
                }
            }
        }
        return 1.;
    };
public:
    std::string year_str;

    VBSVVHXWqqSFs() { /* Do nothing */ };

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

    double getSF(double pt, std::string wp) 
    { 
        return boohftCalib(year_str, pt, wp, "nominal");
    };

    double getSFUp(double pt, std::string wp) 
    { 
        return boohftCalib(year_str, pt, wp, "up");
    };

    double getSFDn(double pt, std::string wp) 
    { 
        return boohftCalib(year_str, pt, wp, "dn");
    };
};

#endif
