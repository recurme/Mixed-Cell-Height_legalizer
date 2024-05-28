#include "nall.h"

const int Naller::dir_array_x[49] = 
{0,-1,1,-2,2,-3,3,-4,4,-5,5,-6,6,-7,7,-8,8,-9,9,-10,10,-11,11,-12,12,
  0,-1,1,-2,2,-3,3,-4,4,-5,5,
  0,-1,1,-2,2,-3,3,-4,4,-5,5,
0,0};
const int Naller::dir_array_x2[57] = 
{0,-3,3,-6,6,-9,9,
  -30,30,-60,60,-90,90,-120,120,-150,150,-180,180,-210,
  210,-240,240,-270,270,-300,300,-360,360,-420,420,
  0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0};
const int Naller::dir_array_x3[77] = 
{0,-1,1,-2,2,-3,3,-4,4,-5,5,-6,6,-7,7,-8,8,-9,9,-18,18,
  0,-1,1,-2,2,-3,3,-9,9,-15,15,-21,21,-33,33,-45,45,-57,57,
  0,-1,1,-2,2,-3,3,
  0,0,
  0,-1,1,-2,2,-3,3,-9,9,-15,15,-21,21,-33,33,-45,45,-57,57,
  0,-1,1,-2,2,-3,3,
  0,0};  
  //for superblue series of ispd15 benchmarks. Some magic number due to the r.d. inflation.
const int Naller::dir_array_x4[67] = 
{0,-1600,1600,-1700,1700,-1717,1717,-1800,1800,-1900,1900,-2000,2000,-2100,
  2100,-2200,2200,-2300,2300,-2400,2400,-2500,2500,-2600,2600,-2700,2700,
  -750,750,-760,760,-900,900,-2100,2100,-2200,2200,   
  -750,750,-760,760,-900,900,-2100,2100,-2200,2200,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0};


const int Naller::dir_array_y[49] =
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
0,0};
const int Naller::dir_array_y2[57] = 
{0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,
  3,6,9,12,13,15,16,18,21,24,27,29,30,
  -3,-6,-9,-12,-13,-15,-16,-18,-21,-24,-27,-29,-30};  //(16,pci_bridge32_b_md1); (13,pci_bridge32_a_md2); 
const int Naller::dir_array_y3[77] =
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,
  3,4,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -2,-2,-2,-2,-2,-2,-2,
  -3,-4};
const int Naller::dir_array_y4[67] = 
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  1,1,1,1,1,1,1,1,1,1,
  80,86,92,98,104,110,116,120,126,132,
  -80,-86,-92,-98,-104,-110,-116,-120,-126,-132};
const int Naller::total_direc_num = 49;
const int Naller::total_direc_num2 = 57;
const int Naller::total_direc_num3 = 77;
const int Naller::total_direc_num4 = 67;

// bool debug4mt;
bool Naller::cmp(unsigned id1, unsigned id2) {
    Cell* a = ckt.cells[id1];
    Cell* b = ckt.cells[id2];
    if(a->init_y_ < b->init_y_) {
        return true;
    }
    else {
        if(a->init_y_ > b->init_y_) {
            return false;
        }
        else {
            return a->init_x_ < b->init_x_;
        }
    }
}

bool Naller::cmp_row(unsigned id1, unsigned id2) {
  Cell* a = ckt.cells[id1];
  Cell* b = ckt.cells[id2];

  if(a->cur_y_ < b->cur_y_) {
    return true;
  }
  else {
    if(a->cur_y_ > b->cur_y_) {
      return false;
    }
    else {
      return a->cur_x_ < b->cur_x_;
    }
  }
}
bool Naller::cmp_congestion(int id1, int id2) {
    Cell* a = ckt.cells[id1];
    Cell* b = ckt.cells[id2];
    if(a->of_ > b->of_) {   //tested  fpga:> < >
        return true;
    }
    else {
        if(a->of_ < b->of_) {
            return false;
        }
        else {
        if((a->height_ / defaultH % 2) > (b->height_ / defaultH % 2)) {  //odd-height cell first
            return true;
        }
        else if((a->height_ / defaultH % 2) < (b->height_ / defaultH % 2)) {
            return false;
        }
        else {
            if((a->height_ * a->width_) < (b->height_ * b->width_)) {  // like net_HPWL
                return true;
            }
            else if((a->height_ * a->width_) > (b->height_ * b->width_)) {
                return false;
            }
            else 
                return a->height_ < b->height_;
        }
        // else {
        //   if(a->height_  < b->height_ ) {  // like net_HPWL
        //     return true;
        //   }
        //   else if(a->height_ > b->height_ ) {
        //     return false;
        //   }
        //   else 
        //     return a->width_ < b->width_;
        // }      
        }
    }
}



