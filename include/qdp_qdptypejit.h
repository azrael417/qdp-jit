// -*- C++ -*-

#ifndef QDP_QDPTYPEJIT_H
#define QDP_QDPTYPEJIT_H

namespace QDP {

  



  template<class T, class C> 
  class QDPTypeJIT
  {
  public:
    //! Type of the first argument
    typedef T Subtype_t;

    //! Type of the container class
    typedef C Container_t;


    QDPTypeJIT( jit_value base_ ,
		jit_value index_ ): base_m(base_), index_m(index_) {
      //std::cout << "QDPTypeJIT 3er ctor\n";
    }

    // QDPTypeJIT( jit_function_t func_ , 
    // 		jit_value base_ ): func_m(func_), base_m(base_) {
    //   //std::cout << "QDPTypeJIT 2er ctor\n";
    //   assert(func_m);
    //   assert(base_m);
    // }

    QDPTypeJIT(const QDPTypeJIT& a) : base_m(a.base_m), index_m(a.index_m) { }

    ~QDPTypeJIT(){}


    jit_value getThreadedBase( JitDeviceLayout lay ) const {
      jit_value wordsize = jit_value( sizeof(typename WordType<T>::Type_t) );
      jit_value ret0 = jit_ins_mul( index_m , wordsize );
      if ( lay != JitDeviceLayout::Coalesced ) {
	jit_value tsize = jit_value( T::Size_t );
	jit_value ret1 = jit_ins_mul( ret0 , tsize );
	jit_value ret2 = jit_ins_add( ret1 , base_m );
	return ret2;
      }
      jit_value ret1 = jit_ins_add( ret0 , base_m );
      return ret1;
    }


    jit_value getInnerSites( JitDeviceLayout lay) const {
      if ( lay == JitDeviceLayout::Coalesced )
	return jit_value(Layout::sitesOnNode());
      else 
	return jit_value(1);
    }


    T& elem( JitDeviceLayout lay ) {
      F.setup(getThreadedBase(lay),getInnerSites(lay),jit_value(0));
      return F;
    }

    const T& elem( JitDeviceLayout lay ) const {
      F.setup(getThreadedBase(lay),getInnerSites(lay),jit_value(0));
      return F;
    }

    T& elem() {
      F.setup(getThreadedBase( JitDeviceLayout::Scalar ),jit_value(1),jit_value(0));
      return F;
    }

    const T& elem() const {
      F.setup(getThreadedBase( JitDeviceLayout::Scalar ),jit_value(1),jit_value(0));
      return F;
    }



  private:
    jit_value    base_m;
    jit_value    index_m;
    mutable T F;


  };
  

  //-----------------------------------------------------------------------------
  // Traits classes to support return types
  //-----------------------------------------------------------------------------

#if 0
  // Default unary(QDPTypeJIT) -> QDPTypeJIT
    template<class T1, class C1, class Op>
    struct UnaryReturn<QDPTypeJIT<T1,C1>, Op> {
      typedef QDPTypeJIT<typename UnaryReturn<T1, Op>::Type_t,
			 typename UnaryReturn<C1, Op>::Type_t>  Type_t;
};

// Default binary(QDPTypeJIT,QDPTypeJIT) -> QDPTypeJIT
template<class T1, class C1, class T2, class C2, class Op>
struct BinaryReturn<QDPTypeJIT<T1,C1>, QDPTypeJIT<T2,C2>, Op> {
  typedef QDPTypeJIT<typename BinaryReturn<T1, T2, Op>::Type_t,
		     typename BinaryReturn<C1, C2, Op>::Type_t>  Type_t;
};

  // Currently, the only trinary operator is ``where'', so return 
  // based on T2 and T3
  // Default trinary(QDPTypeJIT,QDPTypeJIT,QDPTypeJIT) -> QDPTypeJIT
  template<class T1, class C1, class T2, class C2, class T3, class C3, class Op>
  struct TrinaryReturn<QDPTypeJIT<T1,C1>, QDPTypeJIT<T2,C2>, QDPTypeJIT<T3,C3>, Op> {
    typedef QDPTypeJIT<typename BinaryReturn<T2, T3, Op>::Type_t,
		       typename BinaryReturn<C2, C3, Op>::Type_t>  Type_t;
};


//-----------------------------------------------------------------------------
// We need to specialize CreateLeaf<T> for our class, so that operators
// know what to stick in the leaves of the expression tree.
//-----------------------------------------------------------------------------

template<class T, class C>
struct CreateLeaf<QDPTypeJIT<T,C> >
{
  typedef QDPTypeJIT<T,C> Inp_t;
  typedef Reference<Inp_t> Leaf_t;
  //  typedef Inp_t Leaf_t;
  PETE_DEVICE
  inline static
  Leaf_t make(const Inp_t &a) { return Leaf_t(a); }
};


//-----------------------------------------------------------------------------
// Specialization of LeafFunctor class for applying the EvalLeaf1
// tag to a QDPTypeJIT. The apply method simply returns the array
// evaluated at the point.
//-----------------------------------------------------------------------------

template<class T, class C>
struct LeafFunctor<QDPTypeJIT<T,C>, ElemLeaf>
{
  typedef Reference<T> Type_t;
  //  typedef T Type_t;
  PETE_DEVICE
  inline static Type_t apply(const QDPTypeJIT<T,C> &a, const ElemLeaf &f)
  { 
    return Type_t(a.elem());
  }
};

template<class T, class C>
struct LeafFunctor<QDPTypeJIT<T,C>, EvalLeaf1>
{
  typedef Reference<T> Type_t;
  //  typedef T Type_t;
  PETE_DEVICE
  inline static Type_t apply(const QDPTypeJIT<T,C> &a, const EvalLeaf1 &f)
  { 
    return Type_t(a.elem(f.val1()));
  }
};


template<class T>
struct LeafFunctor<QDPTypeJIT<T,OLattice<T> >, ParamLeaf>
{
  typedef QDPTypeJIT<T,typename JITType<OLattice<T> >::Type_t>  TypeA_t;
  //typedef typename JITType< OLattice<T> >::Type_t  TypeA_t;
  typedef TypeA_t  Type_t;
  inline static Type_t apply(const QDPTypeJIT<T,OLattice<T> > &a, const ParamLeaf& p)
  {
    return Type_t( jit_add_param( jit_ptx_type::u64 ) );
  }
};

template<class T>
struct LeafFunctor<QDPTypeJIT<T,OScalar<T> >, ParamLeaf>
{
  typedef QDPTypeJIT<T,typename JITType<OScalar<T> >::Type_t>  TypeA_t;
  //typedef typename JITType< OScalar<T> >::Type_t  TypeA_t;
  typedef TypeA_t  Type_t;
  inline static Type_t apply(const QDPTypeJIT<T,OScalar<T> > &a, const ParamLeaf& p)
  {
    return Type_t( jit_add_param( jit_ptx_type::u64 ) );
  }
};


// template<class T, class C>
// struct LeafFunctor<QDPTypeJIT<T,C>, ViewLeaf>
// {
//   typedef T Type_t;
//   inline static
//   Type_t apply(const QDPTypeJIT<T,C>& s, const ViewLeaf& v)
//   { 
//     return s.elem( v.getLayout() );
//   }
// };



#endif




} // namespace QDP

#endif

