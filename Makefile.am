#AM_CPPFLAGS = -I$(srcdir)/fileinterface -I$(srcdir)/fileinterface/include
bin_PROGRAMS = initialize_output_directories.out
initialize_output_directories_out_SOURCES = initialize_output_directories/cargs.cc initialize_output_directories/cargs.h initialize_output_directories/main.cc initialize_output_directories/utilities.cc initialize_output_directories/utilities.h initialize_output_directories/yaml_reader.cc initialize_output_directories/yaml_reader.h
initialize_output_directories_out_CXXFLAGS = $(BOOST_CPPFLAGS) -ggdb -Wall -std=c++17
initialize_output_directories_out_LDADD = $(BOOST_LDFLAGS) -lboost_program_options -lyaml-cpp
dist_doc_DATA = README
ACLOCAL_AMFLAGS = -I m4
