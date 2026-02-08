[ "${BASH_SOURCE[0]}" == "$0" ] && { >&2 echo "this script is meant to be sourced, not executed"; exit 1; }

function CXX {
    # ccache \
    g++ \
    -Og -fno-inline -fno-reorder-blocks -fno-thread-jumps -fno-ipa-pure-const -fno-tree-ter -fno-tree-ccp -fno-tree-dce -fno-tree-fre -fno-tree-copy-prop -fno-tree-sink -fno-tree-slsr -fno-cprop-registers -fno-split-wide-types -fno-tree-ch -fno-tree-dominator-opts -fno-ipa-reference-addressable -fno-compare-elim -fno-ipa-profile -fno-ipa-reference -fno-toplevel-reorder -fno-guess-branch-probability -fno-forward-propagate -fno-defer-pop -fno-tree-builtin-call-dce -fno-combine-stack-adjustments -fno-shrink-wrap \
    "$@"
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
