//
//  tester.cpp - run the predictor
//  Coursera Swiftkey Data Science Project
//
//  Created by Alexander Gascoigne on 3/20/16.
//  Copyright © 2016 Alexander Gascoigne. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <numeric>
#include "readvocab.hpp"

using namespace std;

extern void initp(string , int);
extern void predict(const string &, string &, string &, string &, string &);
extern void print_coefs(void);

extern string clean_string(string &s);

static int DEBUGFLAG=0;

int main(int argc, const char * argv[]) {
  // argv[1] : debug flag
  // argv[2] : ngram length
  // argv[3] : path to prediction matrices, vocabulary

  DEBUGFLAG=atoi(argv[1]);

  const int ngramlength=atoi(argv[2]);

  initp(argv[3],ngramlength); // get matrices setup

  int predictcount=0, successcount=0, wordcount=0;

  string word, predictstr;

  bool empty=true;
  while (!(cin >> word).eof()) {
    bool end_punct=ispunct(word[word.size()-1]);
    word=clean_string(word);

    if (!word.empty()) {
      wordcount++;
      if (!empty) {
	string predict1, predict2, predict3; // get clean prediction string
	predict(predictstr,predict1, predict2, predict3, word);
	predictcount++;
	if ((!predict1.compare(word)) || (!predict2.compare(word)) || (!predict3.compare(word)))
	  successcount++;

	if (DEBUGFLAG)
	  cerr << predictstr << " [" << predict1 
	       << " " << predict2 << " " << predict3 << "] " << word << endl;

	predictstr.append(" ");
	predictstr.append(word);
      } else {
	predictstr.append(word);
	empty=false;
      }
    }
    if (end_punct) {
      empty=true;
      predictstr.clear(); // start over if we see punctuation
    }
    if ((wordcount%100000)==0) {
      cerr << "performance: " << wordcount << " " << predictcount << " " << successcount << " " << endl;
      print_coefs();
    }
  }
  cerr << "performance: " << wordcount << " " << predictcount << " " << successcount << " " << endl;
  return 0;
}
