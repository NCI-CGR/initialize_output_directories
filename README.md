# initialize\_output\_directories

## Overview

This program is a reimplementation and improvment of the old `plco-analysis/shared-source/construct_output_filenames.bash`.
That script was originally conceived of as a simple tool to build up paths and file prefixes for valid combinations
of analysis parameters. In time, the needs placed on that script increased, until such time as it was a whole preprocesor,
handling configuration file parsing, tracking files, and assorted tasks. That was generally fine, but eventually the
requirements for per-analysis configuration became too great to straightforwardly support in bash's syntax. Therefore
this project was born: a rebuild, from the ground up, in C++ with boost. It's also _wildly_ faster, thanks to some
nifty file access optimizations when interacting with the phenotype database backend.

This has absolutely no applications outside of the context of `plco-analysis`, but it's pretty cool anyway! Thanks for
reading.

## Dependencies

This is a work in progress! The current list of dependencies is as follows:

 - gcc/g++ with `-std=c++17` support
 - `make` and associated tools; should be ok on most Linux systems
 - [boost](www.boost.org), v1.63.0 or greater. within boost:
   - base headers, of course
   - `program_options`
   - `filesystem`
   - `system`
   - `iostreams`
 - [yaml-cpp](https://github.com/jbeder/yaml-cpp)
 - if developing and changing build parameters, `autoconf`/`automake` 

## Installation

First, clone the repository:

`git clone http://10.133.130.114/PLCO-Atlas-Project/initialize_output_directories.git`

Navigate into the directory:

`cd initialize_output_directories`

Run the standard configure/make/make check process:

`./configure`

Note that depending on your boost installation, you may need to add flags to `configure`: `--with-boost=PATH`, `--with-boost-libdir=PATH`, `--with-boost-program-options=PATH`. Additionally, `configure` doesn't always play nice with versions of compilers present in your global Linux directories and conda environments; you may then need to prepend your `configure` command as follows: `CC=gcc CXX=g++ ./configure [...]`.

`make`

`make check`

`make check` deploys a series of automated tests; make sure they all pass! Once that's complete, you can technically run `make install` if you wish, but you probably don't want to install this software into global directories or `./configure --prefix` targets; instead, you can place the full path and executable within the current working directory into `plco-analysis/Makefile.config`.

## Usage

The following are supported options for this software:

 - -h [ --help ]: emit a help message describing available options
 - -e [ --extension-config ] `arg`: file extension configuration file, yaml format
 - -B [ --force ]: force updates to all tracking files unless in pretend mode
 - -p [ --phenotype-config ] `arg`: phenotype configuration file, yaml format
 - -D [ --phenotype-database ] `arg`: name of current phenotype dataset in use
 - -I [ --phenotype-id-colname ] `arg`: column header for subject IDs in phenotype dataset
 - -n [ --pretend ]: emit analysis target directories but do not write any changes to disk
 - -b [ --bgen-dir ] `arg`: top level directory containing imputed bgen files
 - -r [ --results-dir ] `arg`: top level directory containing analysis results
 - -s [ --software ] `arg`: requested software (e.g. SAIGE, BOLTLMM)
 - -N [ --software-min-sample-size ] `arg`: minimum heuristic sample size for software
 - -t [ --timer ]: emit elapsed runtime at end of program execution

## Version History

17 December 2020: cloned repo into new `PLCO-Atlas-Project` GitLab group, and finally wrote the README lol
