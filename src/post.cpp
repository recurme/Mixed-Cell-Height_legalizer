#include "nall.h"


void Naller::init_edgeType() {
    for (unsigned cellId = 0; cellId < cell_num; ++cellId) {
        Cell* sp = ckt.cells[ cellId ];
        if(sp->isFixed_) {
            continue;
        }    
        int xLL = sp->cur_x_;
        int xUR = sp->cur_x_ + sp->width_ - 1;
        for (int y = sp->cur_y_; y < (sp->cur_y_ + sp->height_); y += defaultH)
        {
            node.node_infos[g_max_x*y/defaultH + xLL].lEdgeT_ = sp->lEdgeT_;
            node.node_infos[g_max_x*y/defaultH + xUR].rEdgeT_ = sp->rEdgeT_;
        }
    }
}

void Naller::reduceNodeCostWithEdgeT(const int& s_x, const int& s_y, 
                          const int& width, const int& height) {  //rip-up
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            node_info.lEdgeT_ = -1;
            node_info.rEdgeT_ = -1;
            --node_info.usage_;
            node_info.updated_ = false;
        }
    }
}

void Naller::addNodeCostWithEdgeT(const int& s_x, const int& s_y, const int& width, const int& height, 
                        const int& lEdgeT, const int& rEdgeT) {
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            ++node_info.usage_;
            node_info.updated_ = false;
            node_info.lEdgeT_ = lEdgeT;
            node_info.rEdgeT_ = rEdgeT;      
            // node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor;
            // node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;
        }
    }
}

