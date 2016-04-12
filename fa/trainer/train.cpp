//
//  train.cpp - Train feature attractor network
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
#include <sstream>
#include <cmath> // for log
#include "train.hpp"
#include "readvocab.hpp"
#include "cppmatrix.hpp"

using namespace std;

int nonalpha(char c) { return !isalpha(c); }

int cogencymode=1;
int ngramlength=0; // not including the target word

typedef BigMatrix<int> Array2D;

//typedef SparseMatrix<int> Array2D;

static const int BLANK=-1;

void initsr(int *sr)
{
  for (int i=0;i<(ngramlength+1);++i)
    sr[i]=BLANK;
}

void shiftleft(int *sr)
{
  // sr[0] sr[1] sr[2] sr[3] sr[4]
  for (int i=0;i<ngramlength;++i)
    sr[i]=sr[i+1];
  sr[ngramlength]=BLANK;
}

bool onlytarget(int *sr)
{
  for (int i=0;i<ngramlength;++i)
    if (sr[i]!=BLANK)
      return false;
  return true;
}

void printsr(int *sr)
{
  for (int i=0;i<ngramlength;++i)
    cerr << sr[i] << " ";
  cerr << endl;
}

void updatecount(Array2D &matrix, int source, int target)
{
  if (source==BLANK)
    return; // do nothing if blank
  int &cell=matrix(source,target); // source is row, target is column
  cell=cell+1;
}

void output_prob_matrix(ostream &ostr, Array2D &matrix, int *targetcount)
{
  int sz=matrix.size();
  
  for (int i=0;i<sz;++i) { // row i: a source
    double den=0.0;
    if (!cogencymode)
      den=matrix.row_sum(i);
    for (int j=0;j<sz;++j) // column j: a target
      {
	if (matrix.nonzero(i,j)) {
	  if (cogencymode)
	    den=targetcount[j];
	  int &cell=matrix(i,j);
	  if (cell>=2) {
	    float prob;
	    prob=(double)cell/den;
	    if (prob>0.001) {
	      prob=log(prob);
	      ostr << i << " " << j << " " << prob << endl;
	    }
	  }
	}
      }
  }
}

void output_target_array(ostream &ostr, int *targetcount, int size)
{
  double totalcount=0;
  for (int i=0;i<size;++i)
    totalcount+=targetcount[i];
  
  for (int i=0;i<size;++i) {
    float prob=(double)targetcount[i]/totalcount;
    prob=log(prob);
    ostr << prob << endl;
  }
}

string clean_string(string &s)
{
  string d;
  for (string::iterator si=s.begin();si!=s.end();++si) {
    char c=*si;
    c=tolower(c);
    if (c>='a' && c<='z')
      d+=c;
  }
  return d;
}

int main(int argc, const char * argv[]) {
  // argv[1] : ngram length
  // argv[2] : 1 = cogency, 0 = aristotelian
  // argv[3] : path to matrices, array, vocabulary

  ifstream ifs;
  vector<string> vocab;

  ngramlength=atoi(argv[1]);
  cogencymode=atoi(argv[2]);
  string outputpath=argv[3];

  cerr << "reading vocabulary" << endl;

  string vocabname=outputpath+"/vocab.dat";
  ifs.open(vocabname.c_str());
  if (ifs.fail()) {
    cerr << "Could not read vocabulary: " << vocabname << endl;
    exit(1);
  }
  read_vocabulary(ifs, vocab);
  ifs.close();

  cerr << "read vocabulary: " << vocab.size() << endl;

  ofstream *outfiles=new ofstream[ngramlength];

  for (int i=0;i<ngramlength;++i) {
    stringstream fn;
    fn << outputpath << "/out" << i << ".dat";
    
    outfiles[i].open((fn.str()).c_str());
    if (outfiles[i].fail()) {
      cerr << "Couldn't open ngram file " << i << " for output." << endl;
      exit(1);
    }
  }

  ofstream outtarget((outputpath+"/outtarget.dat").c_str());

  const int UNKNOWN_0=vocab.size(); // unknown zero based
  const int UNKNOWN_1=UNKNOWN_0+1; // unknown ones based

  int sr[ngramlength]; // token shift register
  initsr(sr);

  Array2D *matrices[ngramlength];

  // initializing matrices

  cerr << "initializing matrices" << endl;

  for (int i=0;i<ngramlength;++i)
    matrices[i]=new Array2D(UNKNOWN_1, UNKNOWN_1);

  int targetcount[UNKNOWN_1];
  for (int i=0;i<UNKNOWN_1;++i)
    targetcount[i]=0;

  cerr << "done initializing matrices" << endl;

  string token;

  long wordcount=0;

  while (!(cin >> token).eof()) {

    bool end_punct=ispunct(token[token.size()-1]);

    token=clean_string(token);
    
    int &target=sr[ngramlength]; // get alias to target register
    
    if (!token.empty()) {
      wordcount++;
      if ((wordcount%1000000)==0)
	cerr << "Word count: " << wordcount << endl;
      
      shiftleft(sr); // make space for new target
      target=get_token_id(vocab,token); // add the new target
      
    }
    
    if (!onlytarget(sr)) {
      for (int i=0;i<ngramlength;++i)
	updatecount(*matrices[i],sr[i],target);
      targetcount[target]++;
    }

    if (end_punct)
      initsr(sr); // clear the shift register if we saw punctuation

  }

  cerr << "Outputing ngram sparse matrices." << endl;

  for (int i=0;i<ngramlength;++i)
    output_prob_matrix(outfiles[i],*matrices[i],targetcount);

  output_target_array(outtarget,targetcount,UNKNOWN_1);

  cerr << "Deleting stuff, closing files." << endl;

  for (int i=0;i<ngramlength;++i) {
    outfiles[i].close();
    delete matrices[i];
  }
  delete [] outfiles;

}
