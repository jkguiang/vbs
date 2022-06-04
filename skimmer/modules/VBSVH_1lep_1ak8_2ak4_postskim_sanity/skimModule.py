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
    
        self.missing_events = [ 
            # Missing from RAPIDO
            13932472,   47872923, 146988487, 105264637, 130851216,  38593547,
            120579460,  63381761,  13272790,   8154638,  31418562,  19352114,
            79175838,   78707236,  34087498,  41088806,  44095960,  61995283,
            62478226,  100513352, 147427542,  34070105, 107802410,  87087771,
            43347159,  138742686,  68564602,  93919191,  57600460, 118133133,
            113422943, 147588651,  85496954,  52913903, 139297953, 140325884,
            50749288,  130697046,
            # Missing from this
            130298786,  90477383, 112696064,  37734046,  38305474,  31526393,
            10021439,  118900901,   2941052,  14543714,   1180118,  77619176,
            112044232,  31685067,  53600390, 113255849,  74702852, 115514661,
            11759130,  126632092,  64075356,  65957371,  54161067, 117859068,
            141135518,   9108043, 121872410,  91185720,  17536806,  93731328,
            135946749, 122616630
        ]

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

        evt = ROOT.nt.event()

        electrons = Collection(event, "Electron")
        muons = Collection(event, "Muon")

        if event in self.missing_events:
            print("-------- START: {} --------".format(evt))
            print("N electrons: {}".format(len(electrons)))
            for i, lep in enumerate(electrons):
                if ROOT.ttH.electronID_DEBUG(i, ROOT.ttH.IDfakable, ROOT.nt.year()):
                    if ROOT.ttH.electronID_DEBUG(i, ROOT.ttH.IDtight, ROOT.nt.year()): print("electron ID: loose && tight")
                    else: print("electron ID: loose && NOT tight")
                else:
                    print("electron ID: veto && NOT loose")
            print("N muons: {}".format(len(muons)))
            for i, lep in enumerate(muons):
                if ROOT.ttH.muonID_DEBUG(i, ROOT.ttH.IDfakable, ROOT.nt.year()):
                    if ROOT.ttH.muonID_DEBUG(i, ROOT.ttH.IDtight, ROOT.nt.year()): print("muon ID: loose && tight")
                    else: print("muon ID: loose && NOT tight")
                else:
                    print("muon ID: veto && NOT loose")
            print("---------- END: {} --------".format(evt))
            self.missing_events.remove(self.missing_events.index(evt))


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
