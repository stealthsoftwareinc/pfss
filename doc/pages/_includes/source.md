{%- comment -%}
//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//
{%- endcomment -%}

{%- comment -%}
//
// This file should roughly mirror the doc/readme/source.adoc file. If
// you edit this file, you might need to edit that file as well.
//
{%- endcomment -%}

## Source

{% assign fp_artifact = "source" %}
{% assign fp_builtin_view_latest_url = "" %}
{% assign fp_builtin_view_pinned_url = "" %}
{% assign fp_builtin_download_latest_url = "" %}
{% assign fp_builtin_download_pinned_url = "" %}
{% assign fp_github_view_latest_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/tree/master" %}
{% assign fp_github_view_pinned_url = "" %}
{% assign fp_github_download_latest_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/archive/refs/heads/master.tar.gz" %}
{% assign fp_github_download_pinned_url = "" %}
{% assign fp_gitlab_view_latest_url = "" %}
{% assign fp_gitlab_view_pinned_url = "" %}
{% assign fp_gitlab_download_latest_url = "" %}
{% assign fp_gitlab_download_pinned_url = "" %}
{% assign fp_devel_view_latest_url = "https://gitlab.stealthsoftwareinc.com/stealth/" | append: PACKAGE_TARNAME | append: "/-/tree/master" %}
{% assign fp_devel_view_pinned_url = "" %}
{% assign fp_devel_download_latest_url = "" %}
{% assign fp_devel_download_pinned_url = "" %}
{% unless in_source_repo %}
{% assign fp_github_download_pinned_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/archive/refs/tags/v" | append: PACKAGE_VERSION | append: ".tar.gz" %}
{% endunless %}

{% include artifact_links_fragment.md %}
