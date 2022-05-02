#ifndef CORE_H
#define CORE_H

// RAPIDO
#include "arbol.h"
#include "looper.h"
#include "cutflow.h"
#include "utilities.h"
// NanoCORE
#include "Nano.h"

typedef std::vector<LorentzVector> LorentzVectors;
typedef std::vector<int> Integers;
typedef std::vector<unsigned int> Indices;

enum DeepJetTag
{
    NoTag = 0,
    LooseTag = 1,
    MediumTag = 2,
    TightTag = 3
};
typedef std::vector<DeepJetTag> DeepJetTags;

struct VBSWHAnalysis
{
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Cutflow& cutflow;

    VBSWHAnalysis(Arbol& arbol_ref, Nano& nt_ref, HEPCLI& cli_ref, Cutflow& cutflow_ref) 
    : arbol(arbol_ref), nt(nt_ref), cli(cli_ref), cutflow(cutflow_ref)
    {
        // Do nothing
    };
};

class VBSWHCut : public Cut
{
public:
    Arbol& arbol;
    Nano& nt;
    HEPCLI& cli;
    Utilities::Variables& globals;

    VBSWHCut(std::string new_name, VBSWHAnalysis& a) 
    : Cut(new_name), arbol(a.arbol), nt(a.nt), cli(a.cli), globals(a.cutflow.globals)
    {
        // Do nothing
    };
};

#endif
