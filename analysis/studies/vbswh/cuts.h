#ifndef CUTS_H
#define CUTS_H

#include "vbswh/collections.h"
#include "vbswh/cuts.h"

class FixEWKSamples : public Core::AnalysisCut
{
public:
    SFHist* ewk_fix;

    FixEWKSamples(std::string name, VBSWH::Analysis& analysis) 
    : Core::AnalysisCut(name, analysis) 
    {
        ewk_fix = new SFHist("data/ewk_fix.root", "Wgt__pdgid5_quarks_pt_varbin");
    };

    int getChargeQx3(int q_pdgID)
    {
        switch (abs(q_pdgID))
        {
        case 1:
            return -1; // down
        case 2:
            return  2; // up
        case 3:
            return -1; // strange
        case 4:
            return  2; // charm
        case 5:
            return -1; // bottom
        case 6:
            return  2; // top
        default:
            return -999;
        }
    };

    double getChargeQQ(int q1_pdgID, int q2_pdgID)
    {
        int q1_sign = (q1_pdgID > 0) - (q1_pdgID < 0);
        int q2_sign = (q2_pdgID > 0) - (q2_pdgID < 0);
        return (q1_sign*getChargeQx3(q1_pdgID) + q2_sign*getChargeQx3(q2_pdgID)) / 3.;
    };

    bool evaluate()
    {
        TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        if (file_name.Contains("EWKWPlus") || file_name.Contains("EWKWMinus"))
        {
            int q1_pdgID = nt.LHEPart_pdgId().at(4);
            int q2_pdgID = nt.LHEPart_pdgId().at(5);
            LorentzVector q1_p4 = nt.LHEPart_p4().at(4);
            LorentzVector q2_p4 = nt.LHEPart_p4().at(5);
            double M_qq = (q1_p4 + q2_p4).M();
            LorentzVector lep_p4 = nt.LHEPart_p4().at(2);
            LorentzVector nu_p4 = nt.LHEPart_p4().at(3);
            double M_lnu = (lep_p4 + nu_p4).M();
            bool is_WW = (
                fabs(getChargeQQ(q1_pdgID, q2_pdgID)) == 1 
                && M_qq >= 70 && M_qq < 90 
                && M_lnu >= 70 && M_lnu < 90
            );
            if (is_WW)
            {
                // WW event
                arbol.setLeaf<double>("ewkfix_sf", 0.);
                return true;
            }
            else if (M_qq >= 95)
            {
                // VBS W event
                double bquark_pt = -999;
                if (abs(q1_pdgID) == 5 && abs(q2_pdgID) == 5)
                {
                    bquark_pt = (q1_p4.pt() > q2_p4.pt()) ? q1_p4.pt() : q2_p4.pt();
                }
                else if (abs(q1_pdgID) == 5)
                {
                    bquark_pt = q1_p4.pt();
                }
                else if (abs(q2_pdgID) == 5)
                {
                    bquark_pt = q2_p4.pt();
                }
                if (bquark_pt != -999)
                {
                    arbol.setLeaf<double>("ewkfix_sf", ewk_fix->getSF(bquark_pt));
                    return true;
                }
            }
        }
        arbol.setLeaf<double>("ewkfix_sf", 1.);
        return true;
    };

    double weight()
    {
        return arbol.getLeaf<double>("ewkfix_sf");
    };
};

#endif
