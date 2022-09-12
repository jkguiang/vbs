#include "../core.h"
// RAPIDO
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "tqdm.h"

int main(int argc, char** argv) 
{
    gconf.nanoAOD_ver = 9;

    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli.input_tchain);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name+"_Cutflow");
    cutflow.globals.newVar<LorentzVectors>("veto_lep_p4s");
    cutflow.globals.newVar<LorentzVectors>("loose_lep_p4s");
    cutflow.globals.newVar<LorentzVectors>("tight_lep_p4s");

    // Initialize Arbol
    TFile* output_tfile = new TFile(
        TString(cli.output_dir+"/"+cli.output_name+".root"),
        "RECREATE"
    );
    Arbol arbol = Arbol(output_tfile);
    arbol.newBranch<int>("event");

    Integers missing_events = {
        // Missing from nanoAOD-tools
        130298786, 90477383, 112696064,  37734046,  38305474,  31526393,
        10021439, 118900901,   2941052,  14543714,   1180118,  77619176,
        112044232, 31685067,  53600390, 113255849,  74702852, 115514661,
        11759130, 126632092,  64075356,  65957371,  54161067, 117859068,
        141135518,  9108043, 121872410,  91185720,  17536806,  93731328,
        135946749, 122616630,
        // Missing from this
        13932472,   47872923, 146988487, 105264637, 130851216,  38593547,
        120579460,  63381761,  13272790,   8154638,  31418562,  19352114,
        79175838,   78707236,  34087498,  41088806,  44095960,  61995283,
        62478226,  100513352, 147427542,  34070105, 107802410,  87087771,
        43347159,  138742686,  68564602,  93919191,  57600460, 118133133,
        113422943, 147588651,  85496954,  52913903, 139297953, 140325884,
        50749288,  130697046
    };

    // Exactly 1 lepton
    Cut* lep_bookkeeping = new LambdaCut(
        "LeptonBookkeeping", 
        [&]() 
        { 
            LorentzVectors veto_lep_p4s;
            LorentzVectors loose_lep_p4s;
            LorentzVectors tight_lep_p4s;
            for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
            {
                LorentzVector lep_p4 = nt.Electron_p4().at(elec_i);
                if (ttH::electronID(elec_i, ttH::IDveto, nt.year()))
                {
                    veto_lep_p4s.push_back(lep_p4);
                }
                if (ttH::electronID(elec_i, ttH::IDfakable, nt.year()))
                {
                    loose_lep_p4s.push_back(lep_p4);
                }
                if (ttH::electronID(elec_i, ttH::IDtight, nt.year()))
                {
                    tight_lep_p4s.push_back(lep_p4);
                }
            }
            for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
            {
                LorentzVector lep_p4 = nt.Muon_p4().at(muon_i);
                if (ttH::muonID(muon_i, ttH::IDveto, nt.year()))
                {
                    veto_lep_p4s.push_back(lep_p4);
                }
                if (ttH::muonID(muon_i, ttH::IDfakable, nt.year()))
                {
                    loose_lep_p4s.push_back(lep_p4);
                }
                if (ttH::muonID(muon_i, ttH::IDtight, nt.year()))
                {
                    tight_lep_p4s.push_back(lep_p4);
                }
            }
            cutflow.globals.setVal<LorentzVectors>("veto_lep_p4s", veto_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("loose_lep_p4s", loose_lep_p4s);
            cutflow.globals.setVal<LorentzVectors>("tight_lep_p4s", tight_lep_p4s);
            return true;
        }
    );
    cutflow.setRoot(lep_bookkeeping);

    // Geq1VetoLep
    Cut* geq1vetolep = new LambdaCut(
        "Geq1VetoLep", [&]() { return cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s").size() >= 1; }
    );
    cutflow.insert(lep_bookkeeping->name, geq1vetolep, Right);

    // Geq2Jets
    Cut* geq2jets = new LambdaCut(
        "Geq2Jets", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s");
            int n_jets = 0;
            for (unsigned int jet_i = 0; jet_i < nt.nJet(); jet_i++)
            {
                LorentzVector jet_p4 = nt.Jet_p4().at(jet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, jet_p4) < 0.4)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap && nt.Jet_pt().at(jet_i) > 20)
                {
                    n_jets++;
                }
            }
            return (n_jets >= 2);
        }
    );
    cutflow.insert(geq1vetolep->name, geq2jets, Right);

    // Geq1FatJet
    Cut* geq1fatjet_skim = new LambdaCut(
        "Geq1FatJetNoVetoLepOverlap", 
        [&]() 
        { 
            LorentzVectors lep_p4s = cutflow.globals.getVal<LorentzVectors>("veto_lep_p4s");
            int n_fatjets = 0;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                bool is_overlap = false;
                for (auto lep_p4 : lep_p4s)
                {
                    if (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8)
                    {
                        is_overlap = true;
                        break;
                    }
                }
                if (!is_overlap 
                    && nt.FatJet_mass().at(fatjet_i) > 10 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 10 
                    && nt.FatJet_pt().at(fatjet_i) > 200)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(geq2jets->name, geq1fatjet_skim, Right);

    // Exactly1Lep
    Cut* exactly1tightlep = new LambdaCut(
        "Exactly1TightLep", 
        [&]() 
        { 
            int n_loose_leps = cutflow.globals.getVal<LorentzVectors>("loose_lep_p4s").size();
            int n_tight_leps = cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").size();
            return (n_loose_leps == 1 && n_tight_leps == 1);
        }
    );
    cutflow.insert(geq1fatjet_skim->name, exactly1tightlep, Right);

    // Geq1FatJet
    Cut* geq1fatjet_postskim = new LambdaCut(
        "Geq1FatJetNoTightLepOverlap", 
        [&]() 
        { 
            LorentzVector lep_p4 = cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").at(0);
            int n_fatjets = 0;
            for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
            {
                LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                bool is_overlap = (ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) < 0.8);
                if (!is_overlap 
                    && nt.FatJet_mass().at(fatjet_i) > 25 
                    && nt.FatJet_msoftdrop().at(fatjet_i) > 25 
                    && nt.FatJet_pt().at(fatjet_i) > 250 
                    && nt.FatJet_particleNet_HbbvsQCD().at(fatjet_i) > 0.5)
                {
                    n_fatjets++;
                }
            }
            return (n_fatjets >= 1);
        }
    );
    cutflow.insert(exactly1tightlep->name, geq1fatjet_postskim, Right);

    Cut* update_branches = new LambdaCut(
        "UpdateBranches", 
        [&]() 
        { 
            arbol.setLeaf<int>("event", nt.event());
            return true;
        }
    );
    cutflow.insert(geq1fatjet_postskim->name, update_branches, Right);

    Cut* check_fatjets = new LambdaCut(
        "DumpFatJetKinematics", 
        [&]() 
        { 
            int event = nt.event();
            bool is_missing = false;
            for (unsigned int i = 0; i < missing_events.size(); i++)
            {
                if (event == missing_events.at(i))
                {
                    is_missing = true;
                    missing_events.erase(missing_events.begin() + i);
                    break;
                }
            }
            if (is_missing)
            {
                LorentzVector lep_p4 = cutflow.globals.getVal<LorentzVectors>("tight_lep_p4s").at(0);
                std::cout << "-------- START: " << event << " --------" << std::endl;
                std::cout << "N fat jets: " << nt.nFatJet() << std::endl;
                for (unsigned int fatjet_i = 0; fatjet_i < nt.nFatJet(); fatjet_i++)
                {
                    LorentzVector fatjet_p4 = nt.FatJet_p4().at(fatjet_i);
                    std::cout << "dR(fat jet, lep): " << ROOT::Math::VectorUtil::DeltaR(lep_p4, fatjet_p4) << std::endl;
                    std::cout << "Fat jet mass: " << nt.FatJet_mass().at(fatjet_i) << std::endl;
                    std::cout << "Fat jet msoftdrop: " << nt.FatJet_msoftdrop().at(fatjet_i) << std::endl;
                    std::cout << "Fat jet pt: " << nt.FatJet_pt().at(fatjet_i) << std::endl;
                    std::cout << "Fat jet Hbb score: " << nt.FatJet_particleNet_HbbvsQCD().at(fatjet_i) << std::endl;
                }
                std::cout << "---------- END: " << event << " --------" << std::endl;
            }
            return true;
        }
    );
    cutflow.insert(exactly1tightlep->name, check_fatjets, Right);

    Cut* check_leps = new LambdaCut(
        "DumpLeptonKinematics", 
        [&]() 
        { 
            int event = nt.event();
            bool is_missing = false;
            for (unsigned int i = 0; i < missing_events.size(); i++)
            {
                if (event == missing_events.at(i))
                {
                    is_missing = true;
                    missing_events.erase(missing_events.begin() + i);
                    break;
                }
            }
            if (is_missing)
            {
                std::cout << "-------- START: " << event << " --------" << std::endl;
                std::cout << "N electrons: " << nt.nElectron() << std::endl;
                for (unsigned int elec_i = 0; elec_i < nt.nElectron(); elec_i++)
                {
                    if (ttH::electronID_DEBUG(elec_i, ttH::IDfakable, nt.year()))
                    {
                        if (ttH::electronID_DEBUG(elec_i, ttH::IDtight, nt.year())) { std::cout << "electron ID: loose && tight" << std::endl; }
                        else { std::cout << "electron ID: loose && NOT tight" << std::endl; }
                    }
                    else
                    {
                        std::cout << "electron ID: veto && NOT loose" << std::endl;
                    }
                }
                std::cout << "N muons: " << nt.nMuon() << std::endl;
                for (unsigned int muon_i = 0; muon_i < nt.nMuon(); muon_i++)
                {
                    if (ttH::muonID_DEBUG(muon_i, ttH::IDfakable, nt.year()))
                    {
                        if (ttH::muonID_DEBUG(muon_i, ttH::IDtight, nt.year())) { std::cout << "muon ID: loose && tight" << std::endl; }
                        else { std::cout << "muon ID: loose && NOT tight" << std::endl; }
                    }
                    else
                    {
                        std::cout << "muon ID: veto && NOT loose" << std::endl;
                    }
                }
                std::cout << "---------- END: " << event << " --------" << std::endl;
            }
            return true;
        }
    );
    cutflow.insert(geq1fatjet_skim->name, check_leps, Right);

    // Run looper
    // tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            gconf.GetConfigs(nt.year());
        },
        [&](int entry) 
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // Reset branches and globals
                cutflow.globals.resetVars();
                // Run cutflow
                nt.GetEntry(entry);
                bool passed = cutflow.run("UpdateBranches");
                if (passed) { arbol.fillTTree(); }
                // bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    cutflow.print();
    cutflow.write(cli.output_dir);
    arbol.writeTFile();
    return 0;
}
