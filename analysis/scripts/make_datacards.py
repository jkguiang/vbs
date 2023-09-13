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
from utils.datacard import Datacard

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
        df = vbswh.sample_df(name=sample_name, selection=SR)
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
        
        df = vbswh.sample_df(name=sample_name, selection=SR)
        df_up = vbswh.sample_df(name=sample_name, selection=SR_up)
        df_dn = vbswh.sample_df(name=sample_name, selection=SR_dn)
        
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

TAG = "new_signal"

for SIG_NAME in ["VBSWH_posLambda", "VBSWH_negLambda"]:

    # grep -i "^launch" /path/to/PROCESS_reweight_card.dat | awk -F'=' '{print $2}' > data/PROCESS_reweights.txt
    with open(f"data/{SIG_NAME}_reweights.txt", "r") as f_in:
        reweight_names = f_in.read().splitlines()

    os.makedirs(f"../combine/datacards/{SIG_NAME}", exist_ok=True)

    babies = glob.glob(f"../analysis/studies/vbswh/output_{TAG}/Run2/*.root")
    sig_babies = [b for b in babies if SIG_NAME in b]
    print(f"Found {len(sig_babies)} signal babies:")
    print("\n".join(sig_babies))
    bkg_babies = [b for b in babies if "Lambda" not in b and "VBSWH" not in b and "data" not in b]
    print(f"Found {len(bkg_babies)} background babies:")
    print("\n".join(bkg_babies))
    data_babies = [b for b in babies if "data.root" in b]
    print(f"Found {len(data_babies)} data babies:")
    print("\n".join(data_babies))

    vbswh = PandasAnalysis(
        sig_root_files=sig_babies,
        bkg_root_files=bkg_babies,
        data_root_files=data_babies,
        ttree_name="tree",
        weight_columns=[
            "xsec_sf", "lep_id_sf", "ewkfix_sf", 
            "elec_reco_sf", "muon_iso_sf", 
            "btag_sf", "pu_sf", "prefire_sf", "trig_sf",
            "xbb_sf" # applied only because Xbb > 0.9 applied everywhere for SR1 and SR2
        ],
        reweight_column="reweights"
    )
    vbswh.df["unity"] = 1 # IMPORTANT
    vbswh.df["presel_noDetaJJ"] = vbswh.df.eval(
        "passes_bveto and M_jj > 500 and hbbjet_score > 0.3"
    )
    vbswh.df["presel"] = vbswh.df.eval(
        "passes_bveto and M_jj > 500 and abs(deta_jj) > 3 and hbbjet_score > 0.3"
    )
    vbswh.df["SR2"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST > 1500 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["SR1"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST > 900 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["SR2_up"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST_up > 1500 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["SR1_up"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST_up > 900 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["SR2_dn"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST_dn > 1500 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["SR1_dn"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST_dn > 900 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["regionA"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST > 900 and hbbjet_score > 0.9 and abs(deta_jj) > 4 and hbbjet_msoftdrop >= 150"
    )
    vbswh.df["regionB"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST > 900 and hbbjet_score > 0.9 and abs(deta_jj) <= 4 and hbbjet_msoftdrop >= 150"
    )
    vbswh.df["regionC"] = vbswh.df.eval(
        "presel_noDetaJJ and M_jj > 600 and ST > 900 and hbbjet_score > 0.9 and abs(deta_jj) <= 4 and hbbjet_msoftdrop < 150"
    )
    vbswh.df["orig_event_weight"] = vbswh.df.event_weight.values.copy()

    # Insert trivial weight
    if SIG_NAME == "VBSWH_negLambda":
        vbswh.sig_reweights = np.insert(vbswh.sig_reweights, 139, 1, axis=1)
    # elif SIG_NAME == "VBSWH_posLambda":
    #     vbswh.sig_reweights = np.insert(vbswh.sig_reweights, 278, 1, axis=1)

    SIGNAL_REGIONS = ["SR1", "SR2"]

    for reweight_i in tqdm(range(vbswh.sig_reweights.shape[-1]), desc=f"Writing datacards to ../combine/datacards/{SIG_NAME}"):

        SIG_SYSTS_LIMIT = SystematicsTable(samples=[SIG_NAME])

        vbswh.df.loc[vbswh.df.is_signal, "event_weight"] = vbswh.df[vbswh.df.is_signal].orig_event_weight.values*vbswh.sig_reweights.T[reweight_i]

        # -- PDF uncertainty -------------------------------------------------------------------
        root_files = glob.glob(f"/data/userdata/jguiang/nanoaod/VBSWHSkim/sig_1lep_1ak8_2ak4_pku/{SIG_NAME}*_Mjj100toInf_Htobb_dipoleRecoilOn*/merged.root")
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

        with uproot.open(f"../analysis/studies/vbswh/output_main/Run2/{SIG_NAME}.root") as f:
            df = f.get("pdf_tree").arrays(library="pd")
            
        systs = []
        for signal_region in SIGNAL_REGIONS:
            SR = df.eval(signal_region)
            count = np.sum(df[SR].event_weight*df[SR].lhe_pdf_0)
            deltas = []
            for i in range(1, 101):
                count_var = np.sum(df[SR].event_weight*df[SR][f"lhe_pdf_{i}"])
                deltas.append(count - count_var/pdf_ratio[i])

            deltas = np.array(deltas)
            systs.append(np.sqrt(np.sum(deltas**2))/count)

        pdf_systs = Systematic("PDF variations", SIGNAL_REGIONS)
        pdf_systs.add_systs(systs)
        SIG_SYSTS_LIMIT.add_row(pdf_systs.copy("CMS_LHE_weights_pdf_vbsvh"))
        # --------------------------------------------------------------------------------------

        # -- LHE scale weights -----------------------------------------------------------------
        lhe_muR_weights = list(vbswh.df.columns[vbswh.df.columns.str.contains("muF1p0")])
        lhe_muF_weights = list(vbswh.df.columns[vbswh.df.columns.str.contains("muR1p0")])

        muR_systs = Systematic("CMS_LHE_weights_scale_muR_vbswh", SIGNAL_REGIONS)
        muR_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "unity", *lhe_muR_weights)
        )

        muF_systs = Systematic("CMS_LHE_weights_scale_muR_vbswh", SIGNAL_REGIONS)
        muF_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "unity", *lhe_muF_weights)
        )

        # SIG_SYSTS_LIMIT.add_row(muR_systs) # muR variations have not effect
        SIG_SYSTS_LIMIT.add_row(muF_systs)
        # --------------------------------------------------------------------------------------

        # -- Parton shower weights -------------------------------------------------------------
        isr_weights = list(vbswh.df.columns[vbswh.df.columns.str.contains("fsr1p0")])
        fsr_weights = list(vbswh.df.columns[vbswh.df.columns.str.contains("isr1p0")])

        isr_sf_systs = Systematic("CMS_PSWeight_ISR_vbsvh", SIGNAL_REGIONS)
        isr_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "unity", *isr_weights)
        )

        fsr_sf_systs = Systematic("CMS_PSWeight_FSR_vbsvh", SIGNAL_REGIONS)
        fsr_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "unity", *fsr_weights)
        )

        SIG_SYSTS_LIMIT.add_row(isr_sf_systs)
        SIG_SYSTS_LIMIT.add_row(fsr_sf_systs)
        # --------------------------------------------------------------------------------------

        # -- Pileup reweighting ----------------------------------------------------------------
        pu_sf_systs = Systematic("CMS_vbswhboosted_puWeight", SIGNAL_REGIONS)
        pu_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "pu_sf", "pu_sf_dn", "pu_sf_up")
        )
        SIG_SYSTS_LIMIT.add_row(pu_sf_systs)
        # --------------------------------------------------------------------------------------

        # -- Pileup jet id ---------------------------------------------------------------------
        puid_sf_systs = Systematic("Pileup jet ID", SIGNAL_REGIONS)
        puid_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "puid_sf", "puid_sf_dn", "puid_sf_up")
        )
        SIG_SYSTS_LIMIT.add_row(puid_sf_systs.copy("CMS_vbswhboosted_puJetId"))
        # --------------------------------------------------------------------------------------

        # -- L1 prefiring weight ---------------------------------------------------------------
        prefire_sf_systs = Systematic("CMS_PrefireWeight_13TeV", SIGNAL_REGIONS)
        prefire_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "prefire_sf", "prefire_sf_up", "prefire_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(prefire_sf_systs)
        # --------------------------------------------------------------------------------------

        # -- HLT scale factors -----------------------------------------------------------------
        # trig_sf_systs = Systematic("hlt_sfs", SIGNAL_REGIONS)
        # trig_sf_systs.add_systs(
        #     get_systs(SIG_NAME, SIGNAL_REGIONS, "trig_sf", "trig_sf_up", "trig_sf_dn")
        # )
        # SIG_SYSTS_LIMIT.add_row(trig_sf_systs)

        trig_elec_sf_systs = Systematic("Single-electron HLT scale factors", SIGNAL_REGIONS)
        trig_elec_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "trig_elec_sf", "trig_elec_sf_up", "trig_elec_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(trig_elec_sf_systs.copy("CMS_vbswhboosted_hlt_e_13TeV"))

        trig_muon_sf_systs = Systematic("Single-muon HLT scale factors", SIGNAL_REGIONS)
        trig_muon_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "trig_muon_sf", "trig_muon_sf_up", "trig_muon_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(trig_muon_sf_systs.copy("CMS_vbswhboosted_hlt_m_13TeV"))
        # --------------------------------------------------------------------------------------

        # -- MC statistical uncertainty --------------------------------------------------------
        stat_systs = Systematic("CMS_vbswhboosted_mcstat", SIGNAL_REGIONS)
        stat_systs.add_systs(
            [
                vbswh.sig_error(selection="SR1")/vbswh.sig_count(selection="SR1"), 
                vbswh.sig_error(selection="SR2")/vbswh.sig_count(selection="SR2")
            ]
        )
        SIG_SYSTS_LIMIT.add_row(stat_systs)
        # --------------------------------------------------------------------------------------

        # -- Lepton scale factors --------------------------------------------------------------
        # lep_id_sf_systs = Systematic("lep_id", SIGNAL_REGIONS)
        # lep_id_sf_systs.add_systs(
        #     get_systs(SIG_NAME, SIGNAL_REGIONS, "lep_id_sf", "lep_id_sf_up", "lep_id_sf_dn")
        # )
        # SIG_SYSTS_LIMIT.add_row(lep_id_sf_systs)

        elec_id_sf_systs = Systematic("CMS_vbswhboosted_id_e_13TeV", SIGNAL_REGIONS)
        elec_id_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "elec_id_sf", "elec_id_sf_up", "elec_id_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(elec_id_sf_systs)

        muon_id_sf_systs = Systematic("CMS_vbswhboosted_id_m_13TeV", SIGNAL_REGIONS)
        muon_id_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "muon_id_sf", "muon_id_sf_up", "muon_id_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(muon_id_sf_systs)

        elec_reco_sf_systs = Systematic("CMS_vbswhboosted_reco_e_13TeV", SIGNAL_REGIONS)
        elec_reco_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "elec_reco_sf", "elec_reco_sf_up", "elec_reco_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(elec_reco_sf_systs)

        muon_iso_sf_systs = Systematic("CMS_vbswhboosted_iso_m_13TeV", SIGNAL_REGIONS)
        muon_iso_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "muon_iso_sf", "muon_iso_sf_up", "muon_iso_sf_dn")
        )
        SIG_SYSTS_LIMIT.add_row(muon_iso_sf_systs)
        # --------------------------------------------------------------------------------------

        # -- ParticleNet Xbb scale factors -----------------------------------------------------
        # xbb_sf_systs = Systematic("ParticleNet Xbb scale factors", SIGNAL_REGIONS)
        # xbb_sf_systs.add_systs(
        #     get_systs(SIG_NAME, SIGNAL_REGIONS, "xbb_sf", "xbb_sf_dn", "xbb_sf_up")
        # )
        # SIG_SYSTS_LIMIT.add_row(xbb_sf_systs.copy("xbb_sfs"))
        for year in [-2016, 2016, 2017, 2018]:
            cms_year_str = get_year_str(year).replace("20", "")
            xbb_sf_systs = Systematic(f"CMS_vbswhboosted_bTagWeightXbb_13TeV_{cms_year_str}", SIGNAL_REGIONS)
            xbb_sf_systs.add_systs(
                get_systs(SIG_NAME, SIGNAL_REGIONS, "xbb_sf", "xbb_sf_dn", "xbb_sf_up", year=year)
            )
            SIG_SYSTS_LIMIT.add_row(xbb_sf_systs)
        # --------------------------------------------------------------------------------------

        # -- DeepJet b-tagging scale factors ---------------------------------------------------
        btag_sf_systs = Systematic("DeepJet b-tagging scale factors", SIGNAL_REGIONS)
        btag_sf_systs.add_systs(
            get_systs(SIG_NAME, SIGNAL_REGIONS, "btag_sf", "btag_sf_dn", "btag_sf_up")
        )
        SIG_SYSTS_LIMIT.add_row(btag_sf_systs.copy("CMS_bTagWeightDeepJet_13TeV"))
        # --------------------------------------------------------------------------------------

        # -- MET uncertainty -------------------------------------------------------------------
        met_unc_systs = Systematic("MET unc.", SIGNAL_REGIONS)
        met_unc_systs.add_systs(
            get_systs_nonSF(SIG_NAME, SIGNAL_REGIONS, ["SR1_dn", "SR2_dn"], ["SR1_up", "SR2_up"])
        )
        SIG_SYSTS_LIMIT.add_row(met_unc_systs.copy("CMS_metUncl_13TeV"))
        # --------------------------------------------------------------------------------------

        # -- Jet energy correction uncertainty -------------------------------------------------
        jec_systs = get_jet_energy_systs(
            f"../analysis/studies/vbswh/output_{TAG}/Run2/{SIG_NAME}_cutflow.cflow",
            f"../analysis/studies/vbswh/output_{TAG}_jec_up/Run2/{SIG_NAME}_cutflow.cflow",
            f"../analysis/studies/vbswh/output_{TAG}_jec_dn/Run2/{SIG_NAME}_cutflow.cflow",
            {"SR1": "XbbGt0p9_MSDLt150", "SR2": "STGt1500"},
            "Jet energy scale"
        )
        SIG_SYSTS_LIMIT.add_row(jec_systs.copy("CMS_scale_j_13TeV"))
        # --------------------------------------------------------------------------------------

        # -- Jet energy resolution uncertainty -------------------------------------------------
        jer_systs = get_jet_energy_systs(
            f"../analysis/studies/vbswh/output_{TAG}/Run2/{SIG_NAME}_cutflow.cflow",
            f"../analysis/studies/vbswh/output_{TAG}_jer_up/Run2/{SIG_NAME}_cutflow.cflow",
            f"../analysis/studies/vbswh/output_{TAG}_jer_dn/Run2/{SIG_NAME}_cutflow.cflow",
            {"SR1": "XbbGt0p9_MSDLt150", "SR2": "STGt1500"},
            "Jet energy resolution"
        )
        SIG_SYSTS_LIMIT.add_row(jer_systs.copy("CMS_res_j_13TeV"))
        # --------------------------------------------------------------------------------------

        # -- Luminosity ------------------------------------------------------------------------
        lumi_systs = Systematic("Luminosity", SIGNAL_REGIONS)
        lumi_systs.add_systs([0.016, 0.016])
        SIG_SYSTS_LIMIT.add_row(lumi_systs.copy("lumi_13TeV_correlated"))
        # --------------------------------------------------------------------------------------

        # -- H to bb BR ------------------------------------------------------------------------
        lumi_systs = Systematic("Htobb BR", SIGNAL_REGIONS)
        lumi_systs.add_systs([0.0127, 0.0127])
        SIG_SYSTS_LIMIT.add_row(lumi_systs.copy("BR_hbb"))
        # --------------------------------------------------------------------------------------

        with open("../notebooks/AN_numbers.json", "r") as f_in:
            AN_numbers = json.load(f_in)

        datacard_systs = {
            "TotalBkg": {
                "abcd_syst": [1 + AN_numbers["BkgEstTotalSystErr"]/100],
                "abcd_stat": [1 + AN_numbers["BkgEstStatErr"]/100]
            }
        }

        datacard_systs[SIG_NAME] = {}
        for syst_obj in SIG_SYSTS_LIMIT.systs:
            systs = syst_obj.get_systs()
            datacard_systs[SIG_NAME][syst_obj.name] = [1 + systs["SR1"][0]]
            
        pred_bkg = AN_numbers["PredBkg"]
        datacard = Datacard(
            [round(pred_bkg)], # dummy value for observed
            {SIG_NAME: [vbswh.sig_count(selection="SR1")]},
            {"TotalBkg": [pred_bkg]},
            datacard_systs
        )
            
        datacard.write(f"../combine/datacards/{SIG_NAME}/{reweight_names[reweight_i]}.dat")
