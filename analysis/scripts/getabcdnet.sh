if [[ "$1" == "" ]]; then
    echo "Usage: sh getabcdnet.sh /path/to/abcdnet.tar.gz"
    exit 1
fi
mkdir temp
tar -zxvf $1 -C temp
for f_abcdnet in temp/*; do 
    f=$(echo $f_abcdnet | sed 's/_abcdnet//')
    if [[ "$f" != "$f_abcdnet" ]]; then mv $f_abcdnet $f; fi
done
TAG=$2
DESTNAME=$PWD/studies/vbsvvhjets/output_abcdnet
if [[ "$TAG" != "" ]]; then
    DESTNAME=${DESTNAME}_$TAG
fi
rm -rf $DESTNAME
mkdir -p $DESTNAME

mv temp $DESTNAME/Run2
