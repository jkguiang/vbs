if [[ "$1" == "" || "$2" == "" ]]; then
    echo "Usage: sh scripts/export.sh config/CONFIG.json USER@HOST.EXT:/path/to/destination/dir"
    exit 1
fi
mkdir -p tarballs

CWD=$PWD

INPUTDIR=$(cat $1 | grep -o '"input_dir": "[^"]*"' | sed 's/"input_dir": "//' | sed 's/"//' | awk '{print $1}')
ABCDNAME=$(basename $1)
ABCDNAME=${ABCDNAME%%.*}

if [[ "$INPUTDIR" == "" ]]; then
    echo "ERROR: encountered error when extracting input directory from $1"
    exit 1
fi
if [[ "$ABCDNAME" == "" ]]; then
    echo "ERROR: encountered error when extracting name from $1"
    exit 1
fi

TARBALL=$CWD/tarballs/${ABCDNAME}.tar.gz
echo "Creating ${TARBALL}..."

cd $INPUTDIR/$ABCDNAME
# tar -zcvf $TARBALL $INPUTDIR/$ABCDNAME/*_abcdnet.root $INPUTDIR/data.root
tar -zcvf $TARBALL *_abcdnet.root
cd $CWD

echo "Done"

scp $TARBALL $2
