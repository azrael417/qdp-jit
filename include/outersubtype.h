// -*- C++ -*-
// $Id: outersubtype.h,v 1.5 2003-01-24 21:06:19 edwards Exp $

/*! \file
 * \brief Outer grid classes after a subset
 */


QDP_BEGIN_NAMESPACE(QDP);

//! OScalar class narrowed to a subset
/*! 
 * Only used for lvalues
 */
template<class T>
class OSubScalar: public QDPSubType<T, OScalar<T> >
{
  typedef OScalar<T> C;

public:
  OSubScalar(const OScalar<T>& a, const Subset& ss): F(a), s(ss) {}
  OSubScalar(const OSubScalar& a): F(a.F), s(a.s) {}
  ~OSubScalar() {}

  //---------------------------------------------------------
  // Operators
  // NOTE: all assignment-like operators except operator= are
  // inherited from QDPSubType

  inline
  void operator=(const typename WordType<T>::Type_t& rhs)
    {
      assign(rhs);
    }

  inline
  void operator=(const Zero& rhs)
    {
      assign(rhs);
    }

  template<class T1,class C1>
  inline
  void operator=(const QDPType<T1,C1>& rhs)
    {
      assign(rhs);
    }

  template<class T1,class C1>
  inline
  void operator=(const QDPExpr<T1,C1>& rhs)
    {
      assign(rhs);
    }


  inline
  void operator=(const OSubScalar& rhs)
    {
      assign(rhs);
    }


private:
  // Hide default constructor
  OSubScalar() {}

public:
  C& field() {return const_cast<C&>(F);}
  const Subset& subset() const {return s;}

private:
  const C&  F;
  const Subset& s;
};



//-------------------------------------------------------------------------------------
//! OLattice class narrowed to a subset
/*! 
 * Only used for lvalues
 */
template<class T> 
class OSubLattice: public QDPSubType<T, OLattice<T> >
{
  typedef OLattice<T> C;

public:
  OSubLattice(const OLattice<T>& a, const Subset& ss): F(a), s(ss) {}
  OSubLattice(const OSubLattice& a): F(a.F), s(a.s) {}
  ~OSubLattice() {}

  //---------------------------------------------------------
  // Operators
  // NOTE: all assignment-like operators except operator= are
  // inherited from QDPType

  inline
  void operator=(const typename WordType<T>::Type_t& rhs)
    {
      assign(rhs);
    }

  inline
  void operator=(const Zero& rhs)
    {
      assign(rhs);
    }

  template<class T1,class C1>
  inline
  void operator=(const QDPType<T1,C1>& rhs)
    {
      assign(rhs);
    }

  template<class T1,class C1>
  inline
  void operator=(const QDPExpr<T1,C1>& rhs)
    {
      assign(rhs);
    }


  inline
  void operator=(const OSubLattice& rhs)
    {
      assign(rhs);
    }


private:
  // Hide default constructor
  OSubLattice() {}

public:
  C& field() {return const_cast<C&>(F);}
  const Subset& subset() const {return s;}

private:
  const C&  F;
  const Subset& s;
};


//-----------------------------------------------------------------------------
// Traits class for returning the subset-ted class name of a outer grid class
//-----------------------------------------------------------------------------

template<class T>
struct QDPSubTypeTrait<OScalar<T> > 
{
  typedef OSubScalar<T>  Type_t;
};

template<class T>
struct QDPSubTypeTrait<OLattice<T> > 
{
  typedef OSubLattice<T>  Type_t;
};


//-----------------------------------------------------------------------------
// Traits classes to support operations of simple scalars (floating constants, 
// etc.) on QDPTypes
//-----------------------------------------------------------------------------

template<class T>
struct WordType<OSubScalar<T> > 
{
  typedef typename WordType<T>::Type_t  Type_t;
};

template<class T>
struct WordType<OSubLattice<T> > 
{
  typedef typename WordType<T>::Type_t  Type_t;
};


//-----------------------------------------------------------------------------
// Scalar Operations
//-----------------------------------------------------------------------------

//! dest = 0
template<class T> 
void zero_rep(OScalar<T>& dest, const Subset& s) 
{
  zero_rep(dest.field().elem());
}

//! dest = 0
template<class T> 
void zero_rep(OSubScalar<T> dest) 
{
  zero_rep(dest.field().elem());
}

//! dest = (mask) ? s1 : dest
template<class T1, class T2> 
void copymask(OSubScalar<T2> dest, const OSubScalar<T1>& mask, const OSubScalar<T2>& s1) 
{
  copymask(dest.field().elem(), mask.elem(), s1.elem());
}


//-----------------------------------------------------------------------------
// Random numbers
//! dest  = random  
/*! Implementation is in the specific files */
template<class T>
void random(OSubScalar<T> d);


//! dest  = gaussian
template<class T>
void gaussian(OSubScalar<T> dd)
{
  OLattice<T>& d = dd.field();
  const Subset& s = dd.subset();

  OScalar<T>  r1, r2;

  random(r1(s));
  random(r2(s));

  fill_gaussian(d.elem(), r1.elem(), r2.elem());
}


QDP_END_NAMESPACE();
