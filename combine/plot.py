import uproot
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
plt.rcParams.update({"figure.facecolor": (1,1,1,0)})
import yahist
import mplhep as hep
plt.style.use(hep.style.CMS)

def plot_r_vs_m2deltaLogL(combine_root_file, output_file="result.pdf"):
    """
    Plot profile likelihood ratio as a function of the signal strength (r)
    """
    with uproot.open(combine_root_file) as f:
        points = f.get("limit").arrays(["r", "deltaNLL"], library="np")
        points["deltaNLL"] *= 2

    fig, axes = plt.subplots()

    axes.plot(points["r"], points["deltaNLL"])

    x_high = axes.get_xticks()[-2]
    deltaNLL_r_eq_1 = points["deltaNLL"][np.where(points["r"] == 1)[0][0]]

    axes.axvline(x=1, color="k", linewidth=0.75)
    axes.axhline(y=deltaNLL_r_eq_1, color="k", linewidth=0.75)
    result = r"$X \sigma$".replace("X", f"{np.sqrt(deltaNLL_r_eq_1):.1f}")
    axes.text(x_high, deltaNLL_r_eq_1, result, ha="right", va="bottom")

    axes.axhline(y=25, color="k", linestyle="--", alpha=0.5, linewidth=0.75)
    axes.text(x_high, 25, r"$5 \sigma$", ha="right", va="bottom", alpha=0.5)

    axes.axhline(y=9,  color="k", linestyle="--", alpha=0.25, linewidth=0.75)
    axes.text(x_high, 9, r"$3 \sigma$", ha="right", va="bottom", alpha=0.25)

    axes.axhline(y=1,  color="k", linestyle="--", alpha=0.125, linewidth=0.75)
    axes.text(x_high, 1, r"$1 \sigma$", ha="right", va="bottom", alpha=0.125)

    axes.set_xlabel("r")
    axes.set_ylabel(r"$-2\Delta\log{L}$")

    hep.cms.label(
        "Preliminary",
        data=False,
        lumi=138,
        loc=0,
        ax=axes,
    )

    if output_file:
        fig.savefig(output_file, bbox_inches="tight")

if __name__ == "__main__":
    plot_r_vs_m2deltaLogL(
        "higgsCombineTest.MultiDimFit.mH125.root", 
        output_file="/home/users/jguiang/public_html/vbswh_plots/limits/r_vs_m2deltaLogL.pdf"
    )
