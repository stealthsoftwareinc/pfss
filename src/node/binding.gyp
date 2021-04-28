#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

{
  "targets": [
    {
      "target_name": "pfss",
      "sources": [
        "pfss-node.cpp",
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
      ],
      "libraries": [
        "-lpfss",
        "-lnettle",
      ],
    },
  ],
}
