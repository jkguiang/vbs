# ABCDNet
Automated ABCD + SR machinery as proposed by Kasieczka et al. (https://doi.org/10.1103/PhysRevD.103.035021).

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
The following instructions were formulated for running this code on the [HiPerGator](https://www.rc.ufl.edu/about/hipergator/) (HPG) HPC system at UFL.
1. Examine the configuration JSON; this is (in principle) the only file that needs to be edited
2. Ingress the data (as far as I can tell, this is not accelerated with GPUs)
```
source setup_hpg.sh
python python/ingress.py configs/CONFIG.json
```
3. Request a GPU node either in an interactive session or via batch submission, then run training:
```
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
python python/train.py configs/CONFIG.json
```
```
sbatch batch/train.script configs/CONFIG.json
```
4. Same as above, but for inference:
```
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
python python/infer.py configs/CONFIG.json
```
```
sbatch batch/infer.script configs/CONFIG.json
```
5. You can make performance plots via the HPG [JupyterHub](https://jhub.rc.ufl.edu)
6. Otherwise, you can export the scores back to the input baby ROOT files using the `infer.py` script:
```
python python/infer.py --export configs/CONFIG.json
```
7. I have written a handy script to copy the results to the UAFs
```
sh scripts/export.sh configs/CONFIG.json jguiang@uaf-10.t2.ucsd.edu:/ceph/cms/store/user/jguiang/ABCDNet
```
8. There is also a batch script for running the entire workflow:
```
sbatch batch/full.script configs/CONFIG.json
```
