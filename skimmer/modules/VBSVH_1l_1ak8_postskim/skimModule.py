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

    def passPostSkim(self, event):
        ROOT.nt.GetEntry(event._entry)

        electrons = Collection(event, "Electron")
        muons = Collection(event, "Muon")

        n_loose_leps = 0
        n_tight_leps = 0
        lep_p4 = None
        # Loop over muons
        for i, lep in enumerate(muons):
            if ROOT.ttH.muonID(i, ROOT.ttH.IDfakable, ROOT.nt.year()):
                n_loose_leps += 1
            if ROOT.ttH.muonID(i, ROOT.ttH.IDtight, ROOT.nt.year()):
                n_tight_leps += 1
                lep_p4 = ROOT.nt.Muon_p4()[i]
        # Loop over electrons
        for i, lep in enumerate(electrons):
            if ROOT.ttH.electronID(i, ROOT.ttH.IDfakable, ROOT.nt.year()):
                n_loose_leps += 1
            if ROOT.ttH.electronID(i, ROOT.ttH.IDtight, ROOT.nt.year()):
                n_tight_leps += 1
                lep_p4 = ROOT.nt.Electron_p4()[i]

        if not (n_loose_leps == 1 and n_tight_leps == 1):
            return False
        
        # Loop over ak8 jets
        fatjets = Collection(event, "FatJet")
        nfatjets = 0
        for i, fatjet in enumerate(fatjets):
            is_overlap = (ROOT.Math.VectorUtil.DeltaR(ROOT.nt.FatJet_p4()[i], lep_p4) < 0.8)
            if not is_overlap and fatjet.mass > 25 and fatjet.msoftdrop > 25 and fatjet.pt > 250 and fatjet.particleNet_HbbvsQCD > 0.5:
                nfatjets += 1

        return nfatjets >= 1

    def analyze(self, event):
        """process event, return True (go to next module) or False (fail, go to next event)"""

        return self.passPostSkim(event)

# define modules using the syntax 'name = lambda : constructor' to avoid having them loaded when not needed

skimModuleConstr = lambda: skimProducer()
