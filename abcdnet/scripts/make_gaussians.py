import os
import uproot
import numpy as np
from tqdm import tqdm

def write_gaussian3D(n_events, mean, covariance, outfile):
    # Generate random numbers following a multivariate gaussian distribution
    rng = np.random.default_rng()
    multigauss = rng.multivariate_normal(mean, covariance, n_events)

    with uproot.recreate(outfile) as f:
        f["tree"] = {
            "X0": multigauss.T[0],
            "X1": multigauss.T[1],
            "X2": multigauss.T[2]
        }

    print(f"Wrote {outfile}")

if __name__ == "__main__":

    n_events = 2000000
    outdir = f"/blue/p.chang/{os.environ['USER']}/data/vbsvvh/gaussiansPosCov"
    os.makedirs(outdir, exist_ok=True)

    mean_bkg = np.array([0, 0, 0])
    variance_bkg = 1.5
    covariance_bkg = np.array([
        [1.0, 0.8, 0.0],
        [0.8, 1.0, 0.0],
        [0.0, 0.0, 1.0]
    ])*(variance_bkg**2)
    write_gaussian3D(n_events, mean_bkg, covariance_bkg, f"{outdir}/bkg.root")

    mean_sig = np.array([2.5, 2.5, 2])
    variance_sig = 1.5
    covariance_sig = np.array([
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1]
    ])*(variance_sig)**2
    write_gaussian3D(n_events, mean_sig, covariance_sig, f"{outdir}/sig.root")
