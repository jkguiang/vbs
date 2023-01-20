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
