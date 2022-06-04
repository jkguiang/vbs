import os
import math
import ROOT
import itertools
from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection
from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module
from PhysicsTools.NanoAODTools.postprocessing.modules.skimmer.mvaTool import *
ROOT.PyConfig.IgnoreCommandLineOptions = True

ROOT.gROOT.SetBatch(True)

elVars = [
     MVAVar("Electron_pt", func=lambda x: x.pt),
     MVAVar("Electron_eta", func=lambda x: x.eta),
     MVAVar("Electron_pfRelIso03_all", func=lambda x: x.pfRelIso03_all),
     MVAVar("Electron_miniPFRelIso_chg", func=lambda x: x.miniPFRelIso_chg),
     MVAVar("Electron_miniRelIsoNeutral := Electron_miniPFRelIso_all - Electron_miniPFRelIso_chg", func=lambda x: x.miniPFRelIso_all - x.miniPFRelIso_chg),
     MVAVar("Electron_jetNDauCharged", func=lambda x: x.jetNDauCharged),
     MVAVar("Electron_jetPtRelv2", func=lambda x: x.jetPtRelv2),
     MVAVar("Electron_jetPtRatio := min(1 / (1 + Electron_jetRelIso), 1.5)", func=lambda x: min(1 / (1 + x.jetRelIso), 1.5)),
     MVAVar("Electron_jetBTagDeepFlavB := Electron_jetIdx > -1 ? Jet_btagDeepFlavB[Electron_jetIdx] : 0", func=lambda x: x.jetBTagDeepFlav),
     MVAVar("Electron_sip3d", func=lambda x: x.sip3d),
     MVAVar("Electron_dxy := log(abs(Electron_dxy))",func=lambda x: math.log(max(1e-10,abs(x.dxy)))),
     MVAVar("Electron_dz  := log(abs(Electron_dz))", func=lambda x: math.log(max(1e-10,abs(x.dz)))),
     MVAVar("Electron_mvaFall17V2noIso", func=lambda x: x.mvaFall17V2noIso)
]

class LeptonMVA:
    def __init__(self, elpath):
        print("Booking {}".format(elpath))

        self.el = MVATool("BDTG", elpath, elVars)

    def __call__(self,lep):
        if   abs(lep.pdgId) == 11: return self.el(lep)
        elif abs(lep.pdgId) == 13: return lep.mvaTTH # for muons we keep the old training since we would lose performance by retraining on top of nano
        else: return -99

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
        # ttH UL lepton ID MVA
        mva_xml_file = "data/leptonMVA/UL20_{}.xml".format(ROOT.nt.year())
        if "APV" in inputFile.GetName():
            mva_xml_file = mva_xml_file.replace(".xml", "APV.xml")
        self.mva = LeptonMVA(mva_xml_file)
        self.wrappedOutputTree = wrappedOutputTree
        self.wrappedOutputTree.branch("Muon_mvaTTHUL", "F", lenVar="nMuon")
        self.wrappedOutputTree.branch("Electron_mvaTTHUL", "F", lenVar="nElectron")
        pass

    def endFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        pass

    def isMuonTTHIDVeto(self, i):
        return ROOT.ttH.muonID(i, ROOT.ttH.IDveto, ROOT.nt.year())

    def isElectronTTHIDVeto(self, i):
        return ROOT.ttH.electronID(i, ROOT.ttH.IDveto, ROOT.nt.year())

    def runTTH_UL_MVA(self, event):
        # Run TTH MVA and save the score
        self.wrappedOutputTree.fillBranch("Muon_mvaTTHUL", [self.mva(lep) for lep in self.muons])
        # Run TTH MVA and save the score
        self.wrappedOutputTree.fillBranch("Electron_mvaTTHUL", [self.mva(lep) for lep in self.electrons])
        pass

    def passSkim(self, event):
        """>= 1 veto lepton(=e, mu only), >= 1 fat (ak8) jet skim"""
        lep_p4s_veto = []
        # Loop over muons
        for i, lep in enumerate(self.muons):
            if self.isMuonTTHIDVeto(i):
                lep_p4s_veto.append(ROOT.nt.Muon_p4()[i])
        # Loop over electrons
        for i, lep in enumerate(self.electrons):
            if self.isElectronTTHIDVeto(i):
                lep_p4s_veto.append(ROOT.nt.Electron_p4()[i])

        # Loop over ak8 jets
        nfatjets = 0
        for i, fatjet in enumerate(self.fatjets):
            is_overlap = False
            for lep_p4 in lep_p4s_veto:
                if ROOT.Math.VectorUtil.DeltaR(ROOT.nt.FatJet_p4()[i], lep_p4) < 0.8:
                    is_overlap = True
                    break
            if not is_overlap and fatjet.mass > 10 and fatjet.msoftdrop > 10 and fatjet.pt > 200:
                nfatjets += 1

        # Loop over ak4 jets
        njets = 0
        for i, jet in enumerate(self.jets):
            is_overlap = False
            for lep_p4 in lep_p4s_veto:
                if ROOT.Math.VectorUtil.DeltaR(ROOT.nt.Jet_p4()[i], lep_p4) < 0.4:
                    is_overlap = True
                    break
            if not is_overlap and jet.pt > 20:
                njets += 1

        if len(lep_p4s_veto) >= 1 and njets >= 2 and nfatjets >= 1:
            return True
        else:
            return False

    def analyze(self, event):
        """process event, return True (go to next module) or False (fail, go to next event)"""
        # NanoCORE
        ROOT.nt.GetEntry(event._entry)
        # Fill collections
        self.muons = Collection(event, "Muon")
        self.electrons = Collection(event, "Electron")
        self.jets = Collection(event, "Jet")
        self.fatjets = Collection(event, "FatJet")
        # Run skim
        if self.passSkim(event):
            # Fill TTH branches
            self.runTTH_UL_MVA(event)
            return True
        else:
            return False

# define modules using the syntax 'name = lambda : constructor' to avoid having them loaded when not needed

skimModuleConstr = lambda: skimProducer()
