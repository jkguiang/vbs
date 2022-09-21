# Instructions
0. Make a soft link to `analysis/utils` here
```
ln -s ../analysis/utils utils
```
1. Install Python environment
```
# Using conda
conda create --name vbs --file requirements.conda.txt
# Using pip
pip install -r requirements.pip.txt
````
2. Run notebook. If on a remote machine, do the following:
```
jupyter notebook --no-browser --port=8892
```
Then, in a separate tab, run this command on your computer
```
ssh -N -f -L 8892:localhost:8892 USER@hostname.ext
```
