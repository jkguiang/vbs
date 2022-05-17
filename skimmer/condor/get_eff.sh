TAG=$1
if [[ "$TAG" != "" ]]; then
    for dir in $(ls -d tasks/*$TAG); do 
        skim_effs=$(cat $dir/logs/std_logs/*.out | grep "Finally selected" | awk '{print $(NF-1)/$2}')
        avg_eff=$(echo $skim_effs | awk '{sum+=$1}; END {print sum/NR}' RS=" ")
        full_name=${dir##*CondorTask_}
        echo ${full_name%%_TuneCP5*}, $avg_eff
    done
fi
