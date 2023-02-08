#ifndef MVATTH_H
#define MVATTH_H

#include "Nano.h"

#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

namespace MVATTH
{

class MVATTH
{
public:

    // TMVA Reader object
    TMVA::Reader* reader;

    // Place holders for the variables
    Float_t Var_pt;
    Float_t Var_eta;
    Float_t Var_pfRelIso03_all;
    Float_t Var_miniPFRelIso_chg;
    Float_t Var_miniRelIsoNeutral;
    Float_t Var_jetNDauCharged;
    Float_t Var_jetPtRelv2;
    Float_t Var_jetPtRatio;
    Float_t Var_jetBTagDeepFlavB;
    Float_t Var_sip3d;
    Float_t Var_dxy;
    Float_t Var_dz;
    Float_t Var_mvaFall17V2noIso;

    MVATTH(TString xmlpath)
    {
        // This loads the library
        TMVA::Tools::Instance();

        // Initialize
        reader = new TMVA::Reader("Silent");

        reader->AddVariable("Electron_pt",                                                                                &Var_pt);
        reader->AddVariable("Electron_eta",                                                                               &Var_eta);
        reader->AddVariable("Electron_pfRelIso03_all",                                                                    &Var_pfRelIso03_all);
        reader->AddVariable("Electron_miniPFRelIso_chg",                                                                  &Var_miniPFRelIso_chg);
        reader->AddVariable("Electron_miniRelIsoNeutral := Electron_miniPFRelIso_all - Electron_miniPFRelIso_chg",        &Var_miniRelIsoNeutral);
        reader->AddVariable("Electron_jetNDauCharged",                                                                    &Var_jetNDauCharged);
        reader->AddVariable("Electron_jetPtRelv2",                                                                        &Var_jetPtRelv2);
        reader->AddVariable("Electron_jetPtRatio := min(1 / (1 + Electron_jetRelIso), 1.5)",                              &Var_jetPtRatio);
        reader->AddVariable("Electron_jetBTagDeepFlavB := Electron_jetIdx > -1 ? Jet_btagDeepFlavB[Electron_jetIdx] : 0", &Var_jetBTagDeepFlavB);
        reader->AddVariable("Electron_sip3d",                                                                             &Var_sip3d);
        reader->AddVariable("Electron_dxy := log(abs(Electron_dxy))",                                                     &Var_dxy);
        reader->AddVariable("Electron_dz  := log(abs(Electron_dz))",                                                      &Var_dz);
        reader->AddVariable("Electron_mvaFall17V2noIso",                                                                  &Var_mvaFall17V2noIso);

        reader->BookMVA("BDTG", xmlpath.Data());

    };

    ~MVATTH()
    {
        if (reader)
            delete reader;
    }

    void setVariables(int idx)
    {
        Var_pt                = nt.Electron_pt()[idx];
        Var_eta               = nt.Electron_eta()[idx];
        Var_pfRelIso03_all    = nt.Electron_pfRelIso03_all()[idx];
        Var_miniPFRelIso_chg  = nt.Electron_miniPFRelIso_chg()[idx];
        Var_miniRelIsoNeutral = nt.Electron_miniPFRelIso_all()[idx] - nt.Electron_miniPFRelIso_chg()[idx];
        Var_jetNDauCharged    = nt.Electron_jetNDauCharged()[idx];
        Var_jetPtRelv2        = nt.Electron_jetPtRelv2()[idx];
        Var_jetPtRatio        = min((double) 1 / (1 + nt.Electron_jetRelIso()[idx]), (double) 1.5);
        Var_jetBTagDeepFlavB  = nt.Electron_jetIdx()[idx] > -1 ? nt.Jet_btagDeepFlavB()[nt.Electron_jetIdx()[idx]] : 0;
        Var_sip3d             = nt.Electron_sip3d()[idx];
        Var_dxy               = log(max((double) 1e-10,abs((double) nt.Electron_dxy()[idx])));
        Var_dz                = log(max((double) 1e-10,abs((double) nt.Electron_dz()[idx])));
        Var_mvaFall17V2noIso  = nt.Electron_mvaFall17V2noIso()[idx];
    }

    float compute_Electron_mvaTTH(int idx)
    {
        setVariables(idx);
        return reader->EvaluateMVA("BDTG");
    }

};

} // End namespace MVATTH;

#endif
