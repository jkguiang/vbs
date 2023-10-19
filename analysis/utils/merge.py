import os
import glob
from multiprocessing import Pool
from subprocess import Popen, PIPE
from tqdm import tqdm
from utils.cutflow import Cutflow, CutflowCollection

def hadd_job(split_cmd):
    hadd = Popen(split_cmd, stdout=PIPE, stderr=PIPE)
    hadd.wait()

def merge(output_dir, sample_map, n_hadders=8):
    # Collect cutflows and stage merge jobs
    hadd_cmds = []
    merged_cutflows = {}
    for group_name, group_map in sample_map.items():
        groups = {}
        root_files_to_merge = []
        for year, sample_list in group_map.items():
            groups[year] = []
            for sample_name in sample_list:
                cflow_files = glob.glob(f"{output_dir}/{year}/{sample_name}_Cutflow.cflow")
                for cflow_file in cflow_files:
                    if group_name in merged_cutflows.keys():
                        merged_cutflows[group_name] += Cutflow.from_file(cflow_file)
                    else:
                        merged_cutflows[group_name] = Cutflow.from_file(cflow_file)
                root_files = glob.glob(f"{output_dir}/{year}/{sample_name}*.root")
                for root_file in root_files:
                    root_files_to_merge.append(root_file)
                    groups[year].append(root_file.split("/")[-1].replace(".root", ""))
            if len(groups[year]) == 0:
                print(f"WARNING: {year} for {group_name} has no files!")
        # Stage merge (hadd) jobs
        output_file = f"{output_dir}/Run2/{group_name}.root"
        hadd_cmds.append(["hadd", output_file] + root_files_to_merge)
        if os.path.exists(output_file):
            os.remove(output_file)
        # Write list of files that were hadded
        with open(output_file.replace(".root", ".txt"), "w") as f_out:
            for year, group in groups.items():
                f_out.write("{0}:\n{1}\n\n".format(year, '\n'.join(sorted(group))))

    if n_hadders > 0:
        # Run hadd jobs
        with Pool(processes=n_hadders) as pool:
            list(tqdm(pool.imap(hadd_job, hadd_cmds), total=len(hadd_cmds), desc="Executing hadds"))

    if merged_cutflows:
        return CutflowCollection(merged_cutflows)
    else:
        return None
