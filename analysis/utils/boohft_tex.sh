#!/bin/bash
function writerow {
    year=$1
    html=$2
    sfs=($(cat $html | grep 'begin{tabular}{l|c|c|c}' -A3 | tail -1 | awk -F' & ' '{print $2" "$3" "$4}'))
    echo "$year & ${sfs[0]} & ${sfs[1]} & ${sfs[2]} \\\\ \\hline"
}

if [[ $1 == "" ]]; then
    echo "ERROR: no name provided; check web/ for valid names"
    exit 1
fi

n_dirs=$(ls web/${1}*/4_fit/index.html | wc -l)
if [[ "$n_dirs" != "4" ]]; then
    echo "ERROR: only $n_dirs/4 webpages exist for web/${1}*"
    exit 1
fi

cat << "EOF"
\begin{table}[!h]
\centering
\renewcommand{\arraystretch}{1.5}%
\begin{tabular}{|c|c|c|c|}
    \hline
    \multirow{2}{*}{Year} & \multicolumn{3}{c|}{\pt\ range in GeV:}                                                 \\
                          & $[250,~500)$                & $[500,~700)$                & $[700,~\infty)$             \\ \hline\hline
EOF
writerow "    2018                 "  web/${1}_2018/4_fit/index.html
writerow "    2017                 "  web/${1}_2017/4_fit/index.html
writerow "    2016 (post-VFP)      "  web/${1}_2016/4_fit/index.html
writerow "    2016 (pre-VFP)       "  web/${1}_2016APV/4_fit/index.html
cat << "EOF"
\end{tabular}
\caption{Summary of the ParticleNet Xbb-tagging scale factors for the Xbb $> 0.9$ working point.}
\label{tab:particleNetSFs}
\end{table}
EOF
