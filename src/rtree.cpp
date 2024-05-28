#include "rtree.h"

void QueryTree::initSet(const std::vector<Rect<int>>& rects) {
    rtree.clear();

    for (unsigned i = 0; i < rects.size(); ++i)  //
    {
        updateSet(rects[i], i);//since block/fence regions's id has conflit with cell's id.
    }

}

void QueryTree::updateSet(const Rect<int>& theRect, int jobIdx) {
    double relaxMargin = -0.1;

    boostBox box(boostPoint(theRect.xLL_ + relaxMargin, theRect.yLL_ + relaxMargin),
                     boostPoint(theRect.xUR_ - relaxMargin, theRect.yUR_ - relaxMargin));   
   
    rtree.insert({box, jobIdx});
}

int QueryTree::conflictedId(const Rect<int>& theRect, int jobIdx) {
    double relaxMargin = -0.1;
    // if(theRect.xLL_ == 873 and theRect.yLL_ == 3590) {
    //     log() << theRect.xLL_ + relaxMargin << " " << theRect.yLL_ + relaxMargin << " " <<
    //     theRect.xUR_ - relaxMargin << " " << theRect.yUR_ - relaxMargin << std::endl;        
    // }


    boostBox box(boostPoint(theRect.xLL_ + relaxMargin, theRect.yLL_ + relaxMargin),
                     boostPoint(theRect.xUR_ - relaxMargin, theRect.yUR_ - relaxMargin));    
    std::vector<std::pair<boostBox, int>> results; 
    rtree.query(bgi::intersects(box), std::back_inserter(results));
    for (const auto &result : results) {
        if (result.second != jobIdx) {
            // log() << bg::wkt<boostBox>(result.first) << "-" << result.second << std::endl;
            return result.second;
        }
    }
    return -1;  //not conflicted
}
bool QueryTree::isConflicted(const Rect<int>& theRect, int conflictedId) {
    double relaxMargin = -0.1;
    // if(theRect.xLL_ == 873 and theRect.yLL_ == 3590) {
    //     log() << theRect.xLL_ + relaxMargin << " " << theRect.yLL_ + relaxMargin << " " <<
    //     theRect.xUR_ - relaxMargin << " " << theRect.yUR_ - relaxMargin << std::endl;        
    // }


    boostBox box(boostPoint(theRect.xLL_ + relaxMargin, theRect.yLL_ + relaxMargin),
                     boostPoint(theRect.xUR_ - relaxMargin, theRect.yUR_ - relaxMargin));    
    std::vector<std::pair<boostBox, int>> results; 
    rtree.query(bgi::intersects(box), std::back_inserter(results));
    for (const auto &result : results) {
        if (result.second != conflictedId) {
            // log() << bg::wkt<boostBox>(result.first) << "-" << result.second << std::endl;
            return true;
        }
    }
    return false;  //not conflicted
}
int QueryTree::nearestId(const Rect<int>& theRect) {
    double relaxMargin = 0.0;
    // if(theRect.xLL_ == 873 and theRect.yLL_ == 3590) {
    //     log() << theRect.xLL_ + relaxMargin << " " << theRect.yLL_ + relaxMargin << " " <<
    //     theRect.xUR_ - relaxMargin << " " << theRect.yUR_ - relaxMargin << std::endl;        
    // }

    boostBox box(boostPoint(theRect.xLL_ + relaxMargin, theRect.yLL_ + relaxMargin),
                     boostPoint(theRect.xUR_ - relaxMargin, theRect.yUR_ - relaxMargin));    
    std::vector<std::pair<boostBox, int>> results; 
    rtree.query(bgi::nearest(box, 1), std::back_inserter(results));
    assert(!results.empty());

            // log() << bg::wkt<boostBox>(result.first) << "-" << result.second << std::endl;
    return results[0].second;



}

std::vector<std::vector<unsigned>> &QueryTree::schedule() {
    // init assigned table
    int cell_num = ckt_.cell_num;
    std::vector<bool> assigned(cell_num, false);;   //all false; cell_num called in main.cc


    // sort by height and width



    size_t lastUnassign = 0;
    while (lastUnassign < cell_num) {
        // create a new batch from a seed
        batches.emplace_back();
        initSet({});
        std::vector<unsigned> &batch = batches.back();
        for (size_t i = lastUnassign; i < cell_num; ++i) {
            int cellId = assignCellIds_[i];
            Cell *cell = ckt_.cells[cellId];
            if (!assigned[cellId] && !isConflicted(cell->extendedWin_, cellId)) {
                batch.push_back(cellId);
                assigned[cellId] = true;
                updateSet(cell->extendedWin_, cellId);
            }
        }
        // find the next seed
        while (lastUnassign < cell_num && assigned[assignCellIds_[lastUnassign]]) {
            ++lastUnassign;
        }
    }

    // sort within batches by NumOfVertices
    // in each batch, big net first routing to be more balance.
    // for (auto &batch : batches) {
    //     std::sort(batch.begin(), batch.end(), [&](int lhs, int rhs) {
    //         return nets[lhs].bbox_ > nets[rhs].bbox_;
    //     });
    // }

    //small cell firstly locate
    std::reverse(batches.begin(), batches.end());
    return batches;
}