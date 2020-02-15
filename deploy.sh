#!/bin/bash
if [ $# -lt 1 ] ; then
    echo "usage: deploy.sh architecture [push]"
    exit 1
fi

ARCH=$1
PUSH=$2

function generate_license() {
    OUTFILE=release/${ARCH}/LICENSE
    # patchtool
    echo -e "patchtool\n" > $OUTFILE
    cat ./LICENSE >> $OUTFILE

    # bzip2
    echo -e "\n---\n" >> $OUTFILE
    echo -e "bzip2\n" >> $OUTFILE
    cat lib/bzip2/LICENSE >> $OUTFILE

    # bsdiff
    echo -e "\n---\n" >> $OUTFILE
    echo -e "bsdiff\n" >> $OUTFILE
    cat lib/bsdiff/LICENSE >> $OUTFILE
}

if [[ "$ARCH" == "win64" ]]; then
    cp patchgen/patchgen.exe release/${ARCH}/
    cp patchapply/patchapply.exe release/${ARCH}/
fi

if [[ "$ARCH" == "win32" ]]; then
    cp patchgen/patchgen.exe release/${ARCH}/
    cp patchapply/patchapply.exe release/${ARCH}/
fi

if [[ "$ARCH" == "macos" ]]; then
    cp patchgen/patchgen release/${ARCH}/
    cp patchapply/patchapply release/${ARCH}/
fi

# generate texts
cp README.md release/${ARCH}/
generate_license

# push
if [[ "$PUSH" == "push" ]]; then
    git checkout master
   git add release/${ARCH}
    git commit -m "[ci skip] deploy."
    git push https://${GITHUB_TOKEN}@github.com/${TRAVIS_REPO_SLUG}.git ${TRAVIS_BRANCH}
fi
