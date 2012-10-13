// -*- C++ -*-


#ifndef QDP_WORDJIT_H
#define QDP_WORDJIT_H



namespace QDP {


  template<class T>
  class WordJIT
  {
  public:
    //! Size (in number of registers) of the underlying object
    enum {Size_t = 1};

    //! View of an object from global state space
    WordJIT(Jit& j , int r_addr_ , int offset_full_ , int offset_level_ ) : 
      jit(j), 
      r_addr(r_addr_),
      offset_full(offset_full_),
      offset_level(offset_level_) , global_state(true) {
      //std::cout << "WordJIT(Jit& func_ , int r_addr_  ) global view\n";
      std::cout << "WordJIT() global view   " << (void*)this << " " << (void*)&j << "\n";
    }

    //! New space 
    WordJIT(Jit& j ) : jit(j), global_state(false) {
      int tmp;
      mapReg.insert( std::make_pair( JitRegType<T>::Val_t , tmp = jit.getRegs( JitRegType<T>::Val_t , 1 ) ) );
      std::cout << "WordJIT(Jit& func_ ) new space   regName = " << jit.getName(tmp) << " " << (void*)this << " " << (void*)&jit <<  "\n";
    }


    template <class T1>
    WordJIT& assign(const WordJIT<T1>& s1) {
      if (global_state)
	jit.asm_st( r_addr , offset_level * WordSize<T>::Size , s1.getReg( JitRegType<T>::Val_t ) );
      else
	jit.asm_mov( getReg( JitRegType<T>::Val_t ) , s1.getReg( JitRegType<T>::Val_t ) );
      return *this;
    }

    //---------------------------------------------------------
    template <class T1>
    WordJIT& operator=(const WordJIT<T1>& s1) {
      std::cout << __PRETTY_FUNCTION__ << ": instructions needed?\n" << (void*)this << " " << (void*)&s1 << "\n";
      // Value can be in register file (AddAssign for example)
      // jit.asm_st( r_addr , offset_level * WordSize<T>::Size , s1.getReg( JitRegType<T>::Val_t ) );
      // return *this;
      return assign(s1);
    }


    WordJIT& operator=(const WordJIT& s1) {
      std::cout << __PRETTY_FUNCTION__ << ": (assignment op) instructions needed?\n" << (void*)this << " " << (void*)&s1 << "\n";
      // jit.asm_st( r_addr , offset_level * WordSize<T>::Size , s1.getReg( JitRegType<T>::Val_t ) );
      // return *this;
      return assign(s1);
    }

    template<class T1>
    WordJIT& operator+=(const WordJIT<T1>& rhs) {
      addRep( *this , *this , rhs );
      assign(*this);
    }

    template<class T1>
    WordJIT& operator-=(const WordJIT<T1>& rhs) {
      subRep( *this , *this , rhs );
      assign(*this);
    }


    //! Do shallow copies here
    // Not sure?? Maybe deep
    //WordJIT(const WordJIT& a): jit(a.jit), addr(a.addr), off(a.off) {}

    int getReg( Jit::RegType type ) const {
      std::cout << "getReg type=" << type 
		<< "  mapReg.count(type)=" << mapReg.count(type) 
		<< "  mapReg.size()=" << mapReg.size() << "\n";
      if (mapReg.count(type) > 0) {
	// We already have the value in a register of the type requested
	return mapReg.at(type);
      } else {
	if (mapReg.size() > 0) {
	  // SANITY
	  if (mapReg.size() > 1) {
	    std::cout << "getReg: We already have the value in 2 different types. Now a 3rd one ??\n";
	    exit(1);
	  }
	  // We have the value in a register, but not with the requested type 
	  // std::cout << "We have the value in a register, but not with the requested type\n";
	  MapRegType::iterator loaded = mapReg.begin();
	  Jit::RegType loadedType = loaded->first;
	  int loadedId = loaded->second;
	  mapReg.insert( std::make_pair( type , jit.getRegs( type , 1 ) ) );
	  jit.asm_cvt( mapReg.at(type) , loadedId );
	  return mapReg.at(type);
	} else {
	  // We don't have the value in a register. Need to load it.
	  // std::cout << "We don't have the value in a register. Need to load it " << (void*)this << " " << (void*)&jit << "\n";
	  Jit::RegType myType = JitRegType<T>::Val_t;
	  mapReg.insert( std::make_pair( myType , jit.getRegs( JitRegType<T>::Val_t , 1 ) ) );
	  jit.asm_ld( mapReg.at( myType ) , r_addr , offset_level * WordSize<T>::Size );
	  return getReg(type);
	}
      }
    }

#if 0
    WordJIT(const WordJIT& a): 
      jit(a.jit), 
      mapReg(a.mapReg), 
      r_addr(a.r_addr),  
      offset_full(a.offset_full), 
      offset_level(a.offset_level) {
      std::cout << "WordJIT copy c-tor\n";
    }
#endif

