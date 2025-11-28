#!/bin/bash
set -o nounset

## duplicate stdout to stderr ##
exec 6>&1 > >(tee -a /dev/stderr)
trap 'exec 1>&6 6>&-' EXIT

echo "create $1"

for module in "${@:2}"; do
    if [[ "$module" =~ .o$ ]]; then
        echo "addmod $module"
    elif [[ "$module" =~ .a$ ]]; then
        echo "addlib $module"
    else
        >&2 echo "unsupported file extension: $module"
        exit 1
    fi
done

echo "save"
echo "end"