bool Naller::cmpT_congestion(unsigned a, unsigned b) {
  Cell* sp1 = ckt.cells[ a ];
  Cell* sp2 = ckt.cells[ b ];
  if(sp1->of_ > sp2->of_) {   //tested
    return true;
  }
  else {
    if(sp1->of_ < sp2->of_) {
      return false;
    }
    else {
      return sp1->height_ < sp2->height_;
    }
  }
}
bool Naller::cmp_distance(unsigned id1, unsigned id2) {
  Cell* a = ckt.cells[id1];
  Cell* b = ckt.cells[id2];
  // double s_am_a =  g_alpha * (pow(a.cur_x_ -  a.init_x_, 2) + pow(a.cur_y_ -  a.init_y_, 2));
  // double s_am_b =  g_alpha * (pow(b.cur_x_ -  b.init_x_, 2) + pow(b.cur_y_ -  b.init_y_, 2));
  double s_am_a = fabs(a->cur_x_ -  a->init_x_) + fabs(a->cur_y_ -  a->init_y_);
  double s_am_b = fabs(b->cur_x_ -  b->init_x_) + fabs(b->cur_y_ -  b->init_y_);
  // double s_am_a = (num_unfixed_inst_ + 0.0) / kMacros[a->height_].first * (abs_disp_a);
  // double s_am_b = (num_unfixed_inst_ + 0.0) / kMacros[b->height_].first * (abs_disp_b);
  if(s_am_a > s_am_b) {
    return true;
  }
  else {
    if(s_am_a < s_am_b) {
      return false;
    }
    else {
      return cmp(id1, id2);
    }
  }
}


void Naller::calDisplacment() {
    unsigned cell_num = ckt.cellIds.size();
    s_am = 0.0;
    max_disp = 0.0;
    avg_disp = 0.0;
    int k_nums = 0;
    for (unsigned i = 0; i < cell_num; ++i)  
    {
        Cell* sp = ckt.cells[ i ];
        if(sp->isFixed_) {
            continue;
        }
        double  cur_disp = fabs(sp->cur_x_ -  sp->init_x_) + fabs(sp->cur_y_ -  sp->init_y_);
        if(debug_max_print) {
            if(cur_disp > 480.0 or cur_disp < -100.0) {  //100.0 -> 2000.0
                std::cout<< "name: " << sp->name_ << "; ( " << sp->init_x_*min_width << ","<< sp->init_y_*min_width  << "); (" 
            << sp->cur_x_*min_width << ", " << sp->cur_y_*min_width << " )" << std::endl;        
            }
        }
        if(sp->cur_x_ > 100000000 or sp->cur_x_ < 0) {
            std::cout << i << " " << sp->name_ << " 0.4 " << sp->init_x_ << " " << sp->init_y_ << " "
            << sp->cur_x_ << " " << sp->cur_y_ << std::endl;
        }
        avg_disp += cur_disp;
        max_disp = std::max( cur_disp, max_disp );
        ckt.kMacros[sp->height_].second +=  cur_disp;
    }
    for (auto it = ckt.kMacros.begin(); it != ckt.kMacros.end(); ++it)
    {
        // printlog(LOG_INFO, "hegiht : %d num : %d", it->first, it->second.first); 
        s_am += (it->second.second) / it->second.first;
        // printlog(LOG_INFO, "disp : %f num : %d", it->second.second, it->second.first); 
        it->second.second = 0.0;  //clear
        ++k_nums;
    }
    // printlog(LOG_INFO, "s_am : %f, k_nums : %d", s_am, k_nums); 
    s_am = s_am / k_nums;
    // printf("    total disp. : %f (sites)\n", avg_disp);
    avg_disp = avg_disp / cell_num;
}

