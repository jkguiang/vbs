from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection
from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

ROOT.gROOT.SetBatch(True)

class skimProducer(Module):
    def __init__(self):
        print("Loading NanoCORE shared libraries...")
        ROOT.gSystem.Load("NanoTools/NanoCORE/libNANO_CORE.so")
        header_files = ["ElectronSelections", "MuonSelections", "TauSelections", "Config"]
        for header_file in header_files:
            print("Loading NanoCORE {} header file...".format(header_file))
            ROOT.gROOT.ProcessLine(".L NanoTools/NanoCORE/{}.h".format(header_file))

    def beginJob(self):
        pass

    def endJob(self):
        pass

    def beginFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        self._tchain = ROOT.TChain("Events")
        self._tchain.Add(inputFile.GetName())
        print(inputFile)
        if "UL16" in inputFile.GetName():
            ROOT.gconf.nanoAOD_ver = 8
        if "UL17" in inputFile.GetName():
            ROOT.gconf.nanoAOD_ver = 8
        if "UL18" in inputFile.GetName():
            ROOT.gconf.nanoAOD_ver = 8
        ROOT.nt.Init(self._tchain)
        ROOT.gconf.GetConfigs(ROOT.nt.year())
        print("year = {}".format(ROOT.nt.year()))
        print("WP_DeepFlav_loose = {}".format(ROOT.gconf.WP_DeepFlav_loose))
        print("WP_DeepFlav_medium = {}".format(ROOT.gconf.WP_DeepFlav_medium))
        print("WP_DeepFlav_tight = {}".format(ROOT.gconf.WP_DeepFlav_tight))
        pass

    def endFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        pass

    def analyze(self, event):
        """process event, return True (go to next module) or False (fail, go to next event)"""
        # print(event._entry)
        ROOT.nt.GetEntry(event._entry)
        electrons = Collection(event, "Electron")
        muons = Collection(event, "Muon")
        taus = Collection(event, "Tau")
        jets = Collection(event, "Jet")

        doSSID = False
        dottHID = not doSSID

        # list to hold the tight leptons with pt > 35 GeV (in the analysis we use 40 GeV but we want to keep it slightly loose for skimming)
        charges = []
        leptons = []
        # list to hold the loose leptons with pt > 10 GeV (for tau maybe a little different)
        charges_veto = []
        leptons_veto = []
        leptons_veto_jetIdx = []
        taus_veto = []

        # Loop over the muons to select the leptons
        nmuons_veto = 0
        nmuons_35 = 0
        for i, lep in enumerate(muons):

            if doSSID:

                # Check that it passes veto Id
                if ROOT.SS.muonID(i, ROOT.SS.IDveto, ROOT.nt.year()):
                    nmuons_veto += 1
                    charges_veto.append(lep.charge)
                    leptons_veto.append(ROOT.nt.Muon_p4()[i])
                    leptons_veto_jetIdx.append(ROOT.nt.Muon_jetIdx()[i])

                    # Then, if it passes tight ID save again
                    if lep.pt > 35. and ROOT.SS.muonID(i, ROOT.SS.IDtight, ROOT.nt.year()):
                        nmuons_35 += 1
                        charges.append(lep.charge)
                        leptons.append(ROOT.nt.Muon_p4()[i])

            elif dottHID:

                # Check that it passes veto Id
                if ROOT.ttH.muonID(i, ROOT.ttH.IDveto, ROOT.nt.year()):
                    nmuons_veto += 1
                    charges_veto.append(lep.charge)
                    leptons_veto.append(ROOT.nt.Muon_p4()[i])
                    leptons_veto_jetIdx.append(ROOT.nt.Muon_jetIdx()[i])

                    # Then, if it passes tight ID save again
                    if lep.pt > 35. and ROOT.ttH.muonID(i, ROOT.ttH.IDtight, ROOT.nt.year()):
                        nmuons_35 += 1
                        charges.append(lep.charge)
                        leptons.append(ROOT.nt.Muon_p4()[i])

        # Loop over the electrons
        nelectrons_veto = 0
        nelectrons_35 = 0
        for i, lep in enumerate(electrons):

            if doSSID:

                # check that if passes loose
                if ROOT.SS.electronID(i, ROOT.SS.IDveto, ROOT.nt.year()):
                    nelectrons_veto += 1
                    charges_veto.append(lep.charge)
                    leptons_veto.append(ROOT.nt.Electron_p4()[i])
                    leptons_veto_jetIdx.append(ROOT.nt.Electron_jetIdx()[i])

                    # If it passes tight save to the list
                    if lep.pt > 35. and ROOT.SS.electronID(i, ROOT.SS.IDtight, ROOT.nt.year()):
                        nelectrons_35 += 1
                        charges.append(lep.charge)
                        leptons.append(ROOT.nt.Electron_p4()[i])

            elif dottHID:

                # check that if passes loose
                if ROOT.ttH.electronID(i, ROOT.ttH.IDveto, ROOT.nt.year()):
                    nelectrons_veto += 1
                    charges_veto.append(lep.charge)
                    leptons_veto.append(ROOT.nt.Electron_p4()[i])
                    leptons_veto_jetIdx.append(ROOT.nt.Electron_jetIdx()[i])

                    # If it passes tight save to the list
                    if lep.pt > 35. and ROOT.ttH.electronID(i, ROOT.ttH.IDtight, ROOT.nt.year()):
                        nelectrons_35 += 1
                        charges.append(lep.charge)
                        leptons.append(ROOT.nt.Electron_p4()[i])

        # Loop over the taus
        ntaus_veto = 0
        ntaus_35 = 0
        for i, lep in enumerate(taus):

            if doSSID:

                # check that it passes loose
                if ROOT.SS.tauID(i, ROOT.SS.IDfakable, ROOT.nt.year()):
                    # tau-(non-tau lep) overlap removal
                    # Basically we give precedence to electrons/muons
                    save_this_tau = True
                    for lep_veto in leptons_veto:
                        dr = ROOT.Math.VectorUtil.DeltaR(ROOT.nt.Tau_p4()[i], lep_veto)
                        if (dr < 0.4):
                            save_this_tau = False
                            break

                    if not save_this_tau:
                        continue

                    ntaus_veto += 1
                    charges_veto.append(lep.charge)
                    taus_veto.append(ROOT.nt.Tau_p4()[i])

                    # Check that it passes the tight
                    if lep.pt > 35. and ROOT.SS.tauID(i, ROOT.SS.IDtight, ROOT.nt.year()):
                        ntaus_35 += 1
                        charges.append(lep.charge)
                        leptons.append(ROOT.nt.Tau_p4()[i])

            elif dottHID:

                # check that it passes loose
                if ROOT.ttH.tauID(i, ROOT.ttH.IDveto, ROOT.nt.year()):
                    # tau-(non-tau lep) overlap removal
                    # Basically we give precedence to electrons/muons
                    save_this_tau = True
                    for lep_veto in leptons_veto:
                        dr = ROOT.Math.VectorUtil.DeltaR(ROOT.nt.Tau_p4()[i], lep_veto)
                        if (dr < 0.4):
                            save_this_tau = False
                            break

                    if not save_this_tau:
                        continue

                    ntaus_veto += 1
                    charges_veto.append(lep.charge)
                    taus_veto.append(ROOT.nt.Tau_p4()[i])

                    # Check that it passes the tight
                    if lep.pt > 35. and ROOT.ttH.tauID(i, ROOT.ttH.IDtight, ROOT.nt.year()):
                        ntaus_35 += 1
                        charges.append(lep.charge)
                        leptons.append(ROOT.nt.Tau_p4()[i])

        #================================================================================================================
        # Check that the number of lepton requirement is met
        if not (nelectrons_veto + nmuons_veto >= 1             ): return False # First check that we have at least one light lepton
        if not (nelectrons_veto + nmuons_veto + ntaus_veto >= 2): return False # Then check that we have greater than or equal two veto leptons
        if not (nelectrons_35 + nmuons_35 + ntaus_35 >= 2      ): return False # Then check that we have at least one tight lepton
        if (nelectrons_veto + nmuons_veto + ntaus_veto == 2):
            if not (charges_veto[0] * charges_veto[1] > 0      ): return False # Then if only 2 leptons then check that we have same-sign leptons
        #================================================================================================================

        # Loop over the jets
        njets_20 = 0
        njets_30 = 0
        for i, jet in enumerate(jets):

            # Perform lepton - jet overlap removal
            isOverlap = False
            for ilep_jetidx in leptons_veto_jetIdx:
                if i == ilep_jetidx:
                    isOverlap = True
                    break

            # Perform lepton - jet overlap removal
            for tau in taus_veto:
                if ROOT.Math.VectorUtil.DeltaR(ROOT.nt.Jet_p4()[i], tau) < 0.4:
                    isOverlap = True
                    break

            if isOverlap:
                continue

            # Count jets with pt > 20
            if jet.pt > 20:
                njets_20 += 1

            # Count jets with pt > 30
            if jet.pt > 30:
                njets_30 += 1

        #================================================================================================================
        # Now do some preliminary cut
        # We need to have at least 3 jets of pt > 20 GeV
        if not (njets_20 >= 4):
            return False
        # We need to have at least 2 jets of pt > 30 GeV
        if not (njets_30 >= 2):
            return False
        #================================================================================================================

        return True

# define modules using the syntax 'name = lambda : constructor' to avoid having them loaded when not needed

skimModuleConstr = lambda: skimProducer()
