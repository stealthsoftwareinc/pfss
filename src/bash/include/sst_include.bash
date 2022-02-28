#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_include() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  declare    array_flag
  declare    declarations
  declare    definitions
  declare    file
  declare    includes
  declare    variable

  includes=
  declarations=
  definitions=
  for file; do
    includes+=" . $(sst_quote "$file");"
    file=${file##*/}
    if [[ "$file" == define_* ]]; then
      variable=${file#define_}
      variable=${variable%%.*}
      if [[ "$variable" == *-[Aa] ]]; then
        array_flag=" -${variable##*-}"
        variable=${variable%-*}
      else
        array_flag=
      fi
      if ((!SST_NDEBUG)); then
        sst_expect_basic_identifier "$variable"
      fi
      declarations+=" declare$array_flag $variable;"
      declarations+=" declare ${variable}_is_set;"
      definitions+=" define_$variable;"
    fi
  done
  readonly includes
  readonly declarations
  readonly definitions

  printf '%s\n' "$includes$declarations$definitions"

}; readonly -f sst_include
