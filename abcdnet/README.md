# ABCDNet
Automated ABCD + SR machinery as proposed by Kasieczka et al. (https://doi.org/10.1103/PhysRevD.103.035021).
The instructions below were formulated for running this code on the [HiPerGator](https://www.rc.ufl.edu/about/hipergator/) (HPG) HPC system at UFL.

# Quick start
See the instructions below this section for a more in-depth explanation of each step. 
This section will reproduce the 3D gaussians example from the PRL paper referenced above.

1. Make the Gaussian data via some interactive session
```
# Run on a CPU node: more responsible, but will take longer to match
srun --ntasks=1 --cpus-per-task=1 --mem=4gb --pty bash -i
python scripts/make_gaussians.py
exit
```
```
# Run on a GPU node: more wasteful (GPU will not be used), but will match almost instantly
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i
python scripts/make_gaussians.py
exit
```
2. Note where the data gets saved (`USER` should already be your HPG username)
```
Wrote /blue/p.chang/USER/data/vbsvvh/gaussiansPosCov/bkg.root
Wrote /blue/p.chang/USER/data/vbsvvh/gaussiansPosCov/sig.root
```
3. Make minor edits to the config (now you need to replace `USER` with your HPG username)
```diff
{
-   "basedir": "/blue/p.chang/jguiang/data/vbsvvh",
+   "basedir": "/blue/p.chang/USER/data/vbsvvh",
    ...
    "ingress": {
-        "input_dir": "/blue/p.chang/jguiang/data/vbsvvh/gaussiansPosCov",
+        "input_dir": "/blue/p.chang/USER/data/vbsvvh/gaussiansPosCov",
        "signal_file": "sig.root",
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
Note that a single batch script can instead be used to submit a job to perform all of the steps on a worker node--see step (7). 

1. Edit the configuration JSON; this is (in principle) the only file that needs to be edited
2. Run the training steps
```
# Run interactively
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
source setup_hpg.sh
python python/ingress.py configs/CONFIG.json
python python/train.py configs/CONFIG.json
python python/infer.py configs/CONFIG.json --epoch=100
exit
```
```
# Run in batch
sbatch batch/ingress.script configs/CONFIG.json
sbatch batch/train.script configs/CONFIG.json
sbatch batch/infer.script configs/CONFIG.json --epoch=100
```
4. You can make performance plots via the HPG [JupyterHub](https://jhub.rc.ufl.edu)
5. Finally, you can export the scores back to the input baby ROOT files using the `infer.py` script:
```
# Run interactively
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
source setup_hpg.sh
python python/infer.py --export configs/CONFIG.json
```
```
# Run in batch
sbatch batch/infer.script configs/CONFIG.json --epoch=100 --export
```
6. I have written a handy script to copy the results to the UAFs
```
sh scripts/export.sh configs/CONFIG.json jguiang@uaf-10.t2.ucsd.edu:/ceph/cms/store/user/jguiang/ABCDNet
```
7. There is also a batch script for running the entire workflow shown in step (2)
```
sbatch batch/full.script configs/CONFIG.json
```
