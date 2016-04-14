
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <vector>
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;

void read_vocabulary(istream &is, vector<string> &vocab)
{
        string token;
        
        while (!(is >> token).eof()) {
                vocab.push_back(token);
        }
}

long get_token_id(vector<string> &vocab, const string &token)
{
        vector<string>::iterator i=lower_bound(vocab.begin(),vocab.end(),token);
        if ((i==vocab.end()) || (*i!=token))
                return vocab.size();
        return distance(vocab.begin(),i);
}

string get_string_from_id(const vector<string> &vocab, int id)
{
        if (id==vocab.size())
                return "UNKNOWN";
        return vocab[id];
}

template <class T>
class SparseMatrix
{
public:
        typedef map<size_t, map<size_t , T> > mat_t;
        typedef typename mat_t::iterator row_iter;
        typedef typename mat_t::iterator const const_row_iter;
        typedef map<size_t, T> col_t;
        typedef typename col_t::iterator col_iter;
        typedef typename col_t::iterator const const_col_iter;
        
        SparseMatrix(size_t i){ m=i; n=i; row_buf=new T[n](); }
        SparseMatrix(size_t i, size_t j){ m=i; n=j; row_buf=new T[n](); }
        
        inline T& operator()(size_t i, size_t j)
        {
                if(i>=m || j>=n) throw;
                return mat[i][j];
        }
        
        inline T operator()(size_t i, size_t j) const
        {
                if(i>=m || j>=n) throw;
                return mat[i][j];
        }
        
        inline bool nonzero(size_t i, size_t j)
        {
                row_iter ii=mat.find(i);
                if (ii==mat.end())
                        return false;
                col_iter jj=(*ii).second.find(j);
                if (jj==(*ii).second.end())
                        return false;
                return true;
        }
        
        T *get_row(size_t i)
        { // get a pointer to current row
                for (int k=0;k<n;++k) {
                        row_buf[k]=0;
                }
                row_iter ii=mat.find(i);
                if (ii==mat.end())
                        return row_buf;
                for (col_iter jj=(*ii).second.begin();jj!=(*ii).second.end();++jj)
                        row_buf[(*jj).first]=(*jj).second;
                return row_buf;
        }
        
        T row_sum(int i)
        {
                T sum=0;
                row_iter ii=mat.find(i);
                col_iter jj;
                if (ii!=mat.end()) {
                        for (jj=(*ii).second.begin(); jj!=(*ii).second.end(); jj++)
                                sum+= (*jj).second;
                }
                return sum;
        }
        
        inline size_t size() { return m; }
        
        vector<T> operator*(const vector<T>& x)
        {  //Computes y=A*x
                if(this->m != x.size()) throw;
                
                vector<T> y(this->m);
                T sum;
                
                row_iter ii;
                col_iter jj;
                
                for(ii=this->mat.begin(); ii!=this->mat.end(); ii++){
                        sum=0;
                        for(jj=(*ii).second.begin(); jj!=(*ii).second.end(); jj++){
                                sum += (*jj).second * x[(*jj).first];
                        }
                        y[(*ii).first]=sum;
                }
                
                return y;
        }
        
        void save(ostream &ostr)
        {
                row_iter ii;
                col_iter jj;
                for(ii=this->mat.begin(); ii!=this->mat.end(); ii++) {
                        for( jj=(*ii).second.begin(); jj!=(*ii).second.end(); jj++){
                                ostr << (*ii).first << " ";
                                ostr << (*jj).first << " ";
                                ostr << (*jj).second << endl;
                        }
                } 
        }
        
        void restore(istream &istr)
        {
                int i,j;
                T prob;
                while (!(istr >> i >> j >> prob).eof()) {
                        mat[i][j]=prob;
                }
        }
        
protected:
        SparseMatrix(){}
        
private:
        T *row_buf;
        mat_t mat;
        size_t m;
        size_t n;
};

using namespace Rcpp;
using namespace std;

typedef SparseMatrix<float> SparseM;

// global variables for predictor state
static SparseM **matrices=0;
static float *targetprob=0;
static float *coefs=0;
static float muc=0, muic=0;

vector<string> vocab;

int ngramlength;

