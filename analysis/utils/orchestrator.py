import os
import json
import logging
from subprocess import Popen, PIPE
import concurrent.futures
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
    def __init__(self, executable, input_files, n_workers=8):
        self.executable = executable
        self.input_files = input_files
        self.n_workers = n_workers

        self.input_files.sort(key=lambda f: os.stat(f).st_size, reverse=True)

    def run(self):
        # Generate jobs
        jobs = []
        stderr_files = []
        for input_file in tqdm(self.input_files, desc="Preparing jobs"):
            cmd = self._get_job(input_file)
            stdout_file, stderr_file = self._get_log_files(input_file)
            jobs.append((cmd, stdout_file, stderr_file))
            stderr_files.append(stderr_file)

        # Run jobs
        with tqdm(total=len(jobs), desc="Executing jobs") as pbar:
            with concurrent.futures.ThreadPoolExecutor(max_workers=self.n_workers) as executor:
                futures = {executor.submit(run_job, job): job for job in jobs}
                for future in concurrent.futures.as_completed(futures):
                    job = futures[future]
                    pbar.update(1)

        # Check for errors
        for stderr_file in stderr_files:
            if os.stat(stderr_file).st_size > 0:
                job_name = stderr_file.split("/")[-1].replace(".err", "")
                logging.error(f"{job_name} failed; check logs: {stderr_file}")

    def _get_job(self, input_file):
        raise NotImplementedError

    def _get_log_files(self, input_file):
        raise NotImplementedError
