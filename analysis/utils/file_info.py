import argparse
import json
import os
import uproot

XSECS_DB = None

def get_year(input_file):
    if "HIPM_UL2016" in input_file or "UL16NanoAODAPVv9" in input_file or "RunIISummer20UL16APV" in input_file:
        return "2016preVFP"
    elif "UL2016" in input_file or "UL16NanoAODv9" in input_file or "RunIISummer20UL16" in input_file:
        return "2016postVFP"
    elif "UL2017" in input_file or "UL17NanoAODv9" in input_file or "RunIISummer20UL17" in input_file:
        return "2017"
    elif "UL2018" in input_file or "UL18NanoAODv9" in input_file or "RunIISummer20UL18" in input_file:
        return "2018"
    else:
        raise Exception(f"No year found in {input_file}")

def get_lumi(input_file):
    year = get_year(input_file)
    if year == "2016preVFP":
        return 19.52
    elif year == "2016postVFP":
        return 16.81
    elif year == "2017":
        return 41.48
    elif year == "2018":
        return 59.83

def get_xsec(input_file, xsecs_json="data/xsecs.json"):
    global XSECS_DB
    if not XSECS_DB:
        with open(xsecs_json) as f_in:
            XSECS_DB = json.load(f_in)
    matched_keys = []
    matched_chars = []
    for key, xsec in XSECS_DB.items():
        if key in input_file:
            matched_keys.append(key)
            matched_chars.append(len(key))
    if len(matched_keys) > 0:
        best_match = matched_keys[matched_chars.index(max(matched_chars))]
        return XSECS_DB[best_match]
    else:
        logging.error(f"no xsec for {input_file} in {xsecs_json}")
        return 1

def parse(input_file=None, input_files=None, xsecs_json="data/xsecs_json"):
    if not input_files:
        if input_file:
            input_files = [input_file]
        else:
            raise Exception("no input file(s) provided")

    output = {}
    n_events = 0
    for input_file in input_files:
        dirname = "/".join(input_file.split("/")[:-1])
        if dirname not in output:
            output[dirname] = {
                "n_events": 0,
                "xsec": get_xsec(input_file, xsecs_json=xsecs_json),
                "lumi": get_lumi(input_file),
                "year": get_year(input_file),
                "xsec_sf": 1
            }
        with uproot.open(input_file) as f:
            output[dirname]["n_events"] += f["Runs"]["genEventSumw"].array(library="np").sum()

    for dirname, info in output.items():
        output[dirname]["xsec_sf"] = info["xsec"]*1000*info["lumi"]/info["n_events"]

    if len(output.keys()) == 1:
        return output[dirname]
    else:
        return output

if __name__ == "__main__":
    # Check that the PWD is correct
    vbs_pwd = os.getenv("VBSPWD")
    if not vbs_pwd:
        print(f"ERROR: `source setup.sh` must be run first")
        exit()
    elif os.getcwd() != vbs_pwd:
        print(f"ERROR: must be run within {vbs_pwd}")
        exit()

    cli = argparse.ArgumentParser(description="Extract file info from path")
    cli.add_argument(
        "files", type=str, nargs="*",
        help="Input file(s) to get info for"
    )
    cli.add_argument(
        "--xsecs_json", type=str, default="data/xsecs.json",
        help="Path to JSON containing cross sections"
    )
    args = cli.parse_args()

    output = parse(input_files=args.files, xsecs_json=args.xsecs_json)

    print(json.dumps(output, indent=4))
