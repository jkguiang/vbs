NAME=vbswh
if [[ "$1" == "--unblind" ]]; then
    NAME=${NAME}_unblinded
elif [[ "$1" != "" ]]; then
    if [[ "$1" != "--expectSignal=1" ]]; then
        echo "ERROR: $1 not a recognized argument"
        exit 1
    fi
fi

# Compute impacts
if [[ "$1" == "--unblind" ]]; then
    # Make workspace
    rm -f ${NAME}_impacts.root
    text2workspace.py ../datacards/${NAME}.dat -m 125 -o ${NAME}_impacts.root
    # Set up impacts
    combineTool.py -M Impacts -d ${NAME}_impacts.root -m 125 --doInitialFit --robustFit 1 --setParameterRanges r=-1.0,2.0
    combineTool.py -M Impacts -d ${NAME}_impacts.root -m 125 --robustFit 1 --doFits --setParameterRanges r=-1.0,2.0
    combineTool.py -M Impacts -d ${NAME}_impacts.root -m 125 -o ${NAME}_impacts.json
    plotImpacts.py -i ${NAME}_impacts.json -o ${NAME}_impacts --blind
    mv ${NAME}_impacts.pdf $HOME/public_html/vbswh_plots/limits/${NAME}_impacts.pdf
    echo "Wrote $HOME/public_html/vbswh_plots/limits/${NAME}_impacts.pdf"
elif [[ "$1" == "--expectSignal=1" ]]; then
    # Make workspace
    rm -f ${NAME}_expectSignal1_impacts.root
    text2workspace.py ../datacards/${NAME}.dat -m 125 -o ${NAME}_expectSignal1_impacts.root
    # Set up impacts
    combineTool.py -M Impacts -d ${NAME}_expectSignal1_impacts.root -m 125 --doInitialFit --robustFit 1 -t -1 --expectSignal=1
    combineTool.py -M Impacts -d ${NAME}_expectSignal1_impacts.root -m 125 --robustFit 1 --doFits -t -1 --expectSignal=1
    combineTool.py -M Impacts -d ${NAME}_expectSignal1_impacts.root -m 125 -o ${NAME}_expectSignal1_impacts.json
    # Make plot
    plotImpacts.py -i ${NAME}_expectSignal1_impacts.json -o ${NAME}_expectSignal1_impacts
    mv ${NAME}_expectSignal1_impacts.pdf $HOME/public_html/vbswh_plots/limits/${NAME}_expectSignal1_impacts.pdf
    echo "Wrote $HOME/public_html/vbswh_plots/limits/${NAME}_expectSignal1_impacts.pdf"
else
    # Make workspace
    rm -f ${NAME}_expectSignal0_impacts.root
    text2workspace.py ../datacards/${NAME}.dat -m 125 -o ${NAME}_expectSignal0_impacts.root
    # Set up impacts
    combineTool.py -M Impacts -d ${NAME}_expectSignal0_impacts.root -m 125 --doInitialFit --robustFit 1 -t -1 --setParameterRanges r=-1.0,2.0 --expectSignal=0
    combineTool.py -M Impacts -d ${NAME}_expectSignal0_impacts.root -m 125 --robustFit 1 --doFits -t -1 --setParameterRanges r=-1.0,2.0 --expectSignal=0
    combineTool.py -M Impacts -d ${NAME}_expectSignal0_impacts.root -m 125 -o ${NAME}_expectSignal0_impacts.json
    # Make plot
    plotImpacts.py -i ${NAME}_expectSignal0_impacts.json -o ${NAME}_expectSignal0_impacts
    mv ${NAME}_expectSignal0_impacts.pdf $HOME/public_html/vbswh_plots/limits/${NAME}_expectSignal0_impacts.pdf
    echo "Wrote $HOME/public_html/vbswh_plots/limits/${NAME}_expectSignal0_impacts.pdf"
fi
