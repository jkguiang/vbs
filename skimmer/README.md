### Instructions
1. Ensure [ProjectMetis](https://github.com/aminnj/ProjectMetis) is installed and set up
2. Create the skimmer package
    - Ensure that you have ROOT available (e.g. in a CMSSW env) if you indent to to compile NanoCORE
```
./bin/make_package --module=MODULE
```
(Optional) Run the skimmer locally
```
./bin/test_package /path/to/package.tar.gz root://hostname:port//path/to/file.root
```
3. Run the Metis submission script
    - Note that Metis needs `tqdm`, so make sure this is installed (should be included in CMSSW 10.6.25)
    - The `debug` flag will run just one job such that you can ensure everything is running properly
```
./bin/metis --tag=<your tag here> --xrootd_host=<cache:port> --sites T2_US_SITE1 T2_US_SITE2 ...
```
