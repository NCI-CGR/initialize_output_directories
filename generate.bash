#!/bin/bash
autoreconf --force --install
CC=gcc CXX=g++ ./configure --prefix=$PWD --with-boost=/home/palmercd/Development/stage --with-boost-libdir=/home/palmercd/Development/stage/lib --with-boost-program-options=boost_program_options
