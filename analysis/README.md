# Overview
This repository contains the code required to run the VBS WH and VBS VVH (all-hadronic) analyses and is structured as follows:
- `bin`: all executables
- `data`: extraneous data like scale factors, golden JSON, etc.
- `include`: analysis code that is global to all studies
- `studies/{STUDY}`: analysis code specific to a given study named `STUDY`
- `utils`: various utility scripts

# Instructions
1. Install [RAPIDO](https://github.com/jkguiang/rapido)
```
git clone https://github.com/jkguiang/rapido
cd rapido
make -j6
cd -
````
2. Install [NanoTools](https://github.com/cmstas/NanoTools)
```
git clone https://github.com/cmstas/NanoTools
cd NanoTools/NanoCORE
make -j12
cd -
````
3. Retrieve the following files:
    - Scale factors:
        - Run `data/pog_jsons/get_jsons.sh` if `cvmfs` is available
        - If using the PKU lepton ID: included above
        - If using the ttH lepton ID: ask maintainer
    - JECs/JERs:
        - Run `NanoTools/NanoCORE/Tools/jetcorr/data/download_jecs.sh`
        - Run `NanoTools/NanoCORE/Tools/jetcorr/data/download_jers.sh`
    - Golden JSONs
4. Run `source setup.sh`
5. Write source code to `studies/{STUDY}`

## Running over one file
1. Compile your study
```
make study={STUDY} clean
make study={STUDY}
```
2. Run your study
```
./bin/{STUDY} --help
RAPIDO HEP CLI
usage: ./<executable> [options] <path/to/file1> [<path/to/file2> ...]

Options:
  -h, --help             display this message
  -v, --verbose          toggle verbosity flag
  -t, --input_ttree      name of ttree in input ROOT file(s) (e.g. 'Events')
  -d, --output_dir       target directory for output file(s)
  -n, --output_name      short name for output file(s)
  -T, --output_ttree     name of ttree in output ROOT file(s) (e.g. 'Events')
  -V, --variation        variation type (e.g. 'up', 'down', 'nominal', ...)
  -s, --scale_factor     global event weight
  --is_data              data flag
  --is_signal            signal flag
  --debug                debug flag
```

Note that you can use `utils/file_info.py` to get the event weight for properly normalizing
the sample according to cross-section and luminosity:
```
python3 utils/file_info.py /path/to/file.root
```
This will work only if the file path follows typical CMS conventions.

## Running over Run 2
1. Run `bin/run` to run over many samples in parallel or `bin/{STUDY}` to run file-by-file
```
$ ./bin/run --help
usage: run [-h] [--skim SKIM] [--skims [SKIMS ...]] [--tag TAG] [--var VAR] [--filter FILTER] [--output_ttree OUTPUT_TTREE] [--n_workers N_WORKERS] [--no_make] [--data] study

Run a given study in parallel

positional arguments:
  study                 Name of the study to run

optional arguments:
  -h, --help            show this help message and exit
  --skim SKIM           Name of skim (i.e. bkg_{SKIM}, sig_{SKIM}, data_{SKIM})
  --skims [SKIMS ...]   Space-separated list of standalone skims
  --tag TAG             Unique tag for output
  --var VAR             Type of variation (e.g. 'up', 'down', 'nominal', ...)
  --filter FILTER       Regex filter for excluding matching datasets
  --output_ttree OUTPUT_TTREE
                        Name of output ttree
  --n_workers N_WORKERS
                        Maximum number of worker processes
  --no_make             Do not run make before running the study
  --data                Run looper over data files (in addition to MC)
```
2. Run `bin/merge` to merge the results
```
$ ./bin/merge --help
usage: merge [-h] [--tag TAG] [--terminals [TERMINALS ...]] [--debug] [--n_workers N_WORKERS] study

Run merge results from /bin/run

positional arguments:
  study                 Name of the study to run

optional arguments:
  -h, --help            show this help message and exit
  --tag TAG             Unique tag for output
  --terminals [TERMINALS ...]
                        Names of terminal cuts in cutflow to write to CSV
  --debug               Run in debug mode
  --n_workers N_WORKERS
                        Number of workers to run hadds
```

## Running on the cluster
1. Install [Metis](https://github.com/aminnj/ProjectMetis)
```
git clone https://github.com/aminnj/ProjectMetis /path/to/ProjectMetis
```
2. Set up Metis (and make sure `tqdm` is installed; it should be in CMSSW 11.X.x)
```
source /path/to/ProjectMetis/setup.sh
```
3. Run `bin/make_package STUDY` to collect a given study into a tarball
4. Run `bin/metis` to submit jobs to the cluster
```
$ ./bin/metis --help
usage: metis [-h] [-y] [--debug] [--python2] --samples SAMPLES --tag TAG
             [--filter FILTER] [--xrootd_protocol XROOTD_PROTOCOL]
             [--xrootd_host XROOTD_HOST] [--sites [SITES [SITES ...]]]
             [--n_monit_hrs N_MONIT_HRS] [--max_jobs MAX_JOBS]
             study

Submit condor jobs

positional arguments:
  study                 Name of the study to run

optional arguments:
  -h, --help            show this help message and exit
  -y, --yes             Skip any (y/n) prompts
  --debug               Run in debug mode
  --python2             Use Python2 to run nanoAOD-tools on the condor worker
                        node
  --samples SAMPLES     sample to import
  --tag TAG             Unique tag for submissions
  --filter FILTER       Regex filter for excluding matching datasets
  --xrootd_protocol XROOTD_PROTOCOL
                        File transfer protocol for XRootD to use for reading
                        files
  --xrootd_host XROOTD_HOST
                        <IP>:<port> of desired XRootD host for reading files
  --sites [SITES [SITES ...]]
                        Space-separated list of T2 sites
  --n_monit_hrs N_MONIT_HRS
                        Number of hours to run Metis for
  --max_jobs MAX_JOBS   Maximum number of jobs per sample
```
