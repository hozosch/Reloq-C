#!/usr/bin/env bash
# Install the ReLoq C voice for Speech Dispatcher on Linux.
#
#   ./install.sh               as root: for every user; otherwise: for you only
#   ./install.sh --system      for every user (run with sudo)
#   ./install.sh --user        for the current user, without root
#   ./install.sh --uninstall   remove what an earlier run installed
#
# Speech Dispatcher is not restarted: that would interrupt the speech you may
# be using right now. The script says how to restart it when you are ready.
#
# ReLoq C, SPDX-License-Identifier: GPL-3.0-or-later

set -eu

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
mode=
uninstall=0
prefix=/usr/local

usage() {
    sed -n '2,11s/^# \{0,1\}//p' "${BASH_SOURCE[0]}"
    printf '%s\n' '' '  --prefix DIR   installation prefix for --system (default /usr/local)'
}

while (( $# > 0 )); do
    case $1 in
        --system) mode=system ;;
        --user) mode=user ;;
        --uninstall) uninstall=1 ;;
        --prefix)
            if (( $# < 2 )); then printf '%s\n' '--prefix requires a directory.' >&2; exit 2; fi
            prefix=$2; shift ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'Unknown option: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done
if [[ -z ${mode} ]]; then
    if (( EUID == 0 )); then mode=system; else mode=user; fi
fi
if [[ ${mode} == system && ${EUID} -ne 0 ]]; then
    printf '%s\n' 'A system installation needs root: sudo ./install.sh --system' >&2
    exit 1
fi

if [[ ${mode} == system ]]; then
    moduleDir=$(pkg-config --variable=modulebindir speech-dispatcher 2>/dev/null || true)
    if [[ -z ${moduleDir} ]]; then
        for candidate in /usr/lib/x86_64-linux-gnu/speech-dispatcher-modules \
                         /usr/lib64/speech-dispatcher-modules /usr/lib/speech-dispatcher-modules \
                         /usr/libexec/speech-dispatcher-modules; do
            if [[ -d ${candidate} ]]; then moduleDir=${candidate}; break; fi
        done
    fi
    moduleDir=${moduleDir%/}
    moduleConfigDir=/etc/speech-dispatcher/modules
    libDir=${prefix}/lib
    includeDir=${prefix}/include/reloq-c
    shareDir=${prefix}/share/reloq-c
    binDir=${prefix}/bin
    enableArgs=(--system)
    removeArgs=(--system)
else
    moduleDir=${HOME}/.local/libexec/speech-dispatcher-modules
    moduleConfigDir=${XDG_CONFIG_HOME:-${HOME}/.config}/speech-dispatcher/modules
    libDir=${HOME}/.local/lib
    includeDir=${HOME}/.local/include/reloq-c
    shareDir=${XDG_DATA_HOME:-${HOME}/.local/share}/reloq-c
    binDir=${HOME}/.local/bin
    # Named explicitly: as root, reloq-speechd-enable would otherwise default
    # to the system configuration.
    userConfigRoot=${XDG_CONFIG_HOME:-${HOME}/.config}/speech-dispatcher
    enableArgs=(--config-dir "${userConfigRoot}" --module "${moduleDir}/sd_reloq"
                --module-config "${moduleConfigDir}/reloq.conf")
    removeArgs=(--config-dir "${userConfigRoot}")
fi
record=${shareDir}/installed-files

if (( uninstall )); then
    if [[ ! -f ${record} ]]; then
        printf 'No ReLoq C installation was found in %s.\n' "${shareDir}" >&2
        exit 1
    fi
    if [[ -x ${binDir}/reloq-speechd-enable ]]; then
        "${binDir}/reloq-speechd-enable" "${removeArgs[@]}" --remove || true
    fi
    while IFS= read -r path; do
        [[ -n ${path} ]] && rm -f -- "${path}"
    done < "${record}"
    rm -f -- "${record}"
    rmdir -- "${shareDir}/dictionaries" "${shareDir}/licenses" "${shareDir}" "${includeDir}" 2>/dev/null || true
    printf 'ReLoq C was removed. Your %s/reloq.conf was kept.\n' "${moduleConfigDir}"
    printf '%s\n' 'Restart Speech Dispatcher when convenient: killall speech-dispatcher'
    exit 0
fi

if [[ ${mode} == system && ( -z ${moduleDir} || ! -d ${moduleDir} ) ]]; then
    printf '%s\n' "Speech Dispatcher's module directory was not found." \
        'Install Speech Dispatcher (version 0.11 or later) first.' >&2
    exit 1
fi
if missing=$(ldd "${here}/sd_reloq" 2>/dev/null | grep 'not found'); then
    printf '%s\n' 'The module cannot start on this system:' "${missing}" \
        'Install the missing system libraries before retrying.' >&2
    exit 1
fi

installed=()
put() { # mode source destination
    install -D -m "$1" -- "$2" "$3"
    installed+=("$3")
}

put 755 "${here}/sd_reloq" "${moduleDir}/sd_reloq"
put 755 "${here}/reloq-speechd-enable" "${binDir}/reloq-speechd-enable"
put 755 "${here}/libreloq_eci.so" "${libDir}/libreloq_eci.so"
put 644 "${here}/include/eci.h" "${includeDir}/eci.h"
put 644 "${here}/include/reloq_eci.h" "${includeDir}/reloq_eci.h"
for file in "${here}"/dictionaries/*; do
    put 644 "${file}" "${shareDir}/dictionaries/${file##*/}"
done
for file in "${here}"/licenses/*; do
    put 644 "${file}" "${shareDir}/licenses/${file##*/}"
done
if [[ -e ${moduleConfigDir}/reloq.conf ]]; then
    printf 'Kept your existing %s/reloq.conf.\n' "${moduleConfigDir}"
else
    mkdir -p -- "${moduleConfigDir}"
    sed "s|^#DictionaryDirectory .*|#DictionaryDirectory ${shareDir}/dictionaries|" \
        "${here}/reloq.conf" > "${moduleConfigDir}/reloq.conf"
    chmod 644 "${moduleConfigDir}/reloq.conf"
fi
printf '%s\n' "${installed[@]}" > "${record}"

printf 'Installed the ReLoq C Speech Dispatcher module in %s.\n' "${moduleDir}"
"${binDir}/reloq-speechd-enable" "${enableArgs[@]}"
printf '%s\n' '' \
    'Choose "reloq" as the speech synthesizer in your screen reader, or try it with:' \
    '  spd-say -o reloq "Hello from ReLoq C"' \
    'Optional pronunciation dictionaries: see DictionaryDirectory in' \
    "  ${moduleConfigDir}/reloq.conf"
