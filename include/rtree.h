#ifndef Rtree_H_
#define Rtree_H_

// #include "global.h"
#include "parser.h"  //inquire macroType、blockRegions
// Boost libraries
#include <boost/program_options.hpp>
#include <boost/icl/split_interval_map.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>



namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using uint32_t = boost::uint32_t;
using string = std::string;
using boostPoint = bg::model::point<double, 2, bg::cs::cartesian>;
using boostBox = bg::model::box<boostPoint>;
using RTree = bgi::rtree<std::pair<boostBox, int>, bgi::rstar<32>>;
using RTrees = std::vector<bgi::rtree<std::pair<boostBox, int>, bgi::rstar<32>>>;

class QueryTree {
public:
    // QueryTree(std::vector<unsigned>& cellIdsToExec) : assignCellIds_(cellIdsToExec) {};
    QueryTree(std::vector<unsigned>& cellIdsToExec, circuit& ckt) : assignCellIds_(cellIdsToExec), ckt_(ckt) {};
public:
    int max_num_init_rects;
    void initSet(const std::vector<Rect<int>>& rects);
    void updateSet(const Rect<int>& theRect, int jobIdx);
    int conflictedId(const Rect<int>& theRect, int jobIdx);
    bool isConflicted(const Rect<int>& theRect, int conflictedId);
    int nearestId(const Rect<int>& theRect);
    
    std::vector<std::vector<unsigned>>& schedule();

private:
    circuit& ckt_;
    // for conflict detect
    RTree rtree;
    std::vector<unsigned>& assignCellIds_;
    std::vector<std::vector<unsigned>> batches;

};

#endif