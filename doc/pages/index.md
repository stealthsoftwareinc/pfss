---
#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

layout: root
---

{% comment %}
//
// This file should roughly mirror the README.adoc file. If you edit
// this file, you might need to edit that file as well.
//
{% endcomment %}

{% include common.liquid %}
{% include section_files.liquid %}

{% if in_source_repo %}
# {{ PACKAGE_NAME }}
{% else %}
# {{ PACKAGE_NAME }} {{ PACKAGE_VERSION }}
{% endif %}

{% for section_file in section_files %}
  {% capture x %}{% include {{ section_file }} %}{% endcapture %}
  {{ x | markdownify }}
{% endfor %}
