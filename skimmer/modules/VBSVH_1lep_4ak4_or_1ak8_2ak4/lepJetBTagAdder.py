## Original Version:
## https://github.com/sscruz/cmgtools-lite/blob/104X_dev_nano_lepMVA/TTHAnalysis/python/tools/nanoAOD/lepJetBTagAdder.py

from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module
from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection 

class lepJetBTagAdder( Module ):
    def __init__(self,jetBTagLabel,lepBTagLabel, dummyValue=-99):
        self._jetBTagLabel = jetBTagLabel
        self._lepBTagLabel = lepBTagLabel
        self._dummyValue = dummyValue
    def beginFile(self, inputFile, outputFile, inputTree, wrappedOutputTree):
        self.out = wrappedOutputTree
        self.out.branch("Muon_" + self._lepBTagLabel, "F", lenVar="nMuon")
        self.out.branch("Electron_" + self._lepBTagLabel, "F", lenVar="nElectron")
    def analyze(self, event):
        elec = Collection(event, 'Electron')
        muos = Collection(event, 'Muon')

        jets = Collection(event, 'Jet')
        nJets = len(jets)

        values = []
        for lep in elec:
            if lep.jetIdx >= 0 and lep.jetIdx < nJets:
                values.append(getattr(jets[lep.jetIdx], self._jetBTagLabel))
            else:
                values.append(self._dummyValue)
        self.out.fillBranch("Electron_" + self._lepBTagLabel, values)

        values = []
        for lep in muos:
            if lep.jetIdx >= 0 and lep.jetIdx < nJets:
                values.append(getattr(jets[lep.jetIdx], self._jetBTagLabel))
            else:
                values.append(self._dummyValue)
        self.out.fillBranch("Muon_" + self._lepBTagLabel, values)


        return True

lepJetBTagCSV = lambda : lepJetBTagAdder("btagCSVV2", "jetBTagCSV")
lepJetBTagDeepCSV = lambda : lepJetBTagAdder("btagDeepB", "jetBTagDeepCSV")
lepJetBTagDeepFlav = lambda : lepJetBTagAdder("btagDeepFlavB", "jetBTagDeepFlav")
lepJetBTagDeepFlavC = lambda : lepJetBTagAdder("btagDeepFlavC", "jetBTagDeepFlavC")
