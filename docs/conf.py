# The sphinx template and setup was mostly adopted from openimageios docs to mimic their styling
import subprocess, os, sys

# This is for local development to properly include the paths
sys.path.insert(0, os.path.abspath("../bin-int/cryptomatte-api/x64-debug/python"))
sys.path.insert(0, os.path.abspath("../bin-int/compressed-image/x64-release/python"))


def configureDoxyfile(input_dir, output_dir):
    with open('Doxyfile.in', 'r') as file :
        filedata = file.read()

    filedata = filedata.replace('@DOXYGEN_INPUT_DIR@', input_dir)
    filedata = filedata.replace('@DOXYGEN_OUTPUT_DIR@', output_dir)

    with open('Doxyfile', 'w') as file:
        file.write(filedata)

# Check if we're running on Read the Docs' servers
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

breathe_projects = {}

if read_the_docs_build:
    print("Detected we are running in Readthedocs")
    input_dir = '../cryptomatte_api'
    output_dir = 'build'
    configureDoxyfile(input_dir, output_dir)
    subprocess.call('doxygen', shell=True)
    breathe_projects['CryptomatteAPI'] = output_dir + '/build'


# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'CryptomatteAPI'
copyright = '2025, Emil Dohne'
author = 'Emil Dohne'
release = '2025'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'sphinx.ext.autodoc',
    'sphinx_inline_tabs',
    'numpydoc'
    ]

numpydoc_show_class_members = False
autodoc_typehints = "none"


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_theme_options = {
    "dark_css_variables": {
        "color-api-background" : "#212121",
        "color-api-background-hover" : "#313131"
    },
}

autodoc_member_order = 'bysource'

# -- Breathe configuration ---------------------------------------------------
breathe_default_project = "CryptomatteAPI"