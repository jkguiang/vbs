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

def prepare_job(args):
    orchestrator, input_file = args
    cmd = orchestrator._get_job(input_file)
    stdout_file, stderr_file = orchestrator._get_log_files(input_file)
    return cmd, stdout_file, stderr_file

class Job:
    def __init__(self, cmd, stdout_file, stderr_file):
        self.cmd = cmd
        self.stdout_file = stdout_file
        self.stderr_file = stderr_file

    def unpack(self):
        return (self.cmd, self.stdout_file, self.stderr_file)

class Orchestrator:
    def __init__(self, executable, input_files, n_workers=8):
        self.executable = executable
        self.input_files = sorted(
            input_files, 
            key=lambda f: os.stat(f).st_size, 
            reverse=True
        )
        self.n_workers = n_workers

    def run(self):
        # Prepare jobs
        jobs = []
        stderr_files = []
        with tqdm(total=len(self.input_files), desc="Preparing jobs") as pbar:
            with concurrent.futures.ThreadPoolExecutor(max_workers=self.n_workers) as executor:
                futures = {executor.submit(prepare_job, (self, input_file)): input_file for input_file in self.input_files}
                for future in concurrent.futures.as_completed(futures):
                    job = Job(*future.result())
                    jobs.append(job)
                    stderr_files.append(job.stderr_file)
                    pbar.update(1)

        # Execute jobs
        with tqdm(total=len(jobs), desc="Executing jobs") as pbar:
            with concurrent.futures.ThreadPoolExecutor(max_workers=self.n_workers) as executor:
                futures = {executor.submit(run_job, job.unpack()): job for job in jobs}
                for future in concurrent.futures.as_completed(futures):
                    job = futures[future]
                    pbar.update(1)

        # Check for errors
        n_errors = 0
        for stderr_file in stderr_files:
            if os.stat(stderr_file).st_size > 0:
                job_name = stderr_file.split("/")[-1].replace(".err", "")
                logging.error(f"{job_name} failed; check logs: {stderr_file}")
                n_errors += 1

        # Alert user of errors
        if n_errors > 0:
            logfile = logging.getLoggerClass().root.handlers[0].baseFilename
            n_jobs = f"{n_errors} job{'s' if n_errors > 1 else ''}"
            print(f"WARNING: {n_jobs} did not run successfully; check {logfile}")

    def _get_job(self, input_file):
        raise NotImplementedError

    def _get_log_files(self, input_file):
        raise NotImplementedError
