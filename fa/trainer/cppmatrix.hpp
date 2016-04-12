
#ifndef __SPARSE_MATRIX__
#define __SPARSE_MATRIX__ 1

// http://www.cplusplus.com/forum/general/8352/

#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>

//#define BOUNDS_CHECK 1

template <class T>
class BigMatrix
{
public:

  BigMatrix(size_t i){ m=i; n=i; mat = new T[m*n](); }
  BigMatrix(size_t i, size_t j){ m=i; n=j; mat = new T[m*n](); }
  ~BigMatrix() { delete [] mat; }

  T& operator()(size_t i, size_t j)
  {
#ifdef BOUNDS_CHECK
    if (i>=m || j>=n) throw;
#endif
    return mat[i*n+j];
  }

  T operator()(size_t i, size_t j) const
  {
#ifdef BOUNDS_CHECK
    if (i>m || j>=n) throw;
#endif
    return mat[i*n+j];
  }

  bool nonzero(size_t i, size_t j)
  {
#ifdef BOUNDS_CHECK
    if (i>=m || j>=n) throw; 
#endif
    return mat[i*n+j]!=0; 
}

  T *get_row(size_t i)
  {
    return &mat[i*n];
  }

  T row_sum(int i)
  {
    T sum=0;
    for (int j=0;j<n;++j)
      sum+=mat[i*n+j];
    return sum;
  }

  size_t size() { return m; }

  void save(std::ostream &ostr)
    {
      for(int i=0;i<m;++i) {
	for( int j=0; j<n; ++j) {
	  ostr << i << " ";
	  ostr << j << " ";
	  ostr << mat[i*n+j] << std::endl;
	}
      } 
    }
    
  void restore(std::istream &istr)
  {
    int i,j;
    T prob;
    while (!(istr >> i >> j >> prob).eof()) {
      mat[i*n+j]=prob;
    }
  }

private:

  size_t m,n;
  T *mat;
};

template <class T>
class SparseMatrix
{
 public:
  typedef std::map<size_t, std::map<size_t , T> > mat_t;
  typedef typename mat_t::iterator row_iter;
  typedef typename mat_t::iterator const const_row_iter;
  typedef std::map<size_t, T> col_t;
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

    std::vector<T> operator*(const std::vector<T>& x)
      {  //Computes y=A*x
        if(this->m != x.size()) throw;

	std::vector<T> y(this->m);
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

  void save(std::ostream &ostr)
    {
      row_iter ii;
      col_iter jj;
      for(ii=this->mat.begin(); ii!=this->mat.end(); ii++) {
	for( jj=(*ii).second.begin(); jj!=(*ii).second.end(); jj++){
	  ostr << (*ii).first << " ";
	  ostr << (*jj).first << " ";
	  ostr << (*jj).second << std::endl;
	}
      } 
    }
    
  void restore(std::istream &istr)
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

#endif

