#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ac_config_file() {

  local file
  # Bash >=4.2: declare -g sst_ac_config_file_seen
  local target

  for file; do

    sst_expect_source_path "$file"

    if [[ $file == *.im.in ]]; then
      target=${file%.im.in}
    elif [[ $file == *.in ]]; then
      target=${file%.in}
    elif [[ $file == *.im ]]; then
      target=${file%.im}
    elif [[ -f $file.im.in ]]; then
      target=$file
      file=$file.im.in
    elif [[ -f $file.in ]]; then
      target=$file
      file=$file.in
    elif [[ -f $file.im ]]; then
      target=$file
      file=$file.im
    else
      sst_barf "can't figure out how to process %s" $file
    fi

    sst_expect_file $file

    if [[ "${sst_ac_config_file_seen= }" == *" $target "* ]]; then
      continue
    fi

    if [[ $file == *.im.in ]]; then
      sst_ac_append <<<"GATBPS_CONFIG_FILE([$target.im])"
      sst_ac_append <<<"GATBPS_CONFIG_LATER([$target])"
    elif [[ $file == *.in ]]; then
      sst_ac_append <<<"GATBPS_CONFIG_FILE([$target])"
    elif [[ $file == *.im ]]; then
      sst_ac_append <<<"GATBPS_CONFIG_LATER([$target])"
    fi

    sst_ac_config_file_seen+="$file "

  done

}; readonly -f sst_ac_config_file
