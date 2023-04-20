if [[ "$1" == "" || "$2" == "" ]]; then
    echo "Usage: sh scripts/export.sh config/CONFIG.json USER@HOST.EXT:/path/to/destination/dir"
    exit 1
fi
mkdir -p tarballs

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

TARBALL=tarballs/${ABCDNAME}.tar.gz
echo "Creating ${TARBALL}..."
tar -zcvf $TARBALL $INPUTDIR/*_abcdnet.root $INPUTDIR/data.root
echo "Done"

scp $TARBALL $2
