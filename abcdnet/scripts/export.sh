if [[ "$1" == "" || "$2" == "" ]]; then
    echo "Usage: sh scripts/export.sh config/CONFIG.json USER@HOST.EXT:/path/to/destination/dir"
    exit 1
fi

BASEDIR=$(cat $1 | grep -o '"basedir": "[^"]*"' | sed 's/"basedir": "//' | sed 's/"//' | awk '{print $1}')
INPUTDIR=$(cat $1 | grep -o '"input_dir": "[^"]*"' | sed 's/"input_dir": "//' | sed 's/"//' | awk '{print $1}')

if [[ "$BASEDIR" == "" ]]; then
    echo "ERROR: no 'basedir' specified in $1"
fi

if [[ "$INPUTDIR" == "" ]]; then
    echo "ERROR: no 'input_dir' specified in $1"
fi

mkdir -p $BASEDIR/tarballs

CWD=$PWD

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

TARBALL=$BASEDIR/tarballs/${ABCDNAME}.tar.gz
echo "Creating ${TARBALL}..."

cd $INPUTDIR/$ABCDNAME
tar -zcvf $TARBALL *_abcdnet.root
cd $CWD

echo "Done"

scp $TARBALL $2
