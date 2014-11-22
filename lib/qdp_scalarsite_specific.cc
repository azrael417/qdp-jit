
/*! @file
 * @brief Scalar-like architecture specific routines
 * 
 * Routines common to all scalar-like architectures including scalar and parscalar
 */


#include "qdp.h"
#include "qdp_util.h"

namespace QDP {

//-----------------------------------------------------------------------------
namespace Layout
{
  //! coord[mu]  <- mu  : fill with lattice coord in mu direction
  /* Assumes no inner grid */
  LatticeInteger latticeCoordinate(int mu)
  {
    static multi1d<LatticeInteger> latCoord(Nd);
    static std::vector<bool> availCoord(Nd,false);

    if (mu < 0 || mu >= Nd)
      QDP_error_exit("dimension out of bounds");

    if (!availCoord[mu]) {
      QDPIO::cout << "creating latticeCoordinate " << mu << "..." << flush;
      const int nodeSites = Layout::sitesOnNode();
      const int nodeNumber = Layout::nodeNumber();
      LatticeInteger d;
      for(int i=0; i < nodeSites; ++i) 
	{
	  Integer cc = Layout::siteCoords(nodeNumber,i)[mu];
	  d.elem(i) = cc.elem();
	}
      latCoord[mu] = d;
      availCoord[mu] = true;
	  
	  QDPIO::cout << "done!" << endl;
    }

    return latCoord[mu];
  }
}


//-----------------------------------------------------------------------------
// IO routine solely for debugging. Only defined here
template<class T>
ostream& operator<<(ostream& s, const multi1d<T>& s1)
{
  for(int i=0; i < s1.size(); ++i)
    s << " " << s1[i];

  return s;
}


//-----------------------------------------------------------------------------
//! Constructor from a function object
void Set::make(const SetFunc& fun)
{
  int nsubset_indices = fun.numSubsets();
  const int nodeSites = Layout::sitesOnNode();
  const int nodeNumber = Layout::nodeNumber();

#if QDP_DEBUG >= 2
  QDP_info("Set a subset: nsubset = %d",nsubset_indices);
#endif

  // This actually allocates the subsets
  sub.resize(nsubset_indices);

  // Create the space of the colorings of the lattice
  lat_color.resize(nodeSites);

  // Create the array holding the array of sitetable info
  sitetables.resize(nsubset_indices);

  // Create the array holding the array of membertable info
  membertables.resize(nsubset_indices);

  // Loop over linear sites determining their color
  for(int linear=0; linear < nodeSites; ++linear)
  {
    multi1d<int> coord = Layout::siteCoords(nodeNumber, linear);

    int node   = Layout::nodeNumber(coord);
    int lin    = Layout::linearSiteIndex(coord);
    int icolor = fun(coord);

#if QDP_DEBUG >= 4
    QDPIO::cerr<<"linear="<<linear<<" coord="<<coord<<" node="<<node<<" col="<<icolor << endl;
#endif

    // Sanity checks
    if (node != nodeNumber)
      QDP_error_exit("Set: found site with node outside current node!");

    if (lin != linear)
      QDP_error_exit("Set: inconsistent linear sites");

    if (icolor < 0 || icolor >= nsubset_indices)
      QDP_error_exit("Set: coloring is outside legal range: color[%d]=%d",linear,icolor);

    // The coloring of this linear site
    lat_color[linear] = icolor;
  }


  // largest_subset = 0;
  // enableGPU = true;


  /*
   * Loop over the lexicographic sites.
   * This implementation of the Set will always use a
   * sitetable.
   */
  for(int cb=0; cb < nsubset_indices; ++cb)
  {
    // Always construct the sitetables. 

    multi1d<bool>& membertable = membertables[cb];
    membertable.resize(nodeSites);

    // First loop and see how many sites are needed
    int num_sitetable = 0;
    for(int linear=0; linear < nodeSites; ++linear)
      if (lat_color[linear] == cb) {
	++num_sitetable;
	membertable[linear] = true;
      } else {
	membertable[linear] = false;
      }

    // Now take the inverse of the lattice coloring to produce
    // the site list
    multi1d<int>& sitetable = sitetables[cb];
    sitetable.resize(num_sitetable);


    // Site ordering stuff for later
    bool ordRep;
    int start, end;

    // Handle the case that there are no sites
    if (num_sitetable > 0)
    {
      // For later sanity, initialize this to something 
      for(int i=0; i < num_sitetable; ++i)
	sitetable[i] = -1;

      for(int linear=0, j=0; linear < nodeSites; ++linear)
	if (lat_color[linear] == cb)
	  sitetable[j++] = linear;


      // Check *if* this coloring is contiguous and find the start
      // and ending sites
      ordRep = true;
      start = sitetable[0];   // this is the beginning
      end = sitetable[sitetable.size()-1];  // the absolute last site
      
      // Now look for a hole
      for(int prev=sitetable[0], i=0; i < sitetable.size(); ++i)
	if (sitetable[i] != prev++)
	{
#if QDP_DEBUG >= 2
	  QDP_info("Set(%d): sitetable[%d]=%d",cb,i,sitetable[i]);
#endif
	
	  // Found a hold. The rep is not ordered.
	  ordRep = false;
	  start = end = -1;
	  break;
	}
    }
    else  // num_sitetable == 0
    {
      ordRep = false;
      start = end = -1;
    }

    sub[cb].make(ordRep, start, end, &(sitetables[cb]), cb, this, &(membertables[cb]) );


    // if (largest_subset < sitetables[cb].size()) {
    //   if (largest_subset > 0) {
    // 	QDP_error("Warning: Set found with changing subset sizes. This will cause problems when using with CUDA! Disable device calculation for this set.");
    // 	enableGPU = false;
    //   }
    //   largest_subset = sitetables[cb].size();
    // }


#if QDP_DEBUG >= 2
    QDP_info("Subset(%d)",cb);
#endif
  }

  MasterSet::Instance().registrate( *this );


#if 0
  QDPIO::cout << "Set: Building strided sitetables...\n";
  int ss_size = sitetables[0].size();
  int strided=0;
  sitetables_strided.resize( largest_subset * sitetables.size() );
  bool first=true;
  bool hill0=false;
  bool valley0=false;
  bool hill1=false;
  nonEmptySubsetsOnNode = 0;
  for (int n = 0 ; n < sitetables.size() ; n++) {
    if ((ss_size > 0) && 
	(sitetables[n].size() > 0) && 
	(sitetables[n].size() != ss_size)) {
      QDPIO::cout << "Warning: This set has subsets which sizes are different accross nodes. Disabling it for GPU\n";
      enableGPU = false;
    }
    if (sitetables[n].size() > 0) {
      if (first) {
	first = false;
	stride_offset=n;
	hill0=true;
	//QDP_debug("hill0");
      }
      nonEmptySubsetsOnNode++;
      if (valley0) {
	QDPIO::cout << "Warning: This set has at least two separate junctions. Disabling it for GPU\n";
	enableGPU = false;
      }
      ss_size=sitetables[0].size();
    } else {
      if (hill0) { valley0=true;  QDP_debug_deep("hill0 valley0"); }
    }

    for (int i=0 ; i < sitetables[n].size() ; i++ ) {
      sitetables_strided[strided++]=sitetables[n][i];
    }
  }
  unsigned dsize = sitetables_strided.size() * sizeof(int);
  QDPIO::cout << "Set: Registering this set ...\n";
  if (registered) {
    QDPIO::cout << "Set: Already registered, will sign it off first ...\n";
    QDPCache::Instance().signoff( idStrided );
  }
  idStrided = QDPCache::Instance().registrateOwnHostMem( dsize , (void*)sitetables_strided.slice() , NULL );
  registered=true;
  QDP_debug("nonEmptySubsetsOnNode  = %d" , nonEmptySubsetsOnNode );  
  QDP_debug("stride_offset = %d" , stride_offset );
#endif  

#if 0
  // Now check across nodes
  Integer yo = enableGPU ? 1 : 0;
  QDPInternal::globalSum(yo);
  if ( toInt( yo ) < Layout::numNodes() ) {
    QDPIO::cout << "Set: This set is disabled at least on one node. Disabling it for all.\n";
    enableGPU=false;
  }
#endif

}
	  

} // namespace QDP;
