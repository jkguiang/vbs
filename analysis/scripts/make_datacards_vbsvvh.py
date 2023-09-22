import uproot
import os
import glob
import json
import pandas as pd
from warnings import simplefilter
simplefilter(action="ignore", category=pd.errors.PerformanceWarning)
import numpy as np
import itertools
from tqdm import tqdm
from utils.analysis import PandasAnalysis
from utils.systematics import Systematic, SystematicsTable
from utils.cutflow import Cutflow
from utils.datacard import DatacardABCD

def get_year_str(year, doAPV=True):
    if doAPV and year == -2016:
        return "2016preVFP"
    elif doAPV and year == 2016:
        return "2016postVFP"
    else:
        return str(abs(year))

def get_systs(sample_name, signal_regions, sf, *sf_variations, year=None):
    global vsbwh
    systs = []
    for SR in signal_regions:
        df = vbsvvh.sample_df(name=sample_name, selection=SR)
        # Get nominal
        count = df.event_weight.sum()
        # Get delta = nominal - variation for each variation
        deltas = []
        for sf_var in sf_variations:
            if year:
                year_df = df[df.year == year]
                count_var = (
                    np.sum(year_df.event_weight/year_df[sf]*year_df[sf_var])
                    + np.sum(df[df.year != year].event_weight)
                )
            else:
                count_var = np.sum(df.event_weight/df[sf]*df[sf_var])

            if count == 0:
                print(f"WARNING: count == {count} for {sample_name} in {SR}")
                deltas.append(0)
            else:
                deltas.append(abs((count - count_var)/count))
        
        systs.append(max(deltas))
        
    return systs

def get_systs_nonSF(sample_name, signal_regions, signal_regions_up, signal_regions_dn):
    systs = []
    for SR_i, SR in enumerate(signal_regions):
        SR_up = signal_regions_up[SR_i]
        SR_dn = signal_regions_dn[SR_i]
        
        df = vbsvvh.sample_df(name=sample_name, selection=SR)
        df_up = vbsvvh.sample_df(name=sample_name, selection=SR_up)
        df_dn = vbsvvh.sample_df(name=sample_name, selection=SR_dn)
        
        count = df.event_weight.sum()
        count_up = df_up.event_weight.sum()
        count_dn = df_dn.event_weight.sum()
        
        up_perc = abs((count_up - count)/count)
        dn_perc = abs((count - count_dn)/count)
        
        systs.append(max(up_perc, dn_perc))
        
    return systs

def get_jet_energy_systs(nominal_cflow, up_cflow, dn_cflow, signal_regions, name):
    
    nominal_cutflow = Cutflow.from_file(nominal_cflow)
    up_cutflow = Cutflow.from_file(up_cflow)
    dn_cutflow = Cutflow.from_file(dn_cflow)

    syst_up_cutflow = (up_cutflow - nominal_cutflow)/nominal_cutflow
    syst_dn_cutflow = (nominal_cutflow - dn_cutflow)/nominal_cutflow

    systs = Systematic(name, signal_regions.keys())
    for SR, cut_name in signal_regions.items():
        systs.add_syst(
            max(
                syst_up_cutflow[cut_name].n_pass,
                syst_dn_cutflow[cut_name].n_pass
            ),
            signal_region=SR
        )
        
    return systs

TAG = "abcdnet_v1"
BASEDIR = "/data/userdata/jguiang/vbs_studies"

babies = glob.glob(f"{BASEDIR}/vbsvvhjets/output_{TAG}/Run2/inferences/*.root")
babies = [baby for baby in babies if "Lambda" not in baby]
sig_babies = [baby for baby in babies if "VBSVVH" in baby]
bkg_babies = [baby for baby in babies if "VBS" not in baby and "data.root" not in baby]
data_babies = [baby for baby in babies if "data.root" in baby]
print("Signal:")
print("\n".join(sig_babies))
print("Background:")
print("\n".join(bkg_babies))
print("Data:")
print("\n".join(data_babies))

if not sig_babies or not bkg_babies or not data_babies:
    exit()