static const int BLANK=-1;
static const float SMOOTHING=-12; // 0.00005

void print_coefs(float *x, int length)
{
  cout << "c: ";
  for (int i=0;i<length;++i)
    cout << x[i] << " ";
  cout << endl;
}

void print_coefs()
{
  print_coefs(coefs,ngramlength+1);
}

void initsr(int *sr)
{
  for (int i=0;i<ngramlength;++i)
    sr[i]=BLANK;
}

void printsr(int *sr)
{
  for (int i=0;i<ngramlength;++i)
    cerr << sr[i] << " ";
  cerr << endl;
}

void shiftleft(int *sr)
{
  // sr[0] sr[1] sr[2] sr[3]
  for (int i=0;i<ngramlength-1;++i)
    sr[i]=sr[i+1];
  sr[ngramlength-1]=BLANK;
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

string clean_string_space(string &s)
{
  string d;
  bool last_space=false;
  for (string::iterator si=s.begin();si!=s.end();++si) {
    char c=*si;
    c=tolower(c);
    if (c>='a' && c<='z')
      d+=c;

    if (c==' ' && !last_space) {
      d+=c;
      last_space=true;
    }
    else if (c!=' ')
      last_space=false;
  }
  return d;
}

float dot(float *x, float *y, int length)
{
  float result=0.0;
  for (int i=0;i<length;++i)
    result+=x[i]*y[i];
  return result;
}

void update_coefs(float *c, float mu, float *x, float error, int length)
{
  for (int i=0;i<length;++i)
    c[i]=c[i]+2*mu*x[i]*error;
}

void get_x(SparseM **m, int *sr, int target, int targetprob, float *x)
{
  // this routine consumes half the time
  for (int i=0;i<ngramlength;++i)
    if (sr[i]==BLANK)
      x[i]=0;
    else if (m[i]->nonzero(sr[i],target))
      x[i]=m[i]->operator()(sr[i],target);
    else
      x[i]=SMOOTHING;
  x[ngramlength]=targetprob;
}

void predict(const string &words, string &prediction1,
	   string &prediction2,
	   string &prediction3,
	   string &correct)
{
  // assumes: words: string lower case, no punctuation, one space between words
  //          not blank

  prediction1="";
  prediction2="";
  prediction3="";
  const int UNKNOWN_0=vocab.size();
  const int UNKNOWN_1=UNKNOWN_0+1; // Rcpp complains unused
  const int BLANK=-1;

  int tokens[ngramlength];
  initsr(tokens);

  int rnum=ngramlength-1;
  
  size_t right_most=words.size()-1; // get the rightmost position

  do {
    size_t last_word=words.find_last_of(' ',right_most);
    if (last_word==string::npos) {
      // must be very left most word
      tokens[rnum--]=get_token_id(vocab,words.substr(0,right_most+1));
      right_most=0;
    }
    else {
      tokens[rnum--]=get_token_id(vocab,words.substr(last_word+1,right_most-last_word)); // word is just to the right of the space
      right_most=last_word-1; // one left of the space
    }
  } while (right_most>0 && rnum>=0);

  if (tokens[ngramlength-1]==BLANK) {
    cerr << "error: died with BLANK to predict" << endl;
    exit(1);
    return; 
  }

  // calculate probability for every target
  float targets[UNKNOWN_1];
  for (int i=0;i<UNKNOWN_1;++i)
    targets[i]=0.0;

  for (int i=0;i<ngramlength+1;++i) {
    if (tokens[i]!=BLANK) {
      float *row;
      if (i==ngramlength)
	row=targetprob;
      else
	row=matrices[i]->get_row(tokens[i]);
      
      for (int t=0;t<UNKNOWN_1;++t) {
	float val=row[t];
	if (val==0.0)
	  val=SMOOTHING;
	targets[t]+=val*coefs[i]; // weight the target prob. by the filter coefficient
      }
    }
  }

  // find the maximum probability

  float pprob=-1e10;
  int predict_idx1=UNKNOWN_0;
  for (int i=0;i<UNKNOWN_0;++i) {
    if (targets[i]>pprob) {
      pprob=targets[i];
      predict_idx1=i;
    }
  }

  pprob=-1e10;
  int predict_idx2=UNKNOWN_0;
  for (int i=0;i<UNKNOWN_0;++i) {
    if (i!=predict_idx1 && targets[i]>pprob) {
      pprob=targets[i];
      predict_idx2=i;
    }
  }

  pprob=-1e10;
  int predict_idx3=UNKNOWN_0;
  for (int i=0;i<UNKNOWN_0;++i) {
    if (i!=predict_idx1 && i!=predict_idx2 && targets[i]>pprob) {
      pprob=targets[i];
      predict_idx3=i;
    }
  }

  // get the predicted words as strings
  prediction1=get_string_from_id(vocab, predict_idx1);
  prediction2=get_string_from_id(vocab, predict_idx2);
  prediction3=get_string_from_id(vocab, predict_idx3);

  // update filter coefficients using lms

  int correct_idx=get_token_id(vocab,correct);

  float x[ngramlength+1];
  float y,error,yhat;

  if (predict_idx1!=correct_idx) {

    // incorrect answer error correction

    get_x(matrices, tokens, predict_idx1, targetprob[predict_idx1], x);
    yhat=dot(x,coefs,ngramlength+1);
    y=SMOOTHING*(ngramlength+1); // a low probability
    error=y-yhat;
    update_coefs(coefs, muic, x, error, ngramlength+1);
  }

    // correct answer reinforcement

  get_x(matrices, tokens, correct_idx, targetprob[correct_idx], x);
  yhat=dot(x,coefs,ngramlength+1);
  y=0; // highest probability
  error=y-yhat;
  update_coefs(coefs, muc, x, error, ngramlength+1);
  
}

void initp(string matrixpath, int ngraml)
{
  // read vocabulary
  ifstream ifs((matrixpath+"/vocab.dat").c_str());
  if (ifs.fail()) {
    cerr << "loading vocabulary failed" << endl;
    return;
  }
  read_vocabulary(ifs, vocab);
  ifs.close();
  
  // read in sparse probability matrices
  const int UNKNOWN_0=vocab.size();
  const int UNKNOWN_1=UNKNOWN_0+1;

  ngramlength=ngraml;
  cerr << "Using " << ngramlength << " as ngram length." << endl;

  // allocate predictor state

  muc = 1e-6;
  muic = 1e-6;

  coefs=new float[ngramlength+1];
  for (int i=0;i<ngramlength+1;++i)
    coefs[i]=1.0;

  matrices=new SparseM *[ngramlength];

  for (int i=0;i<ngramlength;++i) {
    matrices[i]=new SparseM(UNKNOWN_1,UNKNOWN_1);
  }

  targetprob = new float[UNKNOWN_1];

  // load prediction matrices, target vector

  cerr << "opening prediction matrices" << endl;

  ifstream *infiles=new ifstream[ngramlength];

  for (int i=0;i<ngramlength;++i) {
    stringstream fn;
    fn << matrixpath << "/out" << i << ".dat";
    infiles[i].open((fn.str()).c_str());
    if (infiles[i].fail()) {
      cerr << "Could not open ngram file " << &fn << "for input." << endl;
      exit(1);
    }
  }

  for (int i=0;i<ngramlength;++i) {
    cerr << "actually loading matrix: " << i << endl;
    matrices[i]->restore(infiles[i]);
  }

  cerr << "loading target word array" << endl;

  ifstream targetfile((matrixpath+"/outtarget.dat").c_str());
  if (targetfile.fail()) {
    cerr << "Cannot open target array data" << endl;
    exit(1);
  }

  float prob;

  int i=0;
  while (!(targetfile >> prob).eof()) {
    targetprob[i++]=prob;
  }

  cerr << "completed all initialization" << endl;

}

// [[Rcpp::export]]
void initR()
{
  initp(".",4);
  float static_coefs[] = {0.551687, 0.631857, 0.790975, 0.963588, 1.38946};
  for (int i=0;i<5;++i)
    coefs[i]=static_coefs[i];
}

// [[Rcpp::export]]
string predictR(string words)
{
  if (!words.compare(""))
          return "";
  string predict1,predict2,predict3,correct;
  words=clean_string_space(words);
  predict(words, predict1, predict2, predict3, correct);
  return predict1;
}

