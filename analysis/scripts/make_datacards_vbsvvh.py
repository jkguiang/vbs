import os
import argparse
import uproot
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

if __name__ == "__main__":
    cli = argparse.ArgumentParser(description="Run a given study in parallel")
    cli.add_argument("tag", type=str, help="Tag of the study output (studies/vbsvvhjets/output_{TAG})")
    cli.add_argument("--private", action="store_true", help="Make datacards from private signal samples")
    cli.add_argument("--allmerged", action="store_true", help="Make datacards for all-merged analysis")
    cli.add_argument("--semimerged", action="store_true", help="Make datacards for semi-merged analysis")
    args = cli.parse_args()

    CHANNEL = ""
    if args.allmerged and args.semimerged:
        print("ERROR: can only do allmerged OR semimerged at one time, be patient!")
        exit()
    elif args.semimerged:
        CHANNEL = "semimerged"
    else:
        CHANNEL = "allmerged"

    BABYDIR = f"/data/userdata/{os.getenv('USER')}/vbs_studies/vbsvvhjets/output_{args.tag}"

    if args.private:
        CENTRAL_RWGT = 84
        SIGNAL_NAME = "Private_C2W_C2Z"
    else:
        CENTRAL_RWGT = 28
        SIGNAL_NAME = "VBSVVH"

    output_dir = f"../combine/vbsvvh/datacards/{SIGNAL_NAME}_{CHANNEL}_{args.tag}"
    os.makedirs(output_dir, exist_ok=True)

    # Find babies
    babies = glob.glob(f"{BABYDIR}/Run2/*.root")
    sig_babies = []
    bkg_babies = []
    data_babies = []
    for baby_path in babies:
        baby_name = baby_path.split("/")[-1]
        if SIGNAL_NAME in baby_name:
            sig_babies.append(baby_path)
        elif "VBS" not in baby_name and "Private" not in baby_name:
            if "data" in baby_name:
                data_babies.append(baby_path)
            else:
                bkg_babies.append(baby_path)
    print("Signal:")
    print("\n".join(sig_babies))
    print("Background:")
    print("\n".join(bkg_babies))
    print("Data:")
    print("\n".join(data_babies))

    if not sig_babies or not bkg_babies or not data_babies:
        print(f"Missing some (or all) babies! Looking for {BABYDIR}/Run2/*.root")
        exit()

    # Load babies
    vbsvvh = PandasAnalysis(
        sig_root_files=sig_babies,
        bkg_root_files=bkg_babies,
        data_root_files=data_babies,
        ttree_name="tree",
        weight_columns=[
            "xsec_sf", 
            "pu_sf",
            "prefire_sf",
            "xwqq_ld_vqq_sf",
            "xwqq_tr_vqq_sf",
            "xbb_sf"
        ]
    )
    vbsvvh.df["unity"] = 1 # IMPORTANT
    vbsvvh.df["objsel"] = vbsvvh.df.eval(f"is_{CHANNEL}")
    if CHANNEL == "allmerged":
        vbsvvh.df["presel"] = vbsvvh.df.eval(
            "objsel and hbbfatjet_xbb > 0.5 and ld_vqqfatjet_xwqq > 0.3 and tr_vqqfatjet_xwqq > 0.3"
        )
    elif CHANNEL == "semimerged":
        vbsvvh.df["presel"] = vbsvvh.df.eval(
            "objsel and hbbfatjet_xbb > 0.8 and ld_vqqfatjet_xwqq > 0.6 and ld_vqqfatjet_xvqq > 0.6"
        )

    # Apply the Preselection => all code below is run only on events passing the Preselection!
    vbsvvh.make_selection("presel")

    vbsvvh.df["orig_event_weight"] = vbsvvh.df.event_weight.values.copy()

    if CHANNEL == "allmerged":
        # Re-normalize QCD yield
        bkg_count = vbsvvh.bkg_count()
        qcd_count = vbsvvh.sample_count("QCD")
        data_count = vbsvvh.data_count()
        vbsvvh.df.loc[vbsvvh.df.name == "QCD", "event_weight"] *= (data_count - (bkg_count - qcd_count))/(qcd_count)
        # Define ABCD cuts
        vbsvvh.df["abcdnet_cut"] = vbsvvh.df.eval("abcdnet_score > 0.89")
        vbsvvh.df["vbs_cut"] = vbsvvh.df.eval("abs_deta_jj > 5.0")
        vbsvvh.df["abcd_presel"] = vbsvvh.df.eval("ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and hbbfatjet_xbb > 0.8")
    elif CHANNEL == "semimerged":
        # Define ABCD cuts
        vbsvvh.df["abcdnet_cut"] = vbsvvh.df.eval("abcdnet_score > 0.976")
        vbsvvh.df["vbs_cut"] = vbsvvh.df.eval("abs_deta_jj > 6.0 and M_jj > 1400")
        vbsvvh.df["abcd_presel"] = vbsvvh.df.eval("presel")

    # Define ABCD regions
    vbsvvh.df["regionA"] = vbsvvh.df.eval(
        "abcdnet_cut and vbs_cut and abcd_presel"
    )
    vbsvvh.df["regionB"] = vbsvvh.df.eval(
        "abcdnet_cut and (not vbs_cut) and abcd_presel"
    )
    vbsvvh.df["regionC"] = vbsvvh.df.eval(
        "(not abcdnet_cut) and vbs_cut and abcd_presel"
    )
    vbsvvh.df["regionD"] = vbsvvh.df.eval(
        "(not abcdnet_cut) and (not vbs_cut) and abcd_presel"
    )
    ABCD_REGIONS = ["regionA", "regionB", "regionC", "regionD"]

    # Load reweights
    with uproot.open(f"{BABYDIR}/Run2/{SIGNAL_NAME}.root") as f:
        rwgt_tree = f["rwgt_tree"].arrays(library="np")
        reweights = np.stack(rwgt_tree["reweights"][rwgt_tree[f"is_{CHANNEL}"]])
        # Insert trivial reweight for central value (C2V = 2)
        reweights = np.insert(reweights, CENTRAL_RWGT, 1, axis=1)
        # Interpret reweights matrix shape
        n_events, n_reweights = reweights.shape
    # grep -i "^launch" /path/to/PROCESS_reweight_card.dat | awk -F'=' '{print $2}' > data/PROCESS_reweights.txt
    with open(f"data/{SIGNAL_NAME}_reweights.txt", "r") as f_in:
        reweight_names = f_in.read().splitlines()

    # Make datacards for each reweighting
    for reweight_i in tqdm(range(n_reweights), desc=f"Writing datacards to {output_dir}"):

        SIG_SYSTS_LIMIT = SystematicsTable(
            samples=[b.split("/")[-1].replace(".root", "") for b in sig_babies]
        )
        vbsvvh.df.loc[vbsvvh.df.is_signal, "event_weight"] = vbsvvh.df[vbsvvh.df.is_signal].orig_event_weight.values*reweights.T[reweight_i]

        # -- PDF uncertainty -------------------------------------------------------------------
        if args.private:
            skim_files = glob.glob(f"/data/userdata/{os.getenv('USER')}/nanoaod/VBSVVHSkim/sig_0lep_2ak4_2ak8_ttH/Private_C2W_C2Z_*/merged.root")
        else:
            skim_files = glob.glob(f"/data/userdata/{os.getenv('USER')}/nanoaod/VBSVVHSkim/sig_0lep_2ak4_2ak8_ttH/VBS*central/merged.root")
        gen_sum = 0
        pdf_sum = np.zeros(101)
        for skim_file in skim_files:
            with uproot.open(skim_file) as f:
                gen_sums = f["Runs"]["genEventSumw"].array(library="np")
                pdf_sums = f["Runs"]["LHEPdfSumw"].array(library="np")
                missed = np.array([len(s) != 101 for s in pdf_sums])
                reshaped = np.vstack(pdf_sums[~missed])
                pdf_sum += np.dot(gen_sums[~missed], reshaped) + np.sum(gen_sums[missed])
                gen_sum += np.sum(gen_sums)
                
        pdf_ratio = pdf_sum/gen_sum

        with uproot.open(f"{BABYDIR}/Run2/{SIGNAL_NAME}.root") as f:
            pdf_df = f.get("pdf_tree").arrays(library="pd")
            pdf_df = pdf_df[pdf_df[f"is_{CHANNEL}"]].reset_index()
            
        systs = []
        for R in ABCD_REGIONS:
            sig_df = vbsvvh.sig_df().reset_index()
            count = np.sum(sig_df[sig_df[R]].event_weight*pdf_df[sig_df[R]].lhe_pdf_0)
            deltas = []
            for i in range(1, 101):
                count_var = np.sum(sig_df[sig_df[R]].event_weight*pdf_df[sig_df[R]][f"lhe_pdf_{i}"])
                deltas.append(count - count_var/pdf_ratio[i])

            deltas = np.array(deltas)
            systs.append(np.sqrt(np.sum(deltas**2))/count)

        pdf_systs = Systematic("CMS_LHE_weights_pdf_vbsvvh", ABCD_REGIONS)
        pdf_systs.add_systs(systs)
        SIG_SYSTS_LIMIT.add_row(pdf_systs)
        # --------------------------------------------------------------------------------------



        # -- LHE scale weights -----------------------------------------------------------------
        lhe_muR_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("muF1p0")])
        lhe_muF_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("muR1p0")])

        muR_systs = Systematic("CMS_LHE_weights_scale_muR_vbsvvh", ABCD_REGIONS)
        muR_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "unity", *lhe_muR_weights)
        )

        muF_systs = Systematic("CMS_LHE_weights_scale_muF_vbsvvh", ABCD_REGIONS)
        muF_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "unity", *lhe_muF_weights)
        )

        # SIG_SYSTS_LIMIT.add_row(muR_systs)                   # muR variations have not effect
        SIG_SYSTS_LIMIT.add_row(muF_systs)
        # --------------------------------------------------------------------------------------


        # -- Parton shower weights -------------------------------------------------------------
        isr_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("fsr1p0")])
        fsr_weights = list(vbsvvh.df.columns[vbsvvh.df.columns.str.contains("isr1p0")])

        isr_sf_systs = Systematic("CMS_PSWeight_ISR_vbsvvh", ABCD_REGIONS)
        isr_sf_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "unity", *isr_weights)
        )

        fsr_sf_systs = Systematic("CMS_PSWeight_FSR_vbsvvh", ABCD_REGIONS)
        fsr_sf_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "unity", *fsr_weights)
        )

        SIG_SYSTS_LIMIT.add_row(isr_sf_systs)
        SIG_SYSTS_LIMIT.add_row(fsr_sf_systs)
        # --------------------------------------------------------------------------------------


        # -- Pileup reweighting ----------------------------------------------------------------
        pu_sf_systs = Systematic("CMS_vbsvvh_puWeight", ABCD_REGIONS)
        pu_sf_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "pu_sf", "pu_sf_dn", "pu_sf_up")
        )
        SIG_SYSTS_LIMIT.add_row(pu_sf_systs)
        # --------------------------------------------------------------------------------------


        # -- Pileup jet id ---------------------------------------------------------------------
        puid_sf_systs = Systematic("CMS_vbsvvh_puJetId", ABCD_REGIONS)
        puid_sf_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "puid_sf", "puid_sf_dn", "puid_sf_up")
        )
        SIG_SYSTS_LIMIT.add_row(puid_sf_systs)
        # --------------------------------------------------------------------------------------


        # -- L1 prefiring weight ---------------------------------------------------------------
        prefire_sf_systs = Systematic("CMS_PrefireWeight_13TeV", ABCD_REGIONS)
        prefire_sf_systs.add_systs(
            get_systs(SIGNAL_NAME, ABCD_REGIONS, "prefire_sf", "prefire_sf_up", "prefire_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(prefire_sf_systs)
        # --------------------------------------------------------------------------------------


        # -- MC statistical uncertainty --------------------------------------------------------
        # stat_systs = Systematic("CMS_vbsvvh_mcstat", ABCD_REGIONS)
        # stat_systs.add_systs(
        #     [
        #         vbsvvh.sig_error(selection=R)/vbsvvh.sig_count(selection=R) for R in ABCD_REGIONS
        #     ]
        # )
        # SIG_SYSTS_LIMIT.add_row(stat_systs)
        for i, (region, R) in enumerate(zip(ABCD_REGIONS, ["A", "B", "C" , "D"])):
            stat_systs = Systematic(f"CMS_vbsvvhjets_mcstat{R}", ABCD_REGIONS)
            temp_systs = [-999, -999, -999, -999]
            temp_systs[i] = vbsvvh.sig_error(selection=region)/vbsvvh.sig_count(selection=region)
            stat_systs.add_systs(temp_systs)
            SIG_SYSTS_LIMIT.add_row(stat_systs)
        # --------------------------------------------------------------------------------------


        # -- ParticleNet Xbb scale factors -----------------------------------------------------
        for year in [-2016, 2016, 2017, 2018]:
            cms_year_str = get_year_str(year).replace("20", "")
            xbb_sf_systs = Systematic(f"CMS_vbsvvhjets_bTagWeightXbb_13TeV_{cms_year_str}", ABCD_REGIONS)
            xbb_sf_systs.add_systs(
                get_systs(SIGNAL_NAME, ABCD_REGIONS, "xbb_sf", "xbb_sf_dn", "xbb_sf_up", year=year)
            )
            SIG_SYSTS_LIMIT.add_row(xbb_sf_systs)
        # --------------------------------------------------------------------------------------


        # -- ParticleNet XWqq scale factors -----------------------------------------------------
        for year in [-2016, 2016, 2017, 2018]:
            cms_year_str = get_year_str(year).replace("20", "")
            # Leading V->qq jet
            xwqq_sf_systs = Systematic(f"CMS_vbsvvhjets_qTagWeightXWqq_ldVqq_13TeV_{cms_year_str}", ABCD_REGIONS)
            xwqq_sf_systs.add_systs(
                get_systs(SIGNAL_NAME, ABCD_REGIONS, "xwqq_ld_vqq_sf", "xwqq_ld_vqq_sf_up", "xwqq_ld_vqq_sf_dn", year=year)
            )
            SIG_SYSTS_LIMIT.add_row(xwqq_sf_systs)
            # Trailing V->qq jet
            xwqq_sf_systs = Systematic(f"CMS_vbsvvhjets_qTagWeightXWqq_trVqq_13TeV_{cms_year_str}", ABCD_REGIONS)
            xwqq_sf_systs.add_systs(
                get_systs(SIGNAL_NAME, ABCD_REGIONS, "xwqq_tr_vqq_sf", "xwqq_tr_vqq_sf_up", "xwqq_tr_vqq_sf_dn", year=year)
            )
            SIG_SYSTS_LIMIT.add_row(xwqq_sf_systs)
        # --------------------------------------------------------------------------------------


        # -- Jet energy correction uncertainty -------------------------------------------------
        jec_sources = [
            "Absolute",
            "Absolute_YEAR",
            "BBEC1_YEAR",
            "BBEC1",
            "EC2_YEAR",
            "EC2",
            "FlavorQCD",
            "HF_YEAR",
            "HF",
            "RelativeBal",
            "RelativeSample_YEAR"
        ]
        for jec_i, jec_source in enumerate(jec_sources):
            if "YEAR" in jec_source:
                for year in ["2016postVFP", "2016preVFP", "2017", "2018"]:
                    jec_systs = get_jet_energy_systs(
                        f"{BABYDIR}/Run2/VBSVVH_cutflow.cflow",
                        f"{BABYDIR}_jec_{jec_i+1}_{year}_up/Run2/VBSVVH_cutflow.cflow",
                        f"{BABYDIR}_jec_{jec_i+1}_{year}_dn/Run2/VBSVVH_cutflow.cflow",
                        {
                            "regionA": "AllMerged_RegionA",
                            "regionB": "AllMerged_RegionB",
                            "regionC": "AllMerged_RegionC",
                            "regionD": "AllMerged_RegionD",
                        },
                        f"CMS_scale_j_{jec_source.replace('YEAR', year)}_13TeV"
                    )
                    SIG_SYSTS_LIMIT.add_row(jec_systs)
            else:
                jec_systs = get_jet_energy_systs(
                    f"{BABYDIR}/Run2/VBSVVH_cutflow.cflow",
                    f"{BABYDIR}_jec_{jec_i+1}_up/Run2/VBSVVH_cutflow.cflow",
                    f"{BABYDIR}_jec_{jec_i+1}_dn/Run2/VBSVVH_cutflow.cflow",
                    {
                        "regionA": "AllMerged_RegionA",
                        "regionB": "AllMerged_RegionB",
                        "regionC": "AllMerged_RegionC",
                        "regionD": "AllMerged_RegionD",
                    },
                    f"CMS_scale_j_{jec_source}_13TeV"
                )
                SIG_SYSTS_LIMIT.add_row(jec_systs)
        # --------------------------------------------------------------------------------------


        # -- Jet energy resolution uncertainty -------------------------------------------------
        jer_systs = get_jet_energy_systs(
            f"{BABYDIR}/Run2/VBSVVH_cutflow.cflow",
            f"{BABYDIR}_jer_up/Run2/VBSVVH_cutflow.cflow",
            f"{BABYDIR}_jer_dn/Run2/VBSVVH_cutflow.cflow",
            {
                "regionA": "AllMerged_RegionA",
                "regionB": "AllMerged_RegionB",
                "regionC": "AllMerged_RegionC",
                "regionD": "AllMerged_RegionD",
            },
            "CMS_res_j_13TeV"
        )
        SIG_SYSTS_LIMIT.add_row(jer_systs)
        # --------------------------------------------------------------------------------------


        # -- Luminosity ------------------------------------------------------------------------
        lumi_systs = Systematic("lumi_13TeV_correlated", ABCD_REGIONS)
        lumi_systs.add_systs([0.016 for R in ABCD_REGIONS])
        SIG_SYSTS_LIMIT.add_row(lumi_systs)
        # --------------------------------------------------------------------------------------


        # -- H to bb BR ------------------------------------------------------------------------
        # hbb_br_systs = Systematic("BR_hbb", ABCD_REGIONS)
        # hbb_br_systs.add_systs([0.0127 for R in ABCD_REGIONS])
        # SIG_SYSTS_LIMIT.add_row(hbb_br_systs)
        # --------------------------------------------------------------------------------------


        datacard_systs = {
            "TotalBkg_AllHad": {
                "CMS_vbsvvhjets_abcd_syst": [1 + 25.4/100],
                # "CMS_vbsvvhjets_abcd_stat": [1 + 34.0/100]
            },
            "TotalSig": {}
        }

        for syst_obj in SIG_SYSTS_LIMIT.systs:
            systs = syst_obj.get_systs()
            datacard_systs["TotalSig"][syst_obj.name] = [1 + systs[R][0] for R in ABCD_REGIONS]

        # for i, (region, R) in enumerate(zip(ABCD_REGIONS[1:], ["B", "C", "D"])):
        #     syst = 1 + vbsvvh.data_error(selection=region)/vbsvvh.data_count(selection=region)
        #     temp_systs = [syst, -999, -999, -999]
        #     temp_systs[i+1] = syst
        #     datacard_systs["TotalBkg_AllHad"][f"CMS_vbsvvhjets_abcd_stat{R}"] = temp_systs

        # Blinded yields
        B = vbsvvh.data_count(selection="regionB")
        C = vbsvvh.data_count(selection="regionC")
        D = vbsvvh.data_count(selection="regionD")
        ABCD_yields = [round(B*C/D), *[vbsvvh.data_count(selection=R) for R in ABCD_REGIONS[1:]]]
        # Unblinded yields
        # ABCD_yields = [vbsvvh.data_count(selection=R) for R in ABCD_REGIONS]

        datacard = DatacardABCD(
            ABCD_yields, # dummy value for observed in region A
            {"TotalSig": [vbsvvh.sig_count(selection=R) for R in ABCD_REGIONS]},
            {"TotalBkg_AllHad": [1.0 for R in ABCD_REGIONS]},
            datacard_systs,
            rparam_labels=["vbsvvhjets_a", "vbsvvhjets_b", "vbsvvhjets_c", "vbsvvhjets_d"]
        )
        datacard.fill()

        reweight_name = reweight_names[reweight_i]
        datacard.write(f"{output_dir}/{reweight_name}.dat")
