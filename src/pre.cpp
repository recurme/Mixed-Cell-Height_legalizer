#include "nall.h"


void Naller::infos_resize() {
    node.node_infos.resize(g_max_x * g_max_y / defaultH);
    temp_usageVec.resize(g_max_x);
    max_disp_th = 3 * defaultH;
    penalty_tech = 1 * defaultH;
    step_space = 8 * defaultH;//280 (superbiules)
    // step_space = 28 * defaultH;//280 (superbiules)
}

void Naller::addBlockCost() {
  // x-direction may be not full used for standard cell.
    int blockRegions_size = ckt.blockRegions.size();
    for (unsigned k = 0; k < blockRegions_size; ++k)
    {
        const Rect<int>& br = ckt.blockRegions[k];
        for (int j = br.yLL_; j < br.yUR_; j+=defaultH)
        {
            for (int i = br.xLL_; i < br.xUR_; ++i)
            {
                NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                node_info.usage_ = 100000000;
                node_info.pCost_ = HUGE_FLOAT;
                node_info.hisCost_ = HUGE_FLOAT;
                node_info.pathCost_ = HUGE_FLOAT;
            }
        }
    }
}

void Naller::moveIntoFence(Cell *sp, Rect<int> &rect) {  //moveIntoFenceRegion(Boundary)
    if(sp->cur_y_temp_ + sp->height_ > rect.yUR_) {
        sp->cur_y_ = rect.yUR_ - sp->height_;
        sp->cur_y_temp_ = sp->cur_y_ + 0.0;
        if(sp->cur_x_ < rect.xLL_) {
            sp->cur_x_ = rect.xLL_;
        }
        else if(sp->cur_x_ + sp->width_ > rect.xUR_) {
            sp->cur_x_ = rect.xUR_ - sp->width_;
        }
    }
    else if(sp->cur_y_temp_ < rect.yLL_) {
        sp->cur_y_ = rect.yLL_;
        sp->cur_y_temp_ = sp->cur_y_ + 0.0;
        if(sp->cur_x_ < rect.xLL_) {
            sp->cur_x_ = rect.xLL_;
        }
        else if(sp->cur_x_ + sp->width_ > rect.xUR_) {
            sp->cur_x_ = rect.xUR_ - sp->width_;
        }    

    }
    else {
        if(sp->cur_x_ < rect.xLL_) {
            sp->cur_x_ = rect.xLL_;
        }
        else if(sp->cur_x_ + sp->width_ > rect.xUR_) {
            sp->cur_x_ = rect.xUR_ - sp->width_;
        }       
    }
}

void Naller::initFenceRegion(){
    log() << "  fenceRegions: " << ckt.fenceRegions.size() <<  
        " defaultCellIds : " << ckt.defaultCellIds.size() << std::endl;
    QueryTree queryTree(ckt.cellIds, ckt);    
    for (unsigned k = 0; k < ckt.fenceRegions.size(); ++k)
    {
        FenceRegion* fenceRegion = ckt.fenceRegions[k];
        queryTree.initSet(fenceRegion->rects_);
        string &name = fenceRegion->name_;
        //setFenceRegionId
        for(auto& fr: fenceRegion->rects_) {
            for (int j = fr.yLL_; j < fr.yUR_; j+=defaultH)
            {
                for (int i = fr.xLL_; i < fr.xUR_; ++i)
                {
                    NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                    node_info.regionId_ = fenceRegion->id_;  //dont assign blockRegion's id
                }
            }      
        }
        log() << "    name " << name << " num_cell " << ckt.region2cellIds[name].size() << std::endl;
        for (auto cellId : ckt.region2cellIds[name])
        {
            Cell *sp = ckt.cells[cellId];
            if(sp->isFixed_) {
                continue;
            }
            if(sp->aligendRow_ == 2) {         
                int quo = round(sp->cur_y_temp_  / defaultH);
                sp->cur_y_ = (int)(quo * defaultH);
            } else if(sp->aligendRow_ == 0) { //VSS
                int quo = round(sp->cur_y_temp_  / (defaultH * 2));
                sp->cur_y_ = std::min( (int)(quo * defaultH * 2), g_max_y - sp->height_ - g_max_y /defaultH % 2 * defaultH); //..
            } else {
                int quo = (int)(sp->cur_y_temp_  / (defaultH * 2)); //VDD
                sp->cur_y_ = std::min( defaultH + quo * defaultH * 2, g_max_y - sp->height_ - (g_max_y /defaultH + 1) % 2 * defaultH);
            } 
            sp->cur_y_temp_ = sp->cur_y_ + 0.0;
            Rect<int> rect(sp->cur_x_, sp->cur_y_, (sp->cur_x_ + sp->width_), sp->cur_y_ + sp->height_);
            int rectId =  queryTree.conflictedId(rect, -1);
            if(rectId == -1) {
                rectId = queryTree.nearestId(rect);
                //move into nearest fenceRegion
                moveIntoFence(sp, fenceRegion->rects_[rectId]);
            }
        }
    }
}

