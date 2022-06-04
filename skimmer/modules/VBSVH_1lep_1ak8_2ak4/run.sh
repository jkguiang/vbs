#!/bin/bash
PYTHONX=$1
shift 1
NANOAODPATHS=($@)

# Run skimmer
$PYTHONX scripts/nano_postproc.py \
    ./ \
    ${NANOAODPATHS[@]} \
    -b python/postprocessing/modules/skimmer/keep_and_drop.txt \
    -I PhysicsTools.NanoAODTools.postprocessing.modules.skimmer.lepJetBTagAdder lepJetBTagDeepFlav \
    -I PhysicsTools.NanoAODTools.postprocessing.modules.skimmer.skimModule skimModuleConstr

# Merge skims if multiple are made
SKIMFILES=($(ls *_Skim.root))
if [[ "${#SKIMFILES[@]}" == "1" ]]; then
    mv ${SKIMFILES[0]} output.root
else
    MERGECMD="$PYTHONX scripts/haddnano.py output.root ${SKIMFILES[@]}"
    echo $MERGECMD
    $MERGECMD
fi
