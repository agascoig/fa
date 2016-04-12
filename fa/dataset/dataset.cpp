// dataset.cc - Create training and training test sets.
//
// Copyright 2016 Alexander Gascoigne (agascoig@ieee.org)
//

#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

/* bool myispunct(char c)
{
  return !(isspace(c) || isdigit(c) || isalpha(c));
  } */

typedef vector<string> StrVec;

void emitvector(ostream &ostr, StrVec &v)
{
  for (StrVec::iterator a=v.begin();a!=v.end();++a) {
    ostr << *a << " ";
  }
  ostr << endl;
  ostr.flush();
}

int main(int argc, char *argv[])
{
  random_device rd;
  mt19937_64 gen(rd());
  uniform_int_distribution<int> dis(1,10); // uniform [1..10]

  ofstream outts(argv[1],ofstream::out);
  ofstream outtts(argv[2],ofstream::out);
  long count=0;
  for (int i=3;i<=argc-1;++i) {
    ifstream istr(argv[i]);
    if (istr.fail()) {
      cerr << "Failed to open " << argv[i] << endl;
      exit(1);
    }
    string token;
    StrVec phrase;
    while (!(istr >> token).eof()) {
      count++;
      bool endpunc=ispunct(token[token.size()-1]);
      phrase.push_back(token);
      if (endpunc) {
	int r=dis(gen);
	if (r<=7)
	  emitvector(outts,phrase);
	else
	  emitvector(outtts,phrase);
	phrase.clear();
      }
    }
    phrase.clear();
  }
}