void Naller::postRouteWithTech(const int& cellId, const int& max_space) { 
    Cell* sp = ckt.cells[cellId];
    double min_cost = 1.e20;
    double min_s_am = 100000.0;
    int direc = 0;
    int row_step = (sp->aligendRow_ == 2) ? 1 : 2;
    for (int i = 0; i < total_direc_num; ++i)
    {
        int s_x = sp->cur_x_ +  dir_array_x[i];
        int s_y = sp->cur_y_ + row_step * dir_array_y[i] * defaultH;
        if(outBox(s_x, s_y, sp->width_, sp->height_)) {
            continue;
        }
        if(isStripCongested(s_x, s_y, sp->width_, sp->height_, 0, sp->regionId_)) {
            continue;
        }
        double cost_temp = 0.0;  
        double s_am_temp = fabs(s_x -  sp->init_x_) + fabs(s_y -  sp->init_y_);
        // double abs_disp = fabs(s_x -  sp->init_x_) + fabs(s_y -  sp->init_y_);
        // double s_am_temp = (num_unfixed_inst_ + 0.0) / kMacros[sp->height_].first * (abs_disp);
        cost_temp += s_am_temp; 
        //pin short/access
        for(auto& pin : sp->signal_pins_) {  //  You can't use the width and height of a cell to determine if it's short-circuited.
            for (int y = (s_y + pin.yLL_offset_psite); y < (s_y + pin.yUR_offset_psite); y+=defaultH)
            {
                for (int x = (s_x + pin.xLL_offset_psite); x < (s_x + pin.xUR_offset_psite); ++x)
                {
                    NodeInfo& node_info = node.node_infos[g_max_x*y/defaultH + x];
                    if((node_info.layer_ - pin.layer) == 0 or (node_info.layer_ - pin.layer) == 1){
                        cost_temp += penalty_tech * defaultH;
                    }
                }
            }      
        }
        //edgeSpace 
        for (int y = s_y; y < (s_y + sp->height_); y += defaultH)
        {
            //left direction
            for(int x = s_x - 1; x >=  (s_x - max_space); --x) {
                if(x > 0) {
                    NodeInfo& node_info = node.node_infos[g_max_x*y/defaultH + x];
                    if(node_info.rEdgeT_ <= 0) {
                        continue;
                    }
                    else {
                        cost_temp += std::max(ckt.edge_table[ node_info.rEdgeT_ + sp->lEdgeT_] - (s_x - x - 1), 0) * penalty_tech * defaultH;
                        break;
                    }
                }
                else {
                    break;
                }
            }
            //right direction
            for(int x = s_x + sp->width_; x <  (s_x + sp->width_ + max_space); ++x) {
                if(x < g_max_x) {
                    NodeInfo& node_info = node.node_infos[g_max_x*y/defaultH + x];
                    if(node_info.lEdgeT_ <= 0) {
                        continue;
                    }
                    else {
                        cost_temp += std::max(ckt.edge_table[ node_info.lEdgeT_ + sp->rEdgeT_] - (x - s_x - sp->width_), 0) * penalty_tech * defaultH;
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }    
        if(cost_temp < min_cost) {
            direc = i;
            min_cost = cost_temp;
            min_s_am = s_am_temp;      
        }
        else if(fabs(min_cost - cost_temp) < EPSILON) {
            if (s_am_temp < min_s_am ) {
                direc = i;
                min_cost = cost_temp;
                min_s_am = s_am_temp;          
            }
        }
    }

    sp->cur_x_ += dir_array_x[direc];
    sp->cur_y_ +=  row_step * dir_array_y[direc] * defaultH;

    addNodeCostWithEdgeT(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, sp->lEdgeT_, sp->rEdgeT_);
}

void Naller::stripShiftingWithTech() {
    auto cmp_distance = [this](int a, int b) {
        return this->cmp_distance(a, b);
    };
    sort(ckt.cellIds.begin(), ckt.cellIds.end(), cmp_distance);
    int iter_num = 5; 
    int of_cnt;
    int max_space = *std::max_element(ckt.edge_table.begin(), ckt.edge_table.end());

    for (int iter = 0; iter < iter_num; ++iter)
    {
        for (auto i : ckt.cellIds)
        {
            Cell* sp = ckt.cells[ i ];
            if(sp->isFixed_) {     //block  
                continue;
            }
            reduceNodeCostWithEdgeT(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
            postRouteWithTech(i, max_space);
        }
    }
    of_cnt = countOverflows(false);
    if((of_cnt == 0) ) {
        calDisplacment();
        log()<<"  5_iteration : "<<" of_cnt : "<<of_cnt
            <<"; s_am : "<<s_am/defaultH<<"; m_max : "<<max_disp/defaultH
            <<"; avg_disp : "<<avg_disp<<"; max_disp : "<<max_disp<<std::endl;
    }
}


void Naller::stripSwapping_modified(std::vector<unsigned int>& cellIds) {
    int bin_x_num = 3 * defaultH; //    40 ,  40
    int bin_y_num = bin_x_num * g_max_y / g_max_x; //
    int bx_size;  //
    int by_size;
    for (auto& subVector : bin) {
        subVector.clear();
    }
    bin.clear();
    bin.resize( bin_x_num*bin_y_num );
    bx_size = ceil( (g_max_x + 0.0 ) / bin_x_num );
    by_size = ceil( (g_max_y + 0.0) / bin_y_num );

    for (auto i : cellIds)
    {
        Cell* sp = ckt.cells[ i ];
        if(sp->isFixed_) {     //block  
            continue;
        }    
        int s_x = sp->cur_x_;
        int s_y = sp->cur_y_;
        bin[ int(s_x  / bx_size) + bin_x_num * int(s_y / by_size) ].push_back(i);
    }
    std::vector<int> stripType0(stripType.size());
    int size0 = 0;
    for (std::set<int>::iterator  it = stripType.begin(); it != stripType.end(); ++it) {
        stripType0[size0++] = *it;
    }
    double infinity = std::numeric_limits<double>::infinity();
    omp_set_num_threads(8); //8
    #pragma omp parallel for  schedule(dynamic)
    for (int i = 0; i < bin_x_num*bin_y_num; ++i)
    {
        if(bin[i].empty()) {
            continue;
        }
        Munkres<double> m;   //Munkres algorithm
        Matrix<double> matrix;
        std::vector<int> v;
        for (int it = 0; it < size0; ++it)
        {//Iterate over several macro kinds
            int nrows = 0;
            for (size_t j = 0; j < bin[i].size(); ++j) {
                Cell* sp = ckt.cells[ bin[i][j] ];
                if((sp->aligendRow_*g_max_y/defaultH*g_max_x + sp->height_/defaultH*g_max_x + sp->width_) == stripType0[it]) {  //type_:index to macro 
                    ++nrows;
                    v.push_back(bin[i][j]);
                }
            }
            if(nrows <= 1) {
                v.clear();
                continue;
            }
            matrix.resize(nrows, nrows);
            for (int row = 0; row < nrows; ++row)
            {
                Cell* sp0 = ckt.cells[ v[row] ];//Initial position of row
                double s_x = sp0->init_x_;
                double s_y = sp0->init_y_;    
                for (int col = 0; col < nrows; ++col)    //Current position of column col
                {
                    Cell* sp = ckt.cells[ v[col] ];
                    double cur_disp = fabs(s_x -  sp->cur_x_ ) + fabs(s_y -  sp->cur_y_ );
                    if((cur_disp - max_disp) > EPSILON) {
                        matrix(row,col) = infinity;
                    }
                    else
                        matrix(row,col) =  cur_disp;
                }
            }
            int state = m.solve(matrix);
            if(state != 0) { 
                //0:sucessed
                v.clear();
                continue;
            }
            for (int row = 0; row < nrows; ++row)
            {
                if(matrix(row,row) > -0.5) {
                    continue;
                }
                for (int col = 0; col < nrows; ++col)
                {
                    if (matrix(row,col) > -0.5) { // match
                        Cell* sp_row = ckt.cells[v[row]], *sp_col = ckt.cells[v[col]];
                        sp_row->cur_x0_ = sp_row->cur_x_;
                        sp_row->cur_y0_ = sp_row->cur_y_;
                        if(col < row) { 
                            sp_row->cur_x_ = sp_col->cur_x0_;
                            sp_row->cur_y_ = sp_col->cur_y0_;
                        }
                        else {
                            sp_row->cur_x_ = sp_col->cur_x_;
                            sp_row->cur_y_ = sp_col->cur_y_;              
                        }
                        break;
                    }
                }
            } 
            v.clear();
        }
    }
    calDisplacment();
    log()<<"  s_am : "<<s_am/defaultH<<"; m_max : "<<max_disp/defaultH
        <<"; avg_disp : "<<avg_disp<<"; max_disp : "<<max_disp
        <<" Kuhn-Munkres algorithm end;"<<std::endl;
}