    Jit& func() const {return jit;}


  private:
    typedef std::map< Jit::RegType , int > MapRegType;
    bool global_state;
    Jit&  jit;
    mutable MapRegType mapReg;
    int r_addr;
    int offset_full;
    int offset_level;
  };


template<class T> 
struct WordType<WordJIT<T> >
{
  typedef T  Type_t;
};


  // Default binary(WordJIT,WordJIT) -> WordJIT
  template<class T1, class T2, class Op>
  struct BinaryReturn<WordJIT<T1>, WordJIT<T2>, Op> {
    typedef WordJIT<typename BinaryReturn<T1, T2, Op>::Type_t>  Type_t;
  };

  // Add
  //template<>
  template<class T0,class T1, class T2>
  inline void
  addRep(const WordJIT<T0>& dest, const WordJIT<T1>& l, const WordJIT<T2>& r)
  {
    dest.func().asm_add( dest.getReg( JitRegType<T0>::Val_t ) , 
			    l.getReg( JitRegType<T0>::Val_t ) , 
			    r.getReg( JitRegType<T0>::Val_t ) );
  }

  // Sub
  //template<>
  template<class T0,class T1, class T2>
  inline void
  subRep(const WordJIT<T0>& dest, const WordJIT<T1>& l, const WordJIT<T2>& r)
  {
    dest.func().asm_sub( dest.getReg( JitRegType<T0>::Val_t ) , 
			    l.getReg( JitRegType<T0>::Val_t ) , 
			    r.getReg( JitRegType<T0>::Val_t ) );
  }


  // Multiply
  // I can't use BinaryReturn since this might be mixed precision words
  template<class T0,class T1, class T2>
  inline void
  mulRep(const WordJIT<T0>& dest,const WordJIT<T1>& l, const WordJIT<T2>& r)
  {
    dest.func().asm_mul( dest.getReg( JitRegType<T0>::Val_t ) , 
			    l.getReg( JitRegType<T0>::Val_t ) , 
			    r.getReg( JitRegType<T0>::Val_t ) );
  }


  // FMA
  template<class T0,class T1,class T2,class T3>
  inline void
  fmaRep( const WordJIT<T0>& dest, const WordJIT<T1>& l, const WordJIT<T2>& r, const WordJIT<T3>& add )
  {
    dest.func().asm_fma( dest.getReg( JitRegType<T0>::Val_t ) , 
			    l.getReg( JitRegType<T0>::Val_t ) , 
			    r.getReg( JitRegType<T0>::Val_t ) , 
			    add.getReg( JitRegType<T0>::Val_t ) );
  }

  // neg
  template<class T0,class T1>
  inline void
  negRep( const WordJIT<T0>& dest, const WordJIT<T1>& src )
  {
    dest.func().asm_neg( dest.getReg( JitRegType<T0>::Val_t ) , 
			    src.getReg( JitRegType<T0>::Val_t ) );
  }

  // identity
  template<class T0,class T1>
  inline void
  idRep( const WordJIT<T0>& dest, const WordJIT<T1>& src )
  {
    dest.func().asm_mov( dest.getReg( JitRegType<T0>::Val_t ) , 
			    src.getReg( JitRegType<T0>::Val_t ) );
  }




} // namespace QDP

#endif