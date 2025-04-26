[ "${BASH_SOURCE[0]}" == "$0" ] && { >&2 echo "this script is meant to be sourced, not executed"; exit 1; }

function make {
    local MAKE="/usr/bin/make"

    # local EXTRA_ARGS="-j16"

    # disable all macros
    local EXTRA_ARGS="-j16 DISABLE_WORDS=ASSOC,ATOM_QUOT,CBG,DOLLARS_CBG,MSBG,MULTILINE_QUOT,PATH,PG,PPG,PSBG,PSBG_SUFFICES,QUOT,SBG,SBT,SPECIAL_ATOMS,"

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

    local final_cmd=""
    for target in $target_args; do
        final_cmd="${final_cmd}${final_cmd:+ && }${make_prefix} $target"
    done

    echo eval \""${final_cmd:-$make_prefix}"\" #debug
    eval "${final_cmd:-$make_prefix}"
}

# detect broken symbolic links
find . -xtype l -exec echo 'WARN: Broken symlink: {}' \;

## add (back) make autocompletion for our new definition of make
source tools/bash_completion_make # exports _make function
complete -F _make make fastmk

alias fastmk='make -f fast.mk'
