#!/bin/bash
shell_path=$(
    cd "$(dirname "$0")" || exit
    pwd
)
lib_path=${shell_path}/lib
project=ts_transmit_test

ulimit -n 65536
export LD_LIBRARY_PATH=${lib_path}:${LD_LIBRARY_PATH}

cd "${lib_path}" || exit
for file in *.so.*; do
    if [ -f "${file}" ]; then
        realname=$(echo "${file}" | rev | cut -d '/' -f 1 | rev)
        libname=$(echo "${realname}" | cut -d '.' -f 1)
        if [ ! -f "${libname}".so ]; then
            ln -sf "${realname}" "${libname}".so
        fi
    fi
done
ldconfig -n .

runlog_max_size=10000000
cd "${shell_path}" || exit
if [ -f runlog ]; then
    runlog_size=$(stat --format=%s runlog)
    if [ "${runlog_size}" -gt ${runlog_max_size} ]; then
        echo -e "runlog too big, restart at $(date)" >runlog
    fi
fi

function TrapSigint() {
    :
}
trap TrapSigint 2

echo -e "${project}-debug start at $(date)" >>runlog

cd "${shell_path}" || exit
chmod +x ${project}
gdb --args ${project} "$@"

echo -e "${project}-debug stop at $(date)" >>runlog
