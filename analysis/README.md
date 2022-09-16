# Instructions
0. Ask a maintainer for the necessary files:
    - Lepton scale factors
    - Golden JSONs
1. Install RAPIDO
```
git clone https://github.com/jkguiang/rapido
cd rapido
make -j6
cd -
````
2. Install NanoTools
```
git clone https://github.com/cmstas/NanoTools
cd NanoTools/NanoCORE
make -j12
cd -
````
3. Run `source setup.sh`
4. Write source code to `studies/{STUDY}`

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
1. Use `bin/run` to run over many samples in parallel or `bin/{STUDY}` to run file-by-file
```
$ ./bin/run --help
usage: run [-h] --study STUDY [--n_workers N_WORKERS] [--debug] [--nomake]

Run a given study in parallel

optional arguments:
  -h, --help            show this help message and exit
  --study STUDY         Name of the study to run
  --n_workers N_WORKERS
                        Maximum number of worker processes
  --debug               Run in debug mode
  --nomake              Do not run make before running the study
```
2. Use `bin/merge` to merge the results
