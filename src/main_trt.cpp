#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <getopt.h>
#include <algorithm>
#include <map>

#include "tree.h"
#include "tree_reader.h"
#include "utils.h"
#include "tree_utils.h"
#include "log.h"
#include "constants.h"

extern std::string PHYX_CITATION;


void print_help() {
    std::cout << "This will trace a big tree given a taxon list and and produce newick." << std::endl;
    std::cout << "Data can be read from a file or STDIN." << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: pxtrt [OPTIONS]..." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << " -t, --treef=FILE     input tree file, STDIN otherwise" << std::endl;
    std::cout << " -n, --names=CSL      names sep by commas (NO SPACES!)" << std::endl;
    std::cout << " -f, --namesf=FILE    names in a file (each on a line)" << std::endl;
    std::cout << " -c, --comp           take the complement (i.e. remove any taxa not in list)" << std::endl;
    std::cout << " -o, --outf=FILE      output tree file, STOUT otherwise" << std::endl;
    std::cout << " -s, --silent         suppress warnings of missing tips" << std::endl;
    std::cout << " -h, --help           display this help and exit" << std::endl;
    std::cout << " -V, --version        display version and exit" << std::endl;
    std::cout << " -C, --citation       display phyx citation and exit" << std::endl;
    std::cout << std::endl;
    std::cout << "Report bugs to: <https://github.com/FePhyFoFum/phyx/issues>" << std::endl;
    std::cout << "phyx home page: <https://github.com/FePhyFoFum/phyx>" << std::endl;
}

std::string versionline("pxtrt 1.1\nCopyright (C) 2017-2020 FePhyFoFum\nLicense GPLv3\nWritten by Stephen A. Smith (blackrim), Joseph W. Brown");

static struct option const long_options[] =
{
    {"treef", required_argument, NULL, 't'},
    {"names", required_argument, NULL, 'n'},
    {"comp", no_argument, NULL, 'c'},
    {"outf", required_argument, NULL, 'o'},
    {"silent", required_argument, NULL, 's'},
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'V'},
    {"citation", no_argument, NULL, 'C'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char * argv[]) {
    
    log_call(argc, argv);
      
    bool fileset = false;
    bool namesset = false;
    bool namefileset = false;
    bool complement = false;
    bool outfileset = false;
    bool silent = false;
    std::vector<std::string> names;

    char * treef = NULL;
    char * outf = NULL;
    char * namesc = NULL;
    char * namesfc = NULL;
    while (1) {
        int oi = -1;
        int c = getopt_long(argc, argv, "t:n:cf:o:shVC", long_options, &oi);
        if (c == -1) {
            break;
        }
        switch(c) {
            case 't':
                fileset = true;
                treef = strdup(optarg);
                check_file_exists(treef);
                break;
            case 'n':
                namesset = true;
                namesc = strdup(optarg);
                break;
            case 'f':
                namefileset = true;
                namesfc = strdup(optarg);
                check_file_exists(namesfc);
                break;
            case 'c':
                complement = true;
                break;
            case 'o':
                outfileset = true;
                outf = strdup(optarg);
                break;
            case 's':
                silent = true;
                break;
            case 'h':
                print_help();
                exit(0);
            case 'V':
                std::cout << versionline << std::endl;
                exit(0);
            case 'C':
                std::cout << PHYX_CITATION << std::endl;
                exit(0);
            default:
                print_error(argv[0], (char)c);
                exit(0);
        }
    }
    
    if (fileset && outfileset) {
        check_inout_streams_identical(treef, outf);
    }
    
    if (namesset == true) {
        std::vector<std::string> tokens2;
        std::string del2(",");
        tokens2.clear();
        tokenize(namesc, tokens2, del2);
        for (unsigned int j=0; j < tokens2.size(); j++) {
            trim_spaces(tokens2[j]);
            names.push_back(tokens2[j]);
        }
    } else if (namefileset == true) {
        std::ifstream nfstr(namesfc);
        std::string tline;
        while (getline(nfstr, tline)) {
            trim_spaces(tline);
            names.push_back(tline);
        }
        nfstr.close();
    } else {
        std::cerr << "Error: you need to set the names of the tips you want to remove (-n). Exiting." << std::endl;
        exit(0);
    }

    std::istream * pios = NULL;
    std::ostream * poos = NULL;
    std::ifstream * fstr = NULL;
    std::ofstream * ofstr = NULL;
    
    if (fileset == true) {
        fstr = new std::ifstream(treef);
        pios = fstr;
    } else {
        pios = &std::cin;
        if (check_for_input_to_stream() == false) {
            print_help();
            exit(1);
        }
    }
    if (outfileset == true) {
        ofstr = new std::ofstream(outf);
        poos = ofstr;
    } else {
        poos = &std::cout;
    }
    
    //read trees 
    std::string retstring;
    int ft = test_tree_filetype_stream(*pios, retstring);
    if (ft != 0 && ft != 1) {
        std::cerr << "Error: this really only works with nexus or newick. Exiting" << std::endl;
        exit(0);
    }
    bool going = true;
    if (!complement) {
        if (ft == 0) {
            // nexus
            std::map<std::string, std::string> translation_table;
            bool ttexists;
            ttexists = get_nexus_translation_table(*pios, &translation_table, &retstring);
            Tree * tree;
            while (going) {
                tree = read_next_tree_from_stream_nexus(*pios, retstring, ttexists,
                    &translation_table, &going);
                if (going == true) {
                    tree = get_induced_tree(tree, names, silent);
                    (*poos) << getNewickString(tree) << std::endl;
                    delete tree;
                }
            }
        } else if (ft == 1) {
            // newick
            Tree * tree;
            while (going) {
                tree = read_next_tree_from_stream_newick(*pios, retstring, &going);
                if (going == true) {
                    tree = get_induced_tree(tree, names, silent);
                    (*poos) << getNewickString(tree) << std::endl;
                    delete tree;
                }
            }
        }
    } else {
        // don't assume all trees have the same leaf set
        std::vector<std::string> toKeep;
        if (ft == 0) {
            std::map<std::string, std::string> translation_table;
            bool ttexists;
            ttexists = get_nexus_translation_table(*pios, &translation_table, &retstring);
            Tree * tree;
            while (going) {
                tree = read_next_tree_from_stream_nexus(*pios, retstring, ttexists,
                    &translation_table, &going);
                if (going == true) {
                    toKeep = get_complement_tip_set(tree, names);
                    if (toKeep.size() > 1) {
                        tree = get_induced_tree(tree, toKeep, silent);
                        (*poos) << getNewickString(tree) << std::endl;
                    }
                    delete tree;
                }
            }
        } else if (ft == 1) {
            Tree * tree;
            while (going) {
                tree = read_next_tree_from_stream_newick(*pios, retstring, &going);
                if (going == true) {
                    toKeep = get_complement_tip_set(tree, names);
                    if (toKeep.size() > 1) {
                        tree = get_induced_tree(tree, toKeep, silent);
                        (*poos) << getNewickString(tree) << std::endl;
                    }
                    delete tree;
                }
            }
        }
    }
    
    if (fileset) {
        fstr->close();
        delete pios;
    }
    if (outfileset) {
        ofstr->close();
        delete poos;
    }
    return EXIT_SUCCESS;
}
