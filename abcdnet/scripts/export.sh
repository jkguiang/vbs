if [[ "$1" == "" || "$2" == "" ]]; then
    echo "Usage: sh scripts/export.sh config/CONFIG.json USER@HOST.EXT:/path/to/destination/dir"
    exit 1
fi

BASEDIR=$(cat $1 | grep -o '"base_dir": "[^"]*"' | sed 's/"base_dir": "//' | sed 's/"//' | awk '{print $1}')

if [[ "$BASEDIR" == "" ]]; then
    echo "ERROR: no 'base_dir' specified in $1"
fi

mkdir -p $BASEDIR/tarballs

CWD=$PWD

ABCDNAME=$(basename $1)
ABCDNAME=${ABCDNAME%%.*}

if [[ "$ABCDNAME" == "" ]]; then
    echo "ERROR: encountered error when extracting name from $1"
    exit 1
fi

TARBALL=$BASEDIR/tarballs/${ABCDNAME}.tar.gz
echo "Creating ${TARBALL}..."

cd $BASEDIR/$ABCDNAME/output
tar -zcvf $TARBALL *_abcdnet.root
cd $CWD

echo "Done"

scp $TARBALL $2
