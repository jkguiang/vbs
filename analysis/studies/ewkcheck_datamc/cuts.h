#ifndef CUTS_H
#define CUTS_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// VBS WH
#include "../core.h"
#include "../vbswh.h"

class SelectLeptonsNoUL : public Core::SelectLeptons
{
public:
    SelectLeptonsNoUL(std::string name, Core::Analysis& analysis) : Core::SelectLeptons(name, analysis) 
    {
        // Do nothing
    };

    bool passesVetoElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDveto, nt.year());
    };

    bool passesVetoMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDveto, nt.year());
    };
};

class Has1LepNoUL : public VBSWH::Has1Lep
{
public:
    Has1LepNoUL(std::string name, Core::Analysis& analysis) : VBSWH::Has1Lep(name, analysis) 
    {
        // Do nothing
    };

    bool passesLooseElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDfakable, nt.year());
    };

    bool passesTightElecID(int elec_i)
    {
        return ttH::electronID(elec_i, ttH::IDtight, nt.year());
    };

    bool passesLooseMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDfakable, nt.year());
    };

    bool passesTightMuonID(int muon_i)
    {
        return ttH::muonID(muon_i, ttH::IDtight, nt.year());
    };
};

#endif
