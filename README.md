# Instructions
1. Write source code to `studies/{STUDY}`
2. Use `bin/run` to run over many samples in parallel
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

