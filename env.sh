[ "${BASH_SOURCE[0]}" == "$0" ] && { >&2 echo "this script is meant to be sourced, not executed"; exit 1; }

# this is meant to be used by a Makefile, that defines $CXX_ and $CXX_OFLAGS env variables
function CXX {
    ${CXX_:-ccache g++} ${CXX_OFLAGS:-} "$@"
}
export -f CXX

function make {
    local MAKE="/usr/bin/make"
    local EXTRA_ARGS="-j16"

    local target_args="$($MAKE --no-print-directory -q -E '$(info $(MAKECMDGOALS))' "$@" | head -n1)"
    local opt_args=""

    # find non-target arguments, append them to opt_args
    for arg in "$@"; do
        [[ " $target_args" =~ \ $arg ]] && continue
        opt_args="${opt_args}${opt_args:+ }'${arg}'"
    done

    # echo "DEBUG opt args: \`$opt_args\`" #debug
    # echo "DEBUG target args: \`$target_args\`" #debug

    local make_prefix="${MAKE}${EXTRA_ARGS:+ }${EXTRA_ARGS}${opt_args:+ }${opt_args}"

    case "$make_prefix" in
    *\ -q* | *\ --question*)
        echo eval \""${make_prefix}${target_args:+ }${target_args}"\" #debug
        eval "${make_prefix}${target_args:+ }${target_args}"
        return $?
        ;;
    esac

    local build_cmd=""
    for target in $target_args; do
        build_cmd="${build_cmd}${build_cmd:+ && }${make_prefix} ${target}"
    done
    local rm_cmd=""
    if [ -n "$__REMAKE" ]; then
        if [ -n "$build_cmd" ]; then
            rm_cmd="rm -rf ${target_args}"
        else
            make_prefix="${make_prefix} -B"
        fi
    fi
    local final_cmd="${rm_cmd}${rm_cmd:+; }${build_cmd}"

    echo eval \""${final_cmd:-$make_prefix}"\" #debug
    if [ -n "$rm_cmd" ]; then
        >&2 echo -n "proceed?(Y/n) >"
        read confirm
        [[ "$confirm" =~ n|N ]] && >&2 echo "aborted" && return
    fi
    eval "${final_cmd:-$make_prefix}"
}

alias remake='__REMAKE=x make'

# detect broken symbolic links
find . -xtype l -exec echo 'WARN: Broken symlink: {}' \;

## add (back) make autocompletion for our new definition of make
source tools/bash_completion_make # exports _make function
complete -F _make make remake
