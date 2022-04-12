#!/bin/bash

#set -e
#set -x

if [ -z "$1" ]; then
    echo "Please specify IoD Sim directory path."
    exit 1
fi
IODSIM_DIR=$1

# check dependencies are OK
SPLASH=$(find . -name splash -executable -type f)
if [ -z "$SPLASH" ]; then
    echo "Cannot find splash executable. Did you compile it?"
    exit 1
fi

which clang &>/dev/null
if [ "$?" -ne 0 ]; then
    echo "Cannot find clang installed in your system. Make sure clang is installed and available in your PATH."
    exit 1
fi

which jq &>/dev/null
if [ "$?" -ne 0 ]; then
    echo "Cannot find jq installed in your system. Make sure jq is installed and available in your PATH."
    exit 1
fi

# cleanup everything
rm -rf {models,irs,packages}
# directory skeleton
mkdir {models,irs,packages} 2>/dev/null

# FILES=$(cat ${IODSIM_DIR}/ns3/build/compile_commands.json | jq -c '.[] | .file' | sed -e 's/^\"\.\.\///g' -e 's/\"$//g' | grep '^src/')
# FILES_NUM=$(cat ${IODSIM_DIR}/ns3/build/compile_commands.json | jq -c '.[] | .file' | sed -e 's/^\"\.\.\///g' -e 's/\"$//g' | grep '^src/' | wc -l)
# INCLUDE_FLAGS=$(cat ${IODSIM_DIR}/ns3/build/compile_commands.json | jq -c '.[] | .command' | grep -o '\-I[a-zA-Z0-9\/\.]* ' | sort | uniq | sed -e '/^-I\// ! s/-I/-I\$\{IODSIM_DIR\}\/ns3\/build\//g')

FILES=$(find -L ${IODSIM_DIR}/ns3/src/ \
             -type d \( -name examples -o -name test \) \
             -prune -false \
             -o -name "*-model.cc" \
             -o -name "*-manager.cc" \
             -o -name "*-mac.cc" \
             -o -name "*application.cc" \
             -o -name "*energy-source.cc")
FILES_NUM=$(find -L ${IODSIM_DIR}/ns3/src/ \
                 -type d \( -name examples -o -name test \) \
                 -prune -false \
                 -o -name "*-model.cc" \
                 -o -name "*-manager.cc" \
                 -o -name "*-mac.cc" \
                 -o -name "*application.cc" \
                 -o -name "*energy-source.cc" | wc -l)
i=1
for f in $FILES; do
    #FPATH="${IODSIM_DIR}/ns3/${f}"
    FPATH=$f
    FNAME=${FPATH##*/}
    FNAME_WITHOUT_EXT=${FNAME%.*}
    PCH_MODEL_PATH="models/${FNAME_WITHOUT_EXT}.pch"
    IR_MODEL_PATH="irs/${FNAME_WITHOUT_EXT}.json"

    echo "[${i}/${FILES_NUM}] Transpiling ${FNAME}"
    clang -x c++ \
      -I${IODSIM_DIR}/ns3/build/ \
      -emit-ast \
      -o $PCH_MODEL_PATH \
      $FPATH

    $SPLASH $PCH_MODEL_PATH $IR_MODEL_PATH
    i=$(($i + 1))
done

pushd irs &>/dev/null
jq -s '.[0]=([.[]]|flatten)|.[0]' *.json > merged.json
popd &>/dev/null

./parent_solver.py irs/merged.json irs/merged-parent_solved.json

./generate_nodes.py irs/merged-parent_solved.json packages/ -p iodsim