bool Naller::checkAlignment() {
    for (unsigned i = 0; i < cell_num; ++i)
    {
        Cell* sp = ckt.cells[ i ];
        if(sp->isFixed_) {
            continue;
        }
        if(sp->aligendRow_ == 0) { //VSS
            assert(sp->isBottomVss_);
            if(sp->cur_y_ / defaultH % 2 == 0) 
                continue;
            else {
                std::cout<< "name: " << sp->name_ <<" "<< sp->aligendRow_ << " ; ( " << sp->init_x_*min_width << ","<< sp->init_y_*min_width  << "); (" 
                << sp->cur_x_*min_width << ", " << sp->cur_y_*min_width << " )" << std::endl;
                return false;
            }

        } else if(sp->aligendRow_ == 1){  //VDD
            assert(!sp->isBottomVss_);
            if(sp->cur_y_ / defaultH % 2 == 1)  {
                continue;
            }
            else {
                std::cout<< "name: " << sp->name_ <<" "<< sp->aligendRow_ << " ; ( " << sp->init_x_*min_width << ","<< sp->init_y_*min_width  << "); (" 
                << sp->cur_x_*min_width << ", " << sp->cur_y_*min_width << " )" << std::endl;
                return false;
            }
        }
        else {
            if(sp->cur_y_ / defaultH % 2 == 0) {
                if(!sp->isBottomVss_) {
                    sp->cellorient_ = "FS";  //assert original_cellorient == "N";
                }
            }
            else if(sp->isBottomVss_) {
                sp->cellorient_ = "FS";  //assert original_cellorient == "N";
            }
        }
    }
    return true;
}

bool Naller::checkFenceRegion() {
    for (auto cellId : ckt.cellIds)  
    {
        Cell* sp = ckt.cells[ cellId ];
        if(sp->isFixed_) {
            continue;
        }
        bool err = false;
        for (int j = sp->cur_y_; j < (sp->cur_y_ + sp->height_); j += defaultH)
        {
            for (int i = sp->cur_x_; i < (sp->cur_x_ + sp->width_); ++i)
            {
                NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];

                if(node_info.regionId_ != sp->regionId_ && !err) {
                    printlog(LOG_ERROR, " error_region : name %s, init_x_ : %f, init_y_ : %f, cur_x_ : %f, cur_y_ : %f, right_id : %d, error id : %d",  sp->name_.c_str(),
                    sp->init_x_ * min_width, sp->init_y_ * min_width, 
                    sp->cur_x_ * min_width, sp->cur_y_ * min_width, node_info.regionId_, sp->regionId_);
                    // return false;
                    err = true;
                }
                    
            }
        }
    }
    log() << "  fenceRegion matched" << std::endl;
    return true;
}

