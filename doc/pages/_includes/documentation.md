{%- comment -%}
//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//
{%- endcomment -%}

{%- comment -%}
//
// This file should roughly mirror the doc/readme/documentation.adoc
// file. If you edit this file, you might need to edit that file as
// well.
//
{%- endcomment -%}

## Documentation

{% assign fp_artifact = "manual" %}
{% assign fp_builtin_view_latest_url = "manual" %}
{% assign fp_builtin_view_pinned_url = "" %}
{% assign fp_builtin_download_latest_url = PACKAGE_TARNAME | append: "-manual.tar.xz" %}
{% assign fp_builtin_download_pinned_url = "" %}
{% assign fp_github_view_latest_url = "https://stealthsoftwareinc.github.io/" | append: PACKAGE_TARNAME | append: "/manual" %}
{% assign fp_github_view_pinned_url = "" %}
{% assign fp_github_download_latest_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/raw/master/doc/manual/" | append: PACKAGE_TARNAME | append: "-manual.tar.xz" %}
{% assign fp_github_download_pinned_url = "" %}
{% assign fp_gitlab_view_latest_url = "" %}
{% assign fp_gitlab_view_pinned_url = "" %}
{% assign fp_gitlab_download_latest_url = "" %}
{% assign fp_gitlab_download_pinned_url = "" %}
{% assign fp_devel_view_latest_url = "https://www.stealthsoftwareinc.io/" | append: PACKAGE_TARNAME | append: "/manual" %}
{% assign fp_devel_view_pinned_url = "" %}
{% assign fp_devel_download_latest_url = "https://www.stealthsoftwareinc.io/" | append: PACKAGE_TARNAME | append: "/" | append: PACKAGE_TARNAME | append: "-manual.tar.xz" %}
{% assign fp_devel_download_pinned_url = "" %}
{% unless in_source_repo %}
{% assign fp_github_download_pinned_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/raw/v" | append: PACKAGE_VERSION | append: "/doc/" | append: PACKAGE_TARNAME | append: "-" | append: PACKAGE_VERSION | append: "-manual.tar.xz" %}
{% endunless %}

{% include artifact_links_fragment.md %}