void Naller::initSpNetRegion() {
    unsigned spNetRegions_size = ckt.spNetRegions.size();
    log() << "  spNetRegions " <<  spNetRegions_size << std::endl;

    for (unsigned k = 0; k < spNetRegions_size; ++k)
    {
        SpNetRegion* spNet = ckt.spNetRegions[k];  //including IO pins, and PG nets
        for (int j = spNet->rect_.yLL_; j < spNet->rect_.yUR_; j+=defaultH)
        {
            for (int i = spNet->rect_.xLL_; i < spNet->rect_.xUR_; ++i)
            {
                if(g_max_x*j/defaultH + i < node.node_infos.size()){
                    NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                    node_info.layer_ = spNet->layer_; 
                }
            }
        }   
    }
}

int Naller::inBlockId(int s_x, int s_y, int width, int height) {
  for (unsigned i = 0; i < ckt.blockRegions.size(); ++i)
  {
    const Rect<int>& br = ckt.blockRegions[i];
    if(s_x >= br.xLL_ and (s_x+width) <= br.xUR_ and s_y >= br.yLL_ and (s_y+height) <= br.yUR_) {
        return i;  // shape completed in block
    }    
  }
  return -1;
}

void Naller::moveOutBlock(const int cellId, const int blockId) {
    Cell* sp = ckt.cells[cellId];
    Rect<int>& br = ckt.blockRegions[blockId];
    int left = sp->cur_x_ + sp->width_ - br.xLL_;
    int right = br.xUR_ - sp->cur_x_;
    int down = int(sp->init_y_) - br.yLL_ + sp->height_;
    int up = br.yUR_ - int(sp->init_y_);

    //if outRegion
    if(sp->cur_x_ - left < 0) left = g_max_x;
    if(br.yLL_ - sp->height_ < 0) down = g_max_y;

    int x_min = std::min(left, right);
    int y_min = std::min(down, up);
    if(x_min < y_min) {
        if(x_min == left) {
            sp->cur_x_ -=  left;
        }
        else
            sp->cur_x_ = br.xUR_;
    } 
    else {
        if(y_min == down) {
            sp->cur_y_ = br.yLL_ - sp->height_;
            sp->cur_y_temp_ = sp->cur_y_ + 0.0;
        }
        else {
            sp->cur_y_ = br.yUR_;
            sp->cur_y_temp_ = sp->cur_y_ + 0.0;
        }    
    }
}



bool Naller::moveOutFence(Cell *sp, int count_outRegion) {  //for regionId_ == -1 cells
    for (FenceRegion* fenceRegion : ckt.fenceRegions)
    {
        for (const Rect<int>& fr : fenceRegion->rects_)
        {
            if(sp->cur_x_ >= fr.xLL_ and (sp->cur_x_ + sp->width_) <= fr.xUR_ and sp->cur_y_ >= fr.yLL_ and (sp->cur_y_ + sp->height_) <= fr.yUR_) {
                int left = sp->cur_x_ + sp->width_ - fr.xLL_;
                int right = fr.xUR_ - sp->cur_x_;
                int down = sp->cur_y_ - fr.yLL_ + sp->height_;
                int up = fr.yUR_ - sp->cur_y_;
                int x_min = std::min(left, right);
                int y_min = std::min(down, up);
                int rem = (count_outRegion - 1) % 4;
                reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                if(rem == 0) {
                    if(x_min < y_min) {//count_outRegion 1,2,3,4,1,2,3,4,...
                        if(x_min == left) {
                            sp->cur_x_ -=  left;
                        }
                        else{
                            sp->cur_x_ = fr.xUR_;
                        }
                    } 
                    else {
                        if(y_min == down) {
                            sp->cur_y_ = fr.yLL_ - sp->height_;
                            sp->cur_y_temp_ = sp->cur_y_ + 0.0;
                        }
                        else {
                            sp->cur_y_ = fr.yUR_;
                            sp->cur_y_temp_ = sp->cur_y_ + 0.0;
                        }          
                    }
                    addNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                    return true;
                }

                else {
                    std::vector<int> offset;
                    offset.push_back(left);
                    offset.push_back(right);
                    offset.push_back(down);
                    offset.push_back(up);
                    std::sort(offset.begin(), offset.end());
                    int cur_offset = offset[rem];
                    if(cur_offset == left){
                        sp->cur_x_ -=  left;
                    }
                    else if(cur_offset == right) {
                        sp->cur_x_ = fr.xUR_;
                    }
                    else if(cur_offset == down) {
                        sp->cur_y_ = fr.yLL_ - sp->height_;
                        sp->cur_y_temp_ = sp->cur_y_ + 0.0;
                    }
                    else if(cur_offset == up) {
                        sp->cur_y_ = fr.yUR_;
                        sp->cur_y_temp_ = sp->cur_y_ + 0.0;
                    }
                    addNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                    return true;
                }
            }
        }
    }
    return false;
}

