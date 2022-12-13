#!/bin/bash
function writefunc {
    year=$1
    html=$2
    bins=($(cat $html | grep 'begin{tabular}{l|c|c|c}' -A2 | tail -1 | awk -F' & ' '{print $2" "$3" "$4}'))
    sfs=($(cat $html | grep 'begin{tabular}{l|c|c|c}' -A3 | tail -1 | awk -F' & ' '{print $2" "$3" "$4}'))
    i=0
    echo "    if (year == \"$year\")"
    echo "    {"
    echo "        /* Markdown table from $html:"
    echo "           $(cat $html | grep 'src="sf_summary_B_comb.png"' -B4 | head -1)"
    echo "           $(cat $html | grep 'src="sf_summary_B_comb.png"' -B3 | head -1)"
    echo "           $(cat $html | grep 'src="sf_summary_B_comb.png"' -B2 | head -1)"
    echo "        */"
    for sf in ${sfs[@]::${#sfs[@]}-1}; do
        # Strip LaTeX symbols
        sf=${sf//$/}
        sf=${sf//\{/}
        sf=${sf//\}/}
        # Turn sf_dnerr^uperr into sf,dnerr,uperr
        sf=${sf//_/,}
        sf=${sf//^/,}
        # Turn 'sf,dnerr,uperr' into bash array
        sf=(${sf//,/ })

        # Strip LaTeX symbols
        bin=${bins[$i]}
        bin=${bin//$/}
        bin=${bin//[/}
        bin=${bin//)/}
        bin=${bin//\~/}
        # Turn 'low,high' into bash array
        bin=(${bin//,/ })

        # Write conditionals
        if [[ "$i" == "0" ]]; then
            echo "        if (pt >= ${bin[0]} && pt < ${bin[1]})"
        elif [[ "${bin[1]}" == "\infty" ]]; then
            echo "        else if (pt >= ${bin[0]})"
        else
            echo "        else if (pt >= ${bin[0]} && pt < ${bin[1]})"
        fi
            echo "        {"
            echo "            if (var == \"nominal\") { return ${sf[0]}; }"
            echo "            else if (var == \"up\") { return ${sf[0]}${sf[2]}; }"
            echo "            else if (var == \"dn\") { return ${sf[0]}${sf[1]}; }"
            echo "        }"
        i=$(($i + 1))
    done
    echo "    }"
}

if [[ $1 != "" ]]; then
    n_dirs=$(ls web/${1}*/4_fit/index.html | wc -l)
    if [[ "$n_dirs" != "4" ]]; then
        echo "ERROR: only $n_dirs/4 webpages exist for web/${1}*"
        exit 1
    else
        echo 'double boohftCalib(std::string year, double pt, std::string var = "nominal")'
        echo "{"
        writefunc 2018        web/${1}_2018/4_fit/index.html
        writefunc 2017        web/${1}_2017/4_fit/index.html
        writefunc 2016postVFP web/${1}_2016/4_fit/index.html
        writefunc 2016preVFP  web/${1}_2016APV/4_fit/index.html
        echo "    return 1.;"
        echo "}"
    fi
fi
