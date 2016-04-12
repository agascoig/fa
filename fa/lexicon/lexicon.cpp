//
//  lexicon.cpp
//  featureattractor project: for Coursera Data Science Capstone/Swiftkey Project
//
//  Created by Alexander Gascoigne on 3/20/16.
//  Copyright © 2016 Alexander Gascoigne. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

typedef map<string,int> WordType;

template <typename T1, typename T2>
struct less_second {
    typedef pair<T1, T2> type;
    bool operator ()(type const& a, type const& b) const {
        return a.second < b.second;
    }
};
int nonalpha(char c) { return !isalpha(c); }

int main(int argc, const char * argv[]) {

    string token;
    WordType wmap;
    
    // get number of words to use
    const int WORDCOUNT=atoi(argv[1]);

    while (!(cin >> token).eof()) {
        // remove punctuation
        token.erase(remove_if(token.begin(), token.end(), ::ispunct), token.end());
        // make lower case
        transform(token.begin(), token.end(), token.begin(), ::tolower);
        // remove any non-alphabetic characters (including numbers)
        token.erase(remove_if(token.begin(), token.end(), ::nonalpha), token.end());
        
        if (!token.empty())
            wmap[token]++; // if token doesn't exist, will initialize to zero
    }

    // remove profanity words
    std::ifstream istr(argv[2]);
    std::string word;
    while (!(istr >> word).eof()) {
      transform(word.begin(), word.end(), word.begin(), ::tolower);
      if (wmap.find(word)!=wmap.end())
	wmap.erase(word);
    }
    
    // sort it
    typedef vector<pair<string, int> > VecType;
    VecType mapcopy(wmap.begin(), wmap.end());
    sort(mapcopy.begin(), mapcopy.end(), less_second<string, int>());

    // choose the top WORDCOUNT for output
    long numwords=(mapcopy.size()<WORDCOUNT ? mapcopy.size() : WORDCOUNT);
    
    // sort the vocabulary
    vector<string> vocab;
    for (VecType::iterator v=mapcopy.end()-numwords;v!=mapcopy.end();++v) {
        vocab.push_back(v->first);
    }
    sort(vocab.begin(),vocab.end());

    // output maximum and minimum frequency in vocabulary
    cerr << "Most frequent word count: " << mapcopy[mapcopy.size()-1].second << endl;
    cerr << "Least frequent word count: " << mapcopy[mapcopy.size()-1-WORDCOUNT].second << endl;

    // output to stdout
    for (vector<string>::iterator v=vocab.begin();v!=vocab.end();++v) {
        cout << *v << endl;
    }
    return 0;
}
