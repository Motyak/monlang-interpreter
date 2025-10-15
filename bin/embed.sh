#!/bin/bash
set -o errexit
set -o nounset

ELF_FILE="$1"
SRC_FILE="$2"

src_size="$(wc -c < "$SRC_FILE")"
cat "$ELF_FILE" "$SRC_FILE" <(printf "monlang") <(
    printf '0: %.8x' $src_size \
    | sed -E 's/0: (..)(..)(..)(..)/0: \4\3\2\1/' \
    | xxd -r -g0
) > "${SRC_FILE%.ml}.elf"

chmod +x "${SRC_FILE%.ml}.elf"
