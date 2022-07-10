import os
import json
import logging
from subprocess import Popen, PIPE
from multiprocessing import Pool
from tqdm import tqdm

def run_job(args):
    cmd, stdout_file, stderr_file = args
    with open(stdout_file,"wb") as stdout:
        stdout.write(f"{' '.join(cmd)}\n".encode("utf-8"))
    with open(stdout_file,"ab") as stdout, open(stderr_file,"wb") as stderr:
        process = Popen(cmd, stdout=stdout, stderr=stderr)
        process.wait()
    return

class Orchestrator:
    def __init__(self, executable, input_files, xsecs_json="", n_workers=8):
        self.executable = executable
        self.input_files = input_files
        self.xsecs_json = xsecs_json
        self.n_workers = n_workers
        if not xsecs_json:
            self.xsecs_db = {}
        else:
            with open(xsecs_json, "r") as f_in:
                self.xsecs_db = json.load(f_in)

    def get_xsec(self, file_name):
        if not self.xsecs_db:
            return 1
        matched_keys = []
        matched_chars = []
        for key, xsec in self.xsecs_db.items():
            if key in file_name:
                matched_keys.append(key)
                matched_chars.append(len(key))
        if len(matched_keys) > 0:
            best_match = matched_keys[matched_chars.index(max(matched_chars))]
            return self.xsecs_db[best_match]
        else:
            logging.warning(f"no xsec for {file_name} in {self.xsecs_json}")
            return 1

    def run(self):
        jobs = []
        stderr_files = []
        for input_file in tqdm(self.input_files, desc="Preparing jobs"):
            cmd = self._get_job(input_file)
            stdout_file, stderr_file = self._get_log_files(input_file)
            jobs.append((cmd, stdout_file, stderr_file))
            stderr_files.append(stderr_file)
        with Pool(processes=self.n_workers) as pool:
            list(tqdm(pool.imap(run_job, jobs), total=len(jobs), desc="Executing jobs"))

        for stderr_file in stderr_files:
            if os.stat(stderr_file).st_size > 0:
                job_name = stderr_file.split("/")[-1].replace(".err", "")
                logging.error(f"job '{job_name}' failed; check logs: {stderr_file}")

    def _get_job(self, input_file):
        raise NotImplementedError

    def _get_log_files(self, input_file):
        raise NotImplementedError
