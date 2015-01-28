#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <fstream>

using namespace std;

#include "utils.h"
#include "sequence.h"
#include "seq_reader.h"
#include "concat.h"

SequenceConcatenater::SequenceConcatenater (string & seqf):numPartitions(0), numChar(0), numTaxa(0), ft(0) {
    read_sequences(seqf);
}

SequenceConcatenater::SequenceConcatenater ():numPartitions(0), numChar(0), numTaxa(0), ft(0) {

}

void SequenceConcatenater::read_sequences (string & seqf) {
    filename = seqf;
    string retstring;
    istream* pios = new ifstream(filename);
    ft = test_seq_filetype_stream(*pios, retstring);
    Sequence seq;
    int counter = 0;
    int length = 0;

    cout << "ft = " << ft << "; retstring = " << retstring << endl;
    if (ft == 1) { // phylip
        vector <string> fileDim;
        tokenize(retstring, fileDim, " ");
        numTaxa = stoi(fileDim[0]);
        numChar = stoi(fileDim[1]);

        //cout << "numTaxa = " << numTaxa << "; numChar = " << numChar << "." << endl;

        while (read_next_seq_from_stream(*pios, ft, retstring, seq)) {
            length = (int)seq.get_sequence().size();
            if (length != numChar) {
                cout << "Sequence '" << seq.get_id() << "' has " << length << " characters, but the file '"
                    << filename << "' specified " << numChar << "characters. Exiting." << endl;
                delete pios;
                exit(1);
            }
            seqs.push_back(seq);
            counter++;
        }
        if (counter != numTaxa) {
            cout << "Read " << counter << " taxa, but the file '" << filename << "' specified "
                << numTaxa << "taxa. Exiting." << endl;
            delete pios;
            exit(1);
        }
    } else { // will need error checking for other file types
        bool first = true;
        while (read_next_seq_from_stream(*pios, ft, retstring, seq)) {
            int curr = (int)seq.get_sequence().size();
            if (!first) {
                if (curr != length) {
                    cout << "Error: current sequence has " << curr << " characters, but previous sequence had "
                        << length << " characters. Exiting." << endl;
                    delete pios;
                    exit(1);
                }
            } else {
                length = curr;
                first = false;
            }
            seqs.push_back(seq);
            cout << "Taxon: " << seq.get_id() << endl;
            counter++;
        }
        numTaxa = counter;
        numChar = length;
    }
    numPartitions = 1;
    partitionSizes.push_back(numChar);
    delete pios;
}

// where stuff actually happens
void SequenceConcatenater::concatenate(SequenceConcatenater & newSeqs) {
    string old_filler(numChar, 'N');
    string new_filler(newSeqs.get_sequence_length(), 'N');

    for (int i = 0; i != numTaxa; i++) {
        bool match_found = false;
        if (newSeqs.numTaxa > 0) {
            for (int j = 0; j != newSeqs.numTaxa; j++) {
                if (seqs[i].get_id() == newSeqs.seqs[j].get_id()) {
                    seqs[i].set_sequence(seqs[i].get_sequence() + newSeqs.seqs[j].get_sequence());
                    match_found = true;
                    // erase matched entry so it won't have to be compared against again.
                    // erase in reverse order.
                    delete_sequence(newSeqs, j);
                    break;
                }
            }
        }
        if (!match_found) { // taxon is missing from present locus.
            seqs[i].set_sequence(seqs[i].get_sequence() + new_filler);
        }
    }

    // now, all that should be left are the novel sequences from the new file
    if (newSeqs.numTaxa > 0) {
        for (int i = 0; i != newSeqs.numTaxa; i++) {
            newSeqs.seqs[i].set_sequence(old_filler + newSeqs.seqs[i].get_sequence());
            seqs.push_back(newSeqs.seqs[i]);
            numTaxa++;
        }
    }
}

int SequenceConcatenater::get_sequence_length () {
    return numChar;
}

int SequenceConcatenater::get_num_taxa () {
    return numTaxa;
}

void SequenceConcatenater::delete_sequence (SequenceConcatenater & newSeqs, int const& index) {
    newSeqs.seqs.erase(newSeqs.seqs.begin() + index);
    newSeqs.numTaxa--;
}

Sequence SequenceConcatenater::get_sequence (int const & index) {
    return seqs[index];
}