vbsvvh = PandasAnalysis(
    sig_root_files=sig_babies,
    bkg_root_files=bkg_babies,
    data_root_files=data_babies,
    ttree_name="tree",
    weight_columns=[
        "xsec_sf", 
        "pu_sf",
        "prefire_sf"
    ]
)
vbsvvh.df["unity"] = 1 # IMPORTANT
vbsvvh.df["objsel"] = True
vbsvvh.df["presel"] = vbsvvh.df.eval(
    "objsel and hbbfatjet_xbb > 0.5 and ld_vqqfatjet_xwqq > 0.3 and tr_vqqfatjet_xwqq > 0.3"
)
vbsvvh.make_selection("presel")

bkg_count = vbsvvh.bkg_count()
qcd_count = vbsvvh.sample_count("QCD")
data_count = vbsvvh.data_count()

vbsvvh.df.loc[vbsvvh.df.name == "QCD", "event_weight"] *= (data_count - (bkg_count - qcd_count))/(qcd_count)

vbsvvh.df["orig_event_weight"] = vbsvvh.df.event_weight.values.copy()

vbsvvh.df["abcdnet_cut"] = vbsvvh.df.eval("abcdnet_score > 0.89")
vbsvvh.df["vbs_cut"] = vbsvvh.df.eval("abs_deta_jj > 5.0")
vbsvvh.df["regionA"] = vbsvvh.df.eval(
    "abcdnet_cut and vbs_cut and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and hbbfatjet_xbb > 0.8"
)
vbsvvh.df["regionB"] = vbsvvh.df.eval(
    "abcdnet_cut and (not vbs_cut) and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and hbbfatjet_xbb > 0.8"
)
vbsvvh.df["regionC"] = vbsvvh.df.eval(
    "(not abcdnet_cut) and vbs_cut and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and hbbfatjet_xbb > 0.8"
)
vbsvvh.df["regionD"] = vbsvvh.df.eval(
    "(not abcdnet_cut) and (not vbs_cut) and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and hbbfatjet_xbb > 0.8"
)

ABCD_REGIONS = ["regionA", "regionB", "regionC", "regionD"]

with uproot.open(f"{BASEDIR}/vbsvvhjets/output_{TAG}/Run2/VBSVVH.root") as f:
    reweights = np.stack(f["rwgt_tree"].arrays(library="np")["reweights"])
    n_events, n_rewights = reweights.shape

# Insert trivial weight
reweights = np.insert(reweights, 28, 1, axis=1)

# grep -i "^launch" /path/to/PROCESS_reweight_card.dat | awk -F'=' '{print $2}' > data/PROCESS_reweights.txt
with open("data/VBSVVH_reweights.txt", "r") as f_in:
    reweight_names = f_in.read().splitlines()

output_dir = "../combine/vbsvvh/datacards/VBSVVH"
os.makedirs(output_dir, exist_ok=True)

