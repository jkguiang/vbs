if [[ "$1" == "" ]]; then
    echo "Usage: export.sh USER@HOST.EXT:/path/to/destination/dir"
    exit 1
fi
mkdir -p tarballs

INPUTDIR=$(cat config.json | grep -o '"input_dir": "[^"]*"' | sed 's/"input_dir": "//' | sed 's/"//' | awk '{print $1}')
ABCDNAME=$(cat config.json | grep -o '"name": "[^"]*"' | sed 's/"name": "//' | sed 's/"//' | awk '{print $1}')
ABCDNAME=$(echo $ABCDNAME | awk '{print $1}')

TARBALL=tarballs/${ABCDNAME}.tar.gz
echo "Creating ${TARBALL}..."
tar -zcvf $TARBALL $INPUTDIR/*_abcdnet.root
echo "Done"

scp $TARBALL $1
