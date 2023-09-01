import os
import glob
import json
import signal
import hashlib
import argparse
import itertools
import concurrent.futures as futures

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import warnings
warnings.simplefilter(action="ignore", category=pd.errors.PerformanceWarning)
from tqdm import tqdm

from utils.analysis import Optimization

class BruteForceScan:
    def __init__(self):
        self.results = []
        self.df = None
        signal.signal(signal.SIGINT, self.stop())

    @staticmethod
    def get_cuts(cut_config, ineq=">", invert=False):
        cuts = {}
        for var, linspace_args in cut_config.items():
            cuts[var] = []
            for wp in np.linspace(*linspace_args):
                if invert:
                    cuts[var].append(f"(not {var} {ineq} {wp})")
                else:
                    cuts[var].append(f"{var} {ineq} {wp}")
        return cuts

    @staticmethod
    def fom(sig, bkg):
        return sig/np.sqrt(bkg)

    def run_job(self, args):
        job_i, A, B, C, D = args
        A_df = self.df.query(A)
        B_df = self.df.query(B)
        C_df = self.df.query(C)
        D_df = self.df.query(D)

        A_sig = A_df[ A_df.is_signal & ~A_df.is_data].event_weight.sum()
        A_bkg = A_df[~A_df.is_signal & ~A_df.is_data].event_weight.sum()

        B_sig = B_df[ B_df.is_signal & ~B_df.is_data].event_weight.sum()
        B_bkg = B_df[~B_df.is_signal & ~B_df.is_data].event_weight.sum()

        C_sig = C_df[ C_df.is_signal & ~C_df.is_data].event_weight.sum()
        C_bkg = C_df[~C_df.is_signal & ~C_df.is_data].event_weight.sum()

        D_sig = D_df[ D_df.is_signal & ~D_df.is_data].event_weight.sum()
        D_bkg = D_df[~D_df.is_signal & ~D_df.is_data].event_weight.sum()
        
        B_data = len(B_df[~B_df.is_signal & B_df.is_data])
        C_data = len(C_df[~C_df.is_signal & C_df.is_data])
        D_data = len(D_df[~D_df.is_signal & D_df.is_data])

        A_pred = B_data*C_data/D_data

        self.results[job_i] = {
            # Signal region definition
            "region_A": A,
            # Signal yields
            "A_sig": A_sig,
            "B_sig": B_sig,
            "C_sig": C_sig,
            "D_sig": D_sig,
            # Background yields
            "A_bkg": A_bkg,
            "B_bkg": B_bkg,
            "C_bkg": C_bkg,
            "D_bkg": D_bkg,
            # Data yields
            "B_data": B_data,
            "C_data": C_data,
            "D_data": D_data,
            # Background yield in Region A predicted from data
            "A_pred": A_pred,
            # Figure of merit (FOM)
            "fom": self.fom(A_sig, A_pred)
        }

    def run(self, config_json, n_workers=32):

        with open(config_json) as f:
            config = json.load(f)
            config_str = "".join(json.dumps(config).split())
            config_hash = hashlib.sha1(config_str.encode("utf-8"))
            results_csv = f"{config['output_dir']}/results_{config_hash.hexdigest()[:12]}.csv"

        if os.path.exists(results_csv):
            print(f"This config has already been processed: {results_csv}")
            return

        analysis = Optimization(
            sig_root_files=[f"{config['input_dir']}/{f}" for f in config["sig_files"]],
            bkg_root_files=[f"{config['input_dir']}/{f}" for f in config["bkg_files"]],
            data_root_files=[f"{config['input_dir']}/{f}" for f in config["data_files"]],
            ttree_name=config["ttree_name"],
            weight_columns=config["weights"]
        )
        self.df = analysis.df

        x_arm = config["abcd_arms"][0]
        y_arm = config["abcd_arms"][1]
        presel = config["presel"]

        presel_cuts = self.get_cuts(presel)

        A_cuts = {**self.get_cuts(x_arm), **self.get_cuts(y_arm), **presel_cuts}
        B_cuts = {**self.get_cuts(x_arm), **self.get_cuts(y_arm, invert=True), **presel_cuts}
        C_cuts = {**self.get_cuts(x_arm, invert=True), **self.get_cuts(y_arm), **presel_cuts}
        D_cuts = {**self.get_cuts(x_arm, invert=True), **self.get_cuts(y_arm, invert=True), **presel_cuts}

        A_regions = [" and ".join(cuts) for cuts in itertools.product(*A_cuts.values())] # signal regions
        B_regions = [" and ".join(cuts) for cuts in itertools.product(*B_cuts.values())] # anchor regions
        C_regions = [" and ".join(cuts) for cuts in itertools.product(*C_cuts.values())] # control region
        D_regions = [" and ".join(cuts) for cuts in itertools.product(*D_cuts.values())] # control region

        n_regions = len(A_regions)
        self.results = [-999 for i in range(n_regions)]
        jobs = [(job_i, *abcd) for job_i, abcd in enumerate(zip(A_regions, B_regions, C_regions, D_regions))]

        # Execute jobs
        submitted_futures = {}
        with tqdm(total=len(jobs), desc="Executing jobs") as pbar:
            with futures.ThreadPoolExecutor(max_workers=n_workers) as executor:
                submitted_futures = {
                    executor.submit(self.run_job, job): job for job in jobs
                }
                for future in futures.as_completed(submitted_futures):
                    # Update progress bar
                    pbar.update(1)

        results_df = pd.DataFrame(self.results)
        print(results_df)
        results_df.sort_values(by="fom", inplace=True)

        print_cols = ["A_sig", "A_bkg", "B_bkg", "C_bkg", "D_bkg", "A_pred", "B_data", "C_data", "D_data", "fom"]
        print(results_df[(results_df.B_data >= 4) & (results_df.A_pred >= 0.8)][print_cols])

        results_df.to_csv(results_csv, index=False)
        print(f"Wrote {results_csv}")

    def stop(self):
        def sigint_handler(sig, frame):
            for future in self.submitted_futures:
                future.cancel()
            sys.exit(0)
        return sigint_handler

if __name__ == "__main__":
    cli = argparse.ArgumentParser(description="Run brute force scan")
    cli.add_argument(
        "config_json", type=str,
        help="path to configuration .json file"
    )
    cli.add_argument(
        "-n", "--n_workers", type=int, default=1,
        help="number of workers"
    )
    args = cli.parse_args()
    
    bfscan = BruteForceScan()
    bfscan.run(args.config_json, n_workers=args.n_workers)