void Naller::checkTech() {
    int nPinShort11 = 0, nPinShort22 = 0;
    int nPinAccess12 = 0, nPinAccess23 = 0;
    Ne = 0;
    for (unsigned cellId = 0; cellId < cell_num; ++cellId)
    {
        Cell* sp = ckt.cells[ cellId ];
        if(sp->isFixed_) {
            continue;
        }
        bool once_count11 = false;
        bool once_count12 = false;
        bool once_count22 = false;
        bool once_count23 = false;
        for(const auto& pin : sp->signal_pins_) {  // You can't use the width and height of a cell to determine if it's short-circuited.
            for (int j = (sp->cur_y_ + pin.yLL_offset_psite); j < (sp->cur_y_ + pin.yUR_offset_psite); j+=defaultH)
            {
                for (int i = (sp->cur_x_ + pin.xLL_offset_psite); i < (sp->cur_x_ + pin.xUR_offset_psite); ++i)
                {
                NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                if((node_info.layer_ - pin.layer) == 0) {
                    if(pin.layer == 0 and !once_count11) {
                        nPinShort11++;
                        once_count11 = true;
                    }
                    else if(pin.layer == 1 and !once_count22) {
                        nPinShort22++;
                        once_count22 = true;
                    }
                } 
                else if((node_info.layer_ - pin.layer) == 1 ){
                    if(pin.layer == 0 and !once_count12) {
                        nPinAccess12++;
                        once_count12 = true;
                    }
                    else if(pin.layer == 1 and !once_count23) {
                        nPinAccess23++;
                        once_count23 = true;
                    }            
                }

                }
            }      
        }  
        int max_space = *std::max_element(ckt.edge_table.begin(), ckt.edge_table.end());
        bool once_count = false;
        for (int y = sp->cur_y_; y < (sp->cur_y_ + sp->height_); y += defaultH)
        {
            //left direction
            if(once_count) break;
            for(int x = sp->cur_x_ - 1; x >=  (sp->cur_x_ - max_space); --x) {
                if(x > 0) {
                    NodeInfo& node_info = node.node_infos[g_max_x*y/defaultH + x];
                    if(node_info.rEdgeT_ <= 0) {
                        continue;
                    }
                    else {
                        if( (ckt.edge_table[ node_info.rEdgeT_ + sp->lEdgeT_] - (sp->cur_x_ - x - 1) ) > 0) {
                            Ne++;
                            once_count = true;
                            break;
                        }
                    }
                }
                else {
                    break;
                }
            }
            //right direction
            if(once_count) break;
            for(int x = sp->cur_x_ + sp->width_; x <  (sp->cur_x_ + sp->width_ + max_space); ++x) {
                if(x < g_max_x) {
                    NodeInfo& node_info = node.node_infos[g_max_x*y/defaultH + x];
                    if(node_info.lEdgeT_ <= 0) {
                        continue;
                    }
                    else {
                        if( (ckt.edge_table[ node_info.lEdgeT_ + sp->rEdgeT_] - (x - sp->cur_x_ - sp->width_)) > 0) {
                            Ne++;
                            once_count = true;
                            break;              
                        }
                    }
                }
                else {
                    break;
                }
            }
        }  
    } 
    Np = nPinShort11 + nPinShort22 + nPinAccess12 + nPinAccess23;
    printlog(LOG_INFO, "  nPinShort11 is %d, nPinShort22 is %d, nPinAccess12 is %d, nPinAccess23 is %d; Np is %d, Ne is %d", 
        nPinShort11, nPinShort22, nPinAccess12, nPinAccess23, Np, Ne); 
    ckt.Np = Np, ckt.Ne = Ne, ckt.max_disp = max_disp, ckt.s_am = s_am;
}



void Naller::calCellOf(const int& s_x, const int& s_y,const int& width, 
                  const int& height, int& of, int& of_cnt, bool update_his) { 
    of = 0;
    for (int j = s_y; j < (s_y + height) ; j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            of += node_info.usage_ - 1;
            if((node_info.usage_) <= 0) {
                printlog(LOG_INFO, "  cur_x_ : %f, cur_y_ : %f, width : %f, height : %f, usage %d", 
                    s_x * min_width, s_y * min_width, 
                    width * min_width, height * min_width, node_info.usage_);
                printlog(LOG_INFO, "i : %f, j is %f", i * min_width, j * min_width);
            }
            if(!update_his) continue;
            if(!node_info.updated_ and node_info.usage_ > 1) {
                node_info.updated_ = true;
                node_info.hisCost_ += 1 * (node_info.usage_ - 1);//
                node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;           
            }
        }
    }
    of_cnt += of; 
}
int Naller::countOverflows(const std::vector<unsigned>& batch) {

  int of_cnt = 0;
  for (auto i : batch)  
  {
    Cell* sp = ckt.cells[ i ];
    if(sp->isFixed_) {
      continue;
    }
    calCellOf(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->of_, of_cnt); 
  }
  return of_cnt;
}
int Naller::countOverflows(int index, int threshold) {
    int of_cnt = 0;
    batches_of_netIds[index].clear();
    for (auto cellId : batches[index])  
    {
        Cell* sp = ckt.cells[ cellId ];
        assert(!sp->isFixed_);
        calCellOf(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->of_, of_cnt); 
        if(threshold > 0) {
            if(sp->of_ > 0) {
                batches_of_netIds[index].push_back(cellId);
            }
        }
    }
    return of_cnt;
}
int Naller::countOverflows(bool update_his) {
    int of_cnt = 0;
    for (unsigned i = 0; i < cell_num; ++i) 
    {
        Cell* sp = ckt.cells[ i ];
        if(sp->isFixed_) {
        continue;
        }
        calCellOf(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->of_, of_cnt, update_his); 
    }
    return of_cnt;
}


bool Naller::isStripCongested(const int& s_x, const int& s_y,
                        const int& width, const int& height, const int& lBound, int regionId) {
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            if(node_info.usage_ > lBound) {
                return true;
            }
            if(node_info.regionId_ != regionId) {
                return true;
            }      
        }
    }
    return false;
}

