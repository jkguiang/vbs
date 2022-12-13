#ifndef SFS_H
#define SFS_H

// STL
#include <filesystem>
// ROOT
#include "TString.h"
#include "TH1.h"
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

#endif
