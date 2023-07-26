# ABCDNet
Automated ABCD + SR machinery as proposed by Kasieczka et al. (https://doi.org/10.1103/PhysRevD.103.035021).
The instructions below were formulated for running this code on the [HiPerGator](https://www.rc.ufl.edu/about/hipergator/) (HPG) HPC system at UFL.

# Quick start
See the instructions below this section for training ABCDNet for analysis. 
This section will reproduce the 3D gaussians example from the PRL paper referenced above as an example.

1. Make the Gaussian data via some interactive session
```
# Run on a CPU node: more responsible, but will take longer to match
srun --ntasks=1 --cpus-per-task=1 --mem=4gb --pty bash -i
source setup_hpg.sh
python scripts/make_gaussians.py
exit
```
```
# Run on a GPU node: more wasteful (GPU will not be used), but will match almost instantly
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i
source setup_hpg.sh
python scripts/make_gaussians.py
exit
```
2. Note where the data gets saved (`USER` should already be your HPG username)
```
Wrote /blue/p.chang/USER/data/vbsvvh/gaussiansPosCov/bkg.root
Wrote /blue/p.chang/USER/data/vbsvvh/gaussiansPosCov/sig.root
```
3. Make minor edits to `configs/Gaussians3D_PRL_dCorr.json` (now you need to replace `USER` with your HPG username)
```diff
{
-   "base_dir": "/blue/p.chang/jguiang/data/vbsvvh",
+   "base_dir": "/blue/p.chang/USER/data/vbsvvh",
    ...
    "ingress": {
-        "input_dir": "/blue/p.chang/jguiang/data/vbsvvh/gaussiansPosCov",
+        "input_dir": "/blue/p.chang/USER/data/vbsvvh/gaussiansPosCov",
        "sig_files": ["sig.root"],
        "bkg_files": ["bkg.root"],
        "ttree_name": "tree", 
        "features": ["X1", "X2"],
        "transforms": {},
        "disco_target": "X0"
    },
    ...
}
```
4. Train the model
```
sbatch batch/full.script configs/Gaussians3D_PRL_dCorr.json
```
5. Check `/blue/p.chang/USER/data/vbsvvh` for the output files

# Instructions
This section enumerates the training steps for ABCDNet. 
Note that a single batch script can instead be used to submit a job to perform all of the steps on a worker node--see step (6). 

1. Edit the configuration JSON; this is (in principle) the only file that needs to be edited
2. Run the training steps (data ingress, training loop, final inferences)
```
# Run interactively
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
source setup_hpg.sh
python python/ingress.py configs/CONFIG.json
python python/train.py configs/CONFIG.json
python python/infer.py configs/CONFIG.json --epoch=100
```
```
# Run in batch
sbatch batch/ingress.script configs/CONFIG.json
sbatch batch/train.script configs/CONFIG.json
sbatch batch/infer.script configs/CONFIG.json --epoch=100
```
3. You can make performance plots via the HPG [JupyterHub](https://jhub.rc.ufl.edu) or with the included plotting script:
```
# Run interactively
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
source setup_hpg.sh
python scripts/plot.py configs/CONFIG.json --epoch=100
python scripts/plot.py configs/CONFIG.json --epoch=200
...
```
```
# Run in batch
sbatch batch/scan.script configs/CONFIG.json 100 200 ...
```
4. Finally, you can export the scores back to the input baby ROOT files in the inference step:
```
# Run interactively
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
source setup_hpg.sh
python python/infer.py configs/CONFIG.json --epoch=100 --export
```
```
# Run in batch
sbatch batch/infer.script configs/CONFIG.json --epoch=100 --export
```
5. I have written a handy script to copy the results to another machine (here the UAF system at UCSD)
```
sh scripts/export.sh configs/CONFIG.json USER@uaf-10.t2.ucsd.edu:/ceph/cms/store/user/USER/
```
6. There is also a batch script for running the entire workflow shown in step (2)
```
sbatch batch/full.script configs/CONFIG.json
```
