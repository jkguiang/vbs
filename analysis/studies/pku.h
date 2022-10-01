#ifndef PKU_H
#define PKU_H

// NanoCORE
#include "Nano.h"

namespace PKU
{

enum IDLevel 
{
    IDveto = 0,
    IDtight = 1
};

bool passesElecID(unsigned int elec_i, IDLevel id_level)
{
    /* Peking U. Tight ID
       tight_electrons = events.Electron[
           (events.Electron.pt > 35) 
           & (events.Electron.cutBased >= 3) 
           & (events.Electron.eta + events.Electron.deltaEtaSC < 2.5) 
           & (((abs(events.Electron.dz) < 0.1) & (abs(events.Electron.dxy) < 0.05) 
               & (events.Electron.eta + events.Electron.deltaEtaSC < 1.479)) 
              | ((abs(events.Electron.dz) < 0.2) & (abs(events.Electron.dxy) < 0.1) 
                   & (events.Electron.eta + events.Electron.deltaEtaSC > 1.479)))
       ]
    */
    if (nt.Electron_pt().at(elec_i) <= 10) { return false; }
    if (nt.Electron_cutBased().at(elec_i) < 1) { return false; }
    if (id_level == IDtight)
    {
        if (nt.Electron_pt().at(elec_i) <= 35) { return false; }
        if (nt.Electron_cutBased().at(elec_i) < 3) { return false; }
        if (fabs(nt.Electron_eta().at(elec_i) + nt.Electron_deltaEtaSC().at(elec_i)) >= 2.5) { return false; }
        if (fabs(nt.Electron_eta().at(elec_i) + nt.Electron_deltaEtaSC().at(elec_i)) >= 1.479)
        {
            if (fabs(nt.Electron_dz().at(elec_i)) >= 0.2) { return false; }
            if (fabs(nt.Electron_dxy().at(elec_i)) >= 0.1) { return false; }
        }
        else
        {
            if (fabs(nt.Electron_dz().at(elec_i)) >= 0.1) { return false; }
            if (fabs(nt.Electron_dxy().at(elec_i)) >= 0.05) { return false; }
        }
    }
    return true;
};

bool passesMuonID(unsigned int muon_i, IDLevel id_level)
{
    /* Peking U. Tight ID
       tight_muons = events.Muon[
           events.Muon.tightId 
           & (events.Muon.pfRelIso04_all < 0.15) 
           & (events.Muon.pt > 26) 
           & (abs(events.Muon.eta) < 2.4)
       ]
    */
    if (!nt.Muon_tightId().at(muon_i)) { return false; }
    if (nt.Muon_pfRelIso04_all().at(muon_i) >= 0.4) { return false; }
    if (nt.Muon_pt().at(muon_i) <= 10) { return false; }
    if (id_level == IDtight)
    {
        if (nt.Muon_pfRelIso04_all().at(muon_i) >= 0.15) { return false; }
        if (nt.Muon_pt().at(muon_i) <= 26) { return false; }
        if (fabs(nt.Muon_eta().at(muon_i)) >= 2.4) { return false; }
    }
    return true;
};

};

#endif
