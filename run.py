import argparse
import glob
import os
import ROOT
from decimal import Decimal
from tools.orchestrator import BabyMakerOrchestrator

class VBSOrchestrator(BabyMakerOrchestrator):
    def __init__(self, year, study, executable, input_files, xsecs_json, n_workers=8):
        self.output_dir = f"studies/{study}/output/{year}"
        os.makedirs(self.output_dir, exist_ok=True)
        super().__init__(executable, input_files, xsecs_json=xsecs_json, n_workers=n_workers)

    def _get_file_info(self, file_name):
        file_info = {
            "is_signal": False,
            "is_data": False,
            "n_events": 0
        }
        if "VBS" in file_name:
            file_info["is_signal"] = True
        if "Run201" in file_name:
            file_info["is_data"] = True
        # Get output info
        file_info["output_dir"] = self.output_dir
        file_info["output_name"] = file_name.split('/')[5]}
        # Get # events
        f = ROOT.TFile(file_name)
        h = f.Get("h_nevents")
        file_info["n_events"] = h.GetBinContent(1)
        return file_info

    def _get_log_files(self, input_file):
        output_name = self._get_file_info(input_file)["output_name"]
        stdout_file = output_name + ".out"
        stderr_file = output_name + ".err"
        return stdout_file, stderr_file

    def _get_job(self, input_file):
        file_info = self._get_file_info(input_file)
        xsec = self.get_xsec(input_file)
        scale1fb = xsec/(file_info["n_events"]*1000)
        cmd = [self.executable]
        cmd.append(f"--input_ttree=Events")
        cmd.append(f"--output_dir={file_info['output_dir']}")
        cmd.append(f"--output_name={file_info['output_name']}")
        cmd.append(f"--scale_factor={Decimal.from_float(scale1fb)}")
        return cmd

if __name__ == "__main__":
    skims = [
        (2016, glob.glob("/nfs-7/userdata/phchang/VBSHWWNanoSkim_v2.6_Excl3L/*UL16*/merged/*.root")),
        (2017, glob.glob("/nfs-7/userdata/phchang/VBSHWWNanoSkim_v2.6_Excl3L/*UL17*/merged/*.root")),
        (2018, glob.glob("/nfs-7/userdata/phchang/VBSHWWNanoSkim_v2.6_Excl3L/*UL18*/merged/*.root"))
    ]
    for year, files in skims:
        print(f"Orchestrating {year} jobs...")
        orchestrator = VBSOrchestrator(
            year, 
            "pilot", 
            "./dummy_executable", 
            files, 
            # xsecs_json="data/xsecs.json",
            xsecs_json="",
            n_workers=8
        )
        orchestrator.run()
