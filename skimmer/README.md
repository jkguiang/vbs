### Instructions
0. Ensure [ProjectMetis](https://github.com/aminnj/ProjectMetis) is installed and set up
1. Create the skimmer package
    - Ensure that you have ROOT available (e.g. in a CMSSW env), since it is needed to compile NanoCORE
```
sh run.sh --no_skim
```
(Optional) Run the skimmer locally
```
sh run.sh --input_file=root://cache:2094//path/to/file.root --package=/path/to/package.tar.gz
```
2. Create skimmer package (cf. parent dir)
3. Run the Metis submission script
    - Note that Metis needs `tqdm`, so make sure this is installed (should be included in CMSSW 10.6.25)
    - The `debug` flag will run just one job such that you can ensure everything is running properly
```
python run.py --tag=<your tag here> --xrootd_host=<cache:port> --sites T2_US_SITE1 T2_US_SITE2 ...
```
4. Wait...