void Naller::moveOutSpNetRegion(int cellId, const int& s_x, const int& s_y, 
                          const int& width, const int& height, 
                           const std::vector<Macro_pin>& signal_pins) {  
    for(const auto& pin : signal_pins) {  // You can't use the width and height of a cell to determine if it's short-circuited.
        for (int j = s_y + pin.yLL_offset_psite; j < (s_y + pin.yUR_offset_psite); j+=defaultH)
        {
            for (int i = s_x + pin.xLL_offset_psite; i < (s_x + pin.xUR_offset_psite); ++i)
            {
                NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                if((node_info.layer_ - pin.layer) == 0 or (node_info.layer_ - pin.layer) == 1) {
                    if((node_info.layer_ % 2) == 0)  {
                        reduceNodeCost(s_x, s_y, width, height);
                        route(cellId, false);  //small distance moving
                        return;
                    }            
                }
            }
        }      
    }  
}

void Naller::init_adjust() {
    log() << "******************" << "init_adjust" << "******************" << std::endl;
    addBlockCost(); //for those isFixed_ cells or blockages
    initFenceRegion();  //noted that i dont move those whiteSpace'cells in fenceRegion here, 
               // and do it in short / long distance moving process. 
    initSpNetRegion(); // P/G nets
    int num_errorRegion = 0;
    for(unsigned i : ckt.cellIds) {
        Cell* sp = ckt.cells[ i ];
        if(sp->isFixed_) {     //block  
            continue;
        }
        stripType.insert(sp->aligendRow_*g_max_y/defaultH*g_max_x + sp->height_/defaultH*g_max_x + sp->width_);
        // 1. move out of block
        int blockId = inBlockId(sp->cur_x_, (int)sp->init_y_, sp->width_, sp->height_);
        if(blockId > -1) { //inBlock
            moveOutBlock(i, blockId);
        }
        if(sp->cur_x_ + sp->width_ > g_max_x ) {   
            sp->cur_x_ =  g_max_x  - sp->width_ ;
        }
        if(sp->cur_x_ < 0) {
            sp->cur_x_ = 0;
        }
        if(sp->cur_y_temp_  > g_max_y - sp->height_ ) {   
            sp->cur_y_ =  g_max_y - sp->height_ ;
            sp->cur_y_temp_ = sp->cur_y_ + 0.0;
        }  
        if(sp->cur_y_temp_ < 0) {
            sp->cur_y_ = 0;
            sp->cur_y_temp_ = 0.0;
        } 
        //row alignement...
        if(sp->aligendRow_ == 2) {         
            int quo = round(sp->cur_y_temp_  / defaultH);
        sp->cur_y_ = (int)(quo * defaultH);
        } else if(sp->aligendRow_ == 0) { //VSS
            int quo = round(sp->cur_y_temp_  / (defaultH * 2));
            sp->cur_y_ = std::min( (int)(quo * defaultH * 2), g_max_y - sp->height_ - g_max_y /defaultH % 2 * defaultH); //..
        } else {
            int quo = (int)(sp->cur_y_temp_  / (defaultH * 2)); //VDD
            sp->cur_y_ = std::min( defaultH + quo * defaultH * 2, g_max_y - sp->height_ - (g_max_y /defaultH + 1) % 2 * defaultH);
        }
        // Use for Counting
        sp->aligendRow_y_ = sp->cur_y_;

        double cost_temp = 0.0;  
        calTempCost(cost_temp, sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        if(cost_temp > 1000000.0) {  //partitially in block, dont deal with tech cons problem.
            route(i, false);  
        }
        else {
            addNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);   //noted
        }
        int count_outRegion = 0;
        while(1) {
            cost_temp = 0.0;
            calTempCost(cost_temp, sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
            if(cost_temp > 1000000.0) {
                num_errorRegion++; 
                reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                if((count_outRegion - 1) <= 8) {
                    route(i, true);  //for those hard to moveOutBlock cells      
                }   
            }      
            else {
                break;
            }
            cost_temp = 0.0;
            calTempCost(cost_temp, sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
            if(cost_temp > 1000000.0) {
                num_errorRegion++; 
                reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                route(i, false);  //small distance moving
            }      
            else {
                break;
            }      
            if(count_outRegion >= 1 and sp->regionId_ == -1) {
                moveOutFence(sp, count_outRegion);  //for defaultCells
            } 
            count_outRegion++;
        }
        moveOutSpNetRegion(i, sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->signal_pins_);
        penalty_tech = 3.0;
        sp->cur_x0_ = sp->cur_x_;  //int = int 
        sp->cur_y0_ = sp->cur_y_;  //int = int 

        if(sp->cur_x_ > 100000000 or sp->cur_x_ < 0) {
            std::cout << sp->name_ << " 0.3 " << sp->init_x_ << " " << sp->init_y_
                << sp->cur_x_ << " " << sp->cur_y_ << std::endl;
        }
    }
    log() <<  "  num_errorRegion : " << num_errorRegion << std::endl;
    checkFenceRegion();
}
