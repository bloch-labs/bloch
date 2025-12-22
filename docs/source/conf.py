# Configuration file for the Bloch Sphinx documentation.
#
# This project uses MyST Markdown for content. Both `.md` and `.rst` sources
# are supported to ease migration from the existing site.

import datetime

project = "Bloch"
author = "Bloch Labs"
current_year = datetime.datetime.now().year
copyright = f"{current_year}, {author}"

# Keep these in sync with release tags when enabling versioned docs.
version = "1.0"
release = "1.0.2"

extensions = [
    "myst_parser",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx_design",
    "sphinx_multiversion",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]
source_suffix = {".rst": "restructuredtext", ".md": "markdown"}

language = "en"
nitpicky = False  # switch to True once references are stabilized

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "linkify",
    "substitution",
    "tasklist",
]

html_theme = "furo"
html_title = "Bloch Documentation"
html_static_path = ["_static"]
html_css_files = ["custom.css"]

html_theme_options = {
    "sidebar_hide_name": True,
    "navigation_with_keys": True,
    "light_css_variables": {
        "color-brand-primary": "#913DD9",
        "color-brand-content": "#913DD9",
    },
    "dark_css_variables": {
        "color-brand-primary": "#B987EB",
        "color-brand-content": "#B987EB",
    },
}

# Example substitutions for reuse across pages.
myst_substitutions = {
    "latest_version": release,
}

# Sphinx-multiversion: build tagged releases plus the current branch.
smv_tag_whitelist = r"^v1\.0\.(0|1|2)$"
smv_branch_whitelist = r"^(main|master)$"
smv_remote_whitelist = r"^origin$"
smv_latest_version = "v1.0.2"
smv_rename_latest_version = "latest"
