#ifndef CUTS_H
#define CUTS_H

// VBS
#include "core/cuts.h"
// ROOT
#include "TH2.h"

class FillHistograms : public Core::AnalysisCut
{
public:
    TH2D* b_total_hist;
    TH2D* b_loose_hist;
    TH2D* b_medium_hist;
    TH2D* b_tight_hist;
    TH2D* c_total_hist;
    TH2D* c_loose_hist;
    TH2D* c_medium_hist;
    TH2D* c_tight_hist;
    TH2D* light_total_hist;
    TH2D* light_loose_hist;
    TH2D* light_medium_hist;
    TH2D* light_tight_hist;

    FillHistograms(std::string name, Core::Analysis& analysis) : Core::AnalysisCut(name, analysis) 
    {
        TH2D* hist_base = new TH2D("hist", "hist", 10, 30, 300, 4, 0, 2.5);
        b_total_hist = (TH2D*)hist_base->Clone("n_b_total");
        b_loose_hist = (TH2D*)hist_base->Clone("n_b_loose");
        b_medium_hist = (TH2D*)hist_base->Clone("n_b_medium");
        b_tight_hist = (TH2D*)hist_base->Clone("n_b_tight");
        c_total_hist = (TH2D*)hist_base->Clone("n_c_total");
        c_loose_hist = (TH2D*)hist_base->Clone("n_c_loose");
        c_medium_hist = (TH2D*)hist_base->Clone("n_c_medium");
        c_tight_hist = (TH2D*)hist_base->Clone("n_c_tight");
        light_total_hist = (TH2D*)hist_base->Clone("n_light_total");
        light_loose_hist = (TH2D*)hist_base->Clone("n_light_loose");
        light_medium_hist = (TH2D*)hist_base->Clone("n_light_medium");
        light_tight_hist = (TH2D*)hist_base->Clone("n_light_tight");
    };

    bool evaluate()
    {
        LorentzVectors good_jet_p4s = globals.getVal<LorentzVectors>("good_jet_p4s");
        Integers good_jet_idxs = globals.getVal<Integers>("good_jet_idxs");
        for (unsigned int good_jet_i = 0; good_jet_i < good_jet_p4s.size(); ++good_jet_i)
        {
            LorentzVector jet_p4 = good_jet_p4s.at(good_jet_i);
            double jet_abseta = fabs(jet_p4.eta());
            if (jet_abseta >= 2.4) { continue; }
            double jet_pt = std::max(std::min(double(jet_p4.pt()), 299.99), 30.);
            int jet_i = good_jet_idxs.at(good_jet_i);
            // Check DeepJet vs. working points in NanoCORE global config (gconf)
            double deepflav_btag = nt.Jet_btagDeepFlavB().at(jet_i);
            bool is_btagged_loose = deepflav_btag > gconf.WP_DeepFlav_loose;
            bool is_btagged_medium = deepflav_btag > gconf.WP_DeepFlav_medium;
            bool is_btagged_tight = deepflav_btag > gconf.WP_DeepFlav_tight;
            switch (nt.Jet_hadronFlavour().at(jet_i))
            {
            case 0:
                light_total_hist->Fill(jet_pt, jet_abseta);
                if (is_btagged_loose) { light_loose_hist->Fill(jet_pt, jet_abseta); }
                if (is_btagged_medium) { light_medium_hist->Fill(jet_pt, jet_abseta); }
                if (is_btagged_tight) { light_tight_hist->Fill(jet_pt, jet_abseta); }
                break;
            case 4:
                c_total_hist->Fill(jet_pt, jet_abseta);
                if (is_btagged_loose) { c_loose_hist->Fill(jet_pt, jet_abseta); }
                if (is_btagged_medium) { c_medium_hist->Fill(jet_pt, jet_abseta); }
                if (is_btagged_tight) { c_tight_hist->Fill(jet_pt, jet_abseta); }
                break;
            case 5:
                b_total_hist->Fill(jet_pt, jet_abseta);
                if (is_btagged_loose) { b_loose_hist->Fill(jet_pt, jet_abseta); }
                if (is_btagged_medium) { b_medium_hist->Fill(jet_pt, jet_abseta); }
                if (is_btagged_tight) { b_tight_hist->Fill(jet_pt, jet_abseta); }
                break;
            }
        }
        return true;
    };

    void writeHists()
    {
        // Divide histograms
        TH2D* b_loose_eff_hist = (TH2D*)b_loose_hist->Clone("deepjet_eff_b_loose");
        TH2D* b_medium_eff_hist = (TH2D*)b_medium_hist->Clone("deepjet_eff_b_medium");
        TH2D* b_tight_eff_hist = (TH2D*)b_tight_hist->Clone("deepjet_eff_b_tight");
        TH2D* c_loose_eff_hist = (TH2D*)c_loose_hist->Clone("deepjet_eff_c_loose");
        TH2D* c_medium_eff_hist = (TH2D*)c_medium_hist->Clone("deepjet_eff_c_medium");
        TH2D* c_tight_eff_hist = (TH2D*)c_tight_hist->Clone("deepjet_eff_c_tight");
        TH2D* light_loose_eff_hist = (TH2D*)light_loose_hist->Clone("deepjet_eff_light_loose");
        TH2D* light_medium_eff_hist = (TH2D*)light_medium_hist->Clone("deepjet_eff_light_medium");
        TH2D* light_tight_eff_hist = (TH2D*)light_tight_hist->Clone("deepjet_eff_light_tight");
        b_loose_eff_hist->Divide(b_total_hist);
        b_medium_eff_hist->Divide(b_total_hist);
        b_tight_eff_hist->Divide(b_total_hist);
        c_loose_eff_hist->Divide(c_total_hist);
        c_medium_eff_hist->Divide(c_total_hist);
        c_tight_eff_hist->Divide(c_total_hist);
        light_loose_eff_hist->Divide(light_total_hist);
        light_medium_eff_hist->Divide(light_total_hist);
        light_tight_eff_hist->Divide(light_total_hist);
        // Write histograms
        b_loose_eff_hist->Write();
        b_medium_eff_hist->Write();
        b_tight_eff_hist->Write();
        c_loose_eff_hist->Write();
        c_medium_eff_hist->Write();
        c_tight_eff_hist->Write();
        light_loose_eff_hist->Write();
        light_medium_eff_hist->Write();
        light_tight_eff_hist->Write();
    };
};

#endif
