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

    def isMuonTTHIDVeto(self, i):
        return ROOT.ttH.muonID(i, ROOT.ttH.IDveto, ROOT.nt.year())

    def isElectronTTHIDVeto(self, i):
        return ROOT.ttH.electronID(i, ROOT.ttH.IDveto, ROOT.nt.year())

    def passSkim_3l(self, event):
        """>= 3 lepton(=e, mu only) skim"""
        # print(event._entry)
        ROOT.nt.GetEntry(event._entry)
        electrons = Collection(event, "Electron")
        muons = Collection(event, "Muon")

        # list to hold the loose leptons with pt > 10 GeV
        charges_veto = []
        leptons_veto = []

        # Looping over muons
        nmuons_veto = 0
        for i, lep in enumerate(muons):

            # Check that it passes veto Id
            if self.isMuonTTHIDVeto(i):
                nmuons_veto += 1
                charges_veto.append(lep.charge)
                leptons_veto.append(ROOT.nt.Muon_p4()[i])

        # Loop over the electrons
        nelectrons_veto = 0
        for i, lep in enumerate(electrons):

            # check that if passes loose
            if self.isElectronTTHIDVeto(i):
                nelectrons_veto += 1
                charges_veto.append(lep.charge)
                leptons_veto.append(ROOT.nt.Electron_p4()[i])

        if nelectrons_veto + nmuons_veto >= 3:
            return True
        else:
            return False

    def analyze(self, event):
        """process event, return True (go to next module) or False (fail, go to next event)"""

        if self.passSkim_3l(event):
            return True
        else:
            return False

# define modules using the syntax 'name = lambda : constructor' to avoid having them loaded when not needed

skimModuleConstr = lambda: skimProducer()