for reweight_i in tqdm(range(reweights.shape[-1]), desc=f"Writing datacards to {output_dir}"):

    SIG_SYSTS_LIMIT = SystematicsTable(
        samples=[b.split("/")[-1].replace(".root", "") for b in sig_babies]
    )
    vbsvvh.df.loc[vbsvvh.df.is_signal, "event_weight"] = vbsvvh.df[vbsvvh.df.is_signal].orig_event_weight.values*reweights.T[reweight_i]

    # -- PDF uncertainty -------------------------------------------------------------------
    root_files = glob.glob("/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/v2/VBS*/merged.root")
    gen_sum = 0
    pdf_sum = np.zeros(101)
    for root_file in root_files:
        with uproot.open(root_file) as f:
            gen_sums = f["Runs"]["genEventSumw"].array(library="np")
            pdf_sums = f["Runs"]["LHEPdfSumw"].array(library="np")
            missed = np.array([len(s) != 101 for s in pdf_sums])
            reshaped = np.vstack(pdf_sums[~missed])
            pdf_sum += np.dot(gen_sums[~missed], reshaped) + np.sum(gen_sums[missed])
            gen_sum += np.sum(gen_sums)
            
    pdf_ratio = pdf_sum/gen_sum

    with uproot.open(f"{BASEDIR}/vbsvvhjets/output_{TAG}/Run2/VBSVVH.root") as f:
        pdf_df = f.get("pdf_tree").arrays(library="pd")
        
    systs = []
    for R in ABCD_REGIONS:
        sig_df = vbsvvh.sig_df()
        count = np.sum(sig_df[sig_df[R]].event_weight*pdf_df[sig_df[R]].lhe_pdf_0)
        deltas = []
        for i in range(1, 101):
            count_var = np.sum(sig_df[sig_df[R]].event_weight*pdf_df[sig_df[R]][f"lhe_pdf_{i}"])
            deltas.append(count - count_var/pdf_ratio[i])

        deltas = np.array(deltas)
        systs.append(np.sqrt(np.sum(deltas**2))/count)

    pdf_systs = Systematic("PDF variations", ABCD_REGIONS)
    pdf_systs.add_systs(systs)
    SIG_SYSTS_LIMIT.add_row(pdf_systs)
    # --------------------------------------------------------------------------------------



    # -- LHE scale weights -----------------------------------------------------------------
    lhe_muR_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("muF1p0")])
    lhe_muF_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("muR1p0")])

    muR_systs = Systematic("$\\mu_R$ scale", ABCD_REGIONS)
    muR_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "unity", *lhe_muR_weights)
    )

    muF_systs = Systematic("$\\mu_F$ scale", ABCD_REGIONS)
    muF_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "unity", *lhe_muF_weights)
    )

    # SIG_SYSTS_LIMIT.add_row(muR_systs)                   # muR variations have not effect
    SIG_SYSTS_LIMIT.add_row(muF_systs)
    # --------------------------------------------------------------------------------------


    # -- Parton shower weights -------------------------------------------------------------
    isr_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("fsr1p0")])
    fsr_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("isr1p0")])

    isr_sf_systs = Systematic("Parton shower ISR weights", ABCD_REGIONS)
    isr_sf_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "unity", *isr_weights)
    )

    fsr_sf_systs = Systematic("Parton shower FSR weights", ABCD_REGIONS)
    fsr_sf_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "unity", *fsr_weights)
    )

    SIG_SYSTS_LIMIT.add_row(isr_sf_systs)
    SIG_SYSTS_LIMIT.add_row(fsr_sf_systs)
    # --------------------------------------------------------------------------------------


    # -- Pileup reweighting ----------------------------------------------------------------
    pu_sf_systs = Systematic("Pileup reweighting", ABCD_REGIONS)
    pu_sf_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "pu_sf", "pu_sf_dn", "pu_sf_up")
    )
    SIG_SYSTS_LIMIT.add_row(pu_sf_systs)
    # --------------------------------------------------------------------------------------


    # -- Pileup jet id ---------------------------------------------------------------------
    puid_sf_systs = Systematic("Pileup jet ID", ABCD_REGIONS)
    puid_sf_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "puid_sf", "puid_sf_dn", "puid_sf_up")
    )
    SIG_SYSTS_LIMIT.add_row(puid_sf_systs)
    # --------------------------------------------------------------------------------------


    # -- L1 prefiring weight ---------------------------------------------------------------
    prefire_sf_systs = Systematic("L1 pre-fire corrections", ABCD_REGIONS)
    prefire_sf_systs.add_systs(
        get_systs("VBSVVH", ABCD_REGIONS, "prefire_sf", "prefire_sf_up", "prefire_sf_dn")
    )
    SIG_SYSTS_LIMIT.add_row(prefire_sf_systs)
    # --------------------------------------------------------------------------------------


    # -- MC statistical uncertainty --------------------------------------------------------
    stat_systs = Systematic("Simulation stat. unc.", ABCD_REGIONS)
    stat_systs.add_systs(
        [
            vbsvvh.sig_error(selection=R)/vbsvvh.sig_count(selection=R) for R in ABCD_REGIONS
        ]
    )
    SIG_SYSTS_LIMIT.add_row(stat_systs)
    # --------------------------------------------------------------------------------------


    # -- ParticleNet Xbb scale factors -----------------------------------------------------
    # FIXME: add these!
    # --------------------------------------------------------------------------------------


    # -- Jet energy correction uncertainty -------------------------------------------------
    jec_systs = get_jet_energy_systs(
        f"../analysis/studies/vbsvvhjets/output_{TAG}/Run2/VBSVVH_cutflow_ABCD.cflow",
        f"../analysis/studies/vbsvvhjets/output_{TAG}_jec_up/Run2/VBSVVH_cutflow_ABCD.cflow",
        f"../analysis/studies/vbsvvhjets/output_{TAG}_jec_dn/Run2/VBSVVH_cutflow_ABCD.cflow",
        {
            "regionA": "AllMerged_RegionA",
            "regionB": "AllMerged_RegionB",
            "regionC": "AllMerged_RegionC",
            "regionD": "AllMerged_RegionD",
        },
        "Jet energy scale"
    )
    SIG_SYSTS_LIMIT.add_row(jec_systs)
    # --------------------------------------------------------------------------------------


    # -- Jet energy resolution uncertainty -------------------------------------------------
    jer_systs = get_jet_energy_systs(
        f"../analysis/studies/vbsvvhjets/output_{TAG}/Run2/VBSVVH_cutflow_ABCD.cflow",
        f"../analysis/studies/vbsvvhjets/output_{TAG}_jer_up/Run2/VBSVVH_cutflow_ABCD.cflow",
        f"../analysis/studies/vbsvvhjets/output_{TAG}_jer_dn/Run2/VBSVVH_cutflow_ABCD.cflow",
        {
            "regionA": "AllMerged_RegionA",
            "regionB": "AllMerged_RegionB",
            "regionC": "AllMerged_RegionC",
            "regionD": "AllMerged_RegionD",
        },
        "Jet energy resolution"
    )
    SIG_SYSTS_LIMIT.add_row(jer_systs)
    # --------------------------------------------------------------------------------------


    # -- Luminosity ------------------------------------------------------------------------
    lumi_systs = Systematic("Luminosity", ABCD_REGIONS)
    lumi_systs.add_systs([0.016 for R in ABCD_REGIONS])
    SIG_SYSTS_LIMIT.add_row(lumi_systs)
    # --------------------------------------------------------------------------------------


    # -- H to bb BR ------------------------------------------------------------------------
    hbb_br_systs = Systematic("\\Htobb BR", ABCD_REGIONS)
    hbb_br_systs.add_systs([0.0127 for R in ABCD_REGIONS])
    SIG_SYSTS_LIMIT.add_row(hbb_br_systs)
    # --------------------------------------------------------------------------------------


    datacard_systs = {
        "TotalBkg": {
            "CMS_vbsvvhjets_abcd_syst": [1 + 25.4/100],
            "CMS_vbsvvhjets_abcd_stat": [1 + 34.0/100]
        },
        "TotalSig": {}
    }

    for syst_obj in SIG_SYSTS_LIMIT.systs:
        systs = syst_obj.get_systs()
        datacard_systs["TotalSig"][syst_obj.name] = [1 + systs[R][0] for R in ABCD_REGIONS]

    ABCD_yields = [round(vbsvvh.bkg_count(selection=ABCD_REGIONS[0])), *[vbsvvh.data_count(selection=R) for R in ABCD_REGIONS[1:]]]

    datacard = DatacardABCD(
        ABCD_yields, # dummy value for observed in region A
        {"TotalSig": [vbsvvh.sig_count(selection=R) for R in ABCD_REGIONS]},
        # {"TotalBkg": [vbsvvh.bkg_count(selection=R) for R in ABCD_REGIONS]},
        {"TotalBkg": [1.0 for R in ABCD_REGIONS]},
        datacard_systs
    )
    datacard.fill()

    reweight_name = reweight_names[reweight_i]
    datacard.write(f"{output_dir}/{reweight_name}.dat")
