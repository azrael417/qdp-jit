// -*- C++ -*-

#ifndef QDP_MASTERMAP_H
#define QDP_MASTERMAP_H

namespace QDP {

  class MasterMap {
  public:
    static MasterMap& Instance();
    int registrate(const Map& map);

    const multi1d<int>& getInnerSites(const Subset& s,int bitmask) const;
    const multi1d<int>& getFaceSites(const Subset& s,int bitmask) const;

    int getCountInner(const Subset& s,int bitmask) const;
    int getCountFace(const Subset& s,int bitmask) const;

  private:
    void complement(multi1d<int>& out, const multi1d<int>& orig) const;
    void remove_neg(multi1d<int>& out, const multi1d<int>& orig) const;
    void remove_neg_in_subset(multi1d<int>& out, const multi1d<int>& orig, int s_no) const;
    void uniquify_list_inplace(multi1d<int>& out , const multi1d<int>& ll) const;

    MasterMap() {
      //QDP_info("MasterMap() reserving");

      QDPIO::cerr << "constructing master map with " << MasterSet::Instance().numSubsets() << " subsets: start\n";

      powerSet.resize( MasterSet::Instance().numSubsets() );
      powerSetC.resize( MasterSet::Instance().numSubsets() );

      // powerSet.reserve(2048);
      // powerSetC.reserve(2048);

      for (int s_no = 0 ; s_no < MasterSet::Instance().numSubsets() ; ++s_no ) {
	powerSet[s_no].resize(1);
	powerSet[s_no][0] = new multi1d<int>;
      }
	  
	  QDPIO::cerr << "constructing master map with " << MasterSet::Instance().numSubsets() << " subsets: done\n";
    }

    std::vector<const Map*> vecPMap;
    std::vector< std::vector< multi1d<int>* > > powerSet;  // Power set of roffsets
    std::vector< std::vector< multi1d<int>* > > powerSetC; // Power set of complements
  };

} // namespace QDP

#endif
