# ABCDNet
Automated ABCD + SR machinery as proposed by Kasieczka et al. (https://doi.org/10.1103/PhysRevD.103.035021).

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
python python/singledisco/train.py configs/CONFIG.json
```
```
sbatch batch/singledisco/train.script configs/CONFIG.json
```
4. Same as above, but for inference:
```
srun --partition=gpu --gpus=1 --mem=16gb --constraint=a100 --pty bash -i # interactive session
python python/singledisco/infer.py configs/CONFIG.json
```
```
sbatch batch/singledisco/infer.script configs/CONFIG.json
```
5. You can make performance plots via the HPG [JupyterHub](https://jhub.rc.ufl.edu)
6. Otherwise, you can export the scores back to the input baby ROOT files using the `infer.py` script:
```
python python/singledisco/infer.py --export configs/CONFIG.json
```
7. I have written a handy script to copy the results to the UAFs
```
sh scripts/export.sh configs/CONFIG.json jguiang@uaf-10.t2.ucsd.edu:/ceph/cms/store/user/jguiang/ABCDNet
```
8. There is also a batch script for running the entire workflow:
```
sbatch batch/singledisco/full.script configs/CONFIG.json
```
