#include "nall.h"

void Naller::nallSolver(const int threshold, std::vector<unsigned>& batch, int cnt_th) {  // only for fenceCellIds
    auto cmp_congestion = [this](int a, int b) {
        return this->cmp_congestion(a, b);
    };
    int iter_num = 5000*threshold+1000;
    int last_of_cnt = 0;
    int count_equal = 0;
    for (int iter = 0; iter < iter_num; ++iter){
        double p_factor_pThread = 1.0 + 0.7*exp(-10*exp(-0.01*(iter-300)));
        for (auto cellId : batch)  // batch : local variable 
        {
            Cell* sp = ckt.cells[ cellId ];
            // assert(!sp->isFixed_);
            if(sp->isFixed_) {
                continue;
            }
            if( !isStripCongested(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, 1, sp->regionId_) and (iter > threshold)) {
                continue;
            }
            ++sp->ripup_cnt_;
            reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, p_factor_pThread);
            if(sp->ripup_cnt_ > 900) {  //for special case superblue11_a
                route(cellId, true);
                sp->ripup_cnt_ = 0;
            }
            else {
                route(cellId, p_factor_pThread);
            }          
        }
        int of_cnt = countOverflows(batch);
        if(of_cnt == 0 or (iter == iter_num - 1)) {
            return;
        }
        if(abs(of_cnt - last_of_cnt) <= 2) {
            ++count_equal;
        }
        else {
            count_equal = 0;
        }
        if(count_equal == cnt_th) {
            return;
        }

        last_of_cnt = of_cnt;
        if((iter) % 100 == 0)  
            sort(batch.begin(), batch.end(), cmp_congestion);
    }
}

void Naller::nallSolver(const int threshold, const int index) {
    auto cmp_congestion = [this](int a, int b) {
        return this->cmp_congestion(a, b);
    };
    int iter_num = 2400*threshold+600;
    int last_of_cnt = 0;
    int count_equal = 0;

    for (int iter = 0; iter < iter_num; ++iter){
        double p_factor_pThread = 1.0 + 0.7*exp(-10*exp(-0.01*(iter-300)));
        for (auto cellId : batches[index]) // batch : global variable 
        {
            Cell* sp = ckt.cells[ cellId ];
            if( !isStripCongested(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, 1, sp->regionId_) and (iter > threshold)) {
                continue;
            }
            ++sp->ripup_cnt_;
            reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, p_factor_pThread);
            route(cellId, p_factor_pThread);
        }
        int of_cnt = countOverflows(index, threshold);
        if(of_cnt == 0 or (iter == iter_num - 1)) {
            return;
        }
        if(abs(of_cnt - last_of_cnt) <= 2) {
            ++count_equal;
        }
        else {
            count_equal = 0;
        }
        if(count_equal == 25) {//25
            return;
        }
        last_of_cnt = of_cnt;
        if((iter) % 50 == 0)  //noted
            sort(batches[index].begin(), batches[index].end(), cmp_congestion);
    }
}

void Naller::nallSolver(const int threshold, bool doParallel) {
    auto cmp_congestion = [this](int a, int b) {
        return this->cmp_congestion(a, b);
    };
    int iter_num = 2400 * threshold + 600;//3000;//2000*threshold+1000;
    if(doParallel){
        max_disp_p = 1.5;
        omp_set_num_threads(numThreads);
        std::cout << "         deal with fenceCellIds" << std::endl;
        #pragma omp parallel for schedule(dynamic)
        for(unsigned i = 0; i < ckt.fenceRegions.size(); ++i) {
            nallSolver(threshold, ckt.fenceCellIds[i], 25);
        }  
        #pragma omp barrier
        std::cout << "         deal with defaultCellIds" << std::endl;
        max_disp_p = 6.0;
        for (int task = 0; task < 4; ++task)
        {
            #pragma omp parallel for schedule(static) 
            for (int i = 0; i < numThreads; ++i)
            {
                nallSolver(threshold, (int)task_id[task][i]);
            }
            #pragma omp barrier
        }
        
        if(threshold > 0) {
            omp_set_num_threads(1);
            max_disp_p = 1.5;
            for(unsigned i = 0; i < ckt.fenceRegions.size(); ++i) {
                nallSolver(threshold, ckt.fenceCellIds[i], 300000);
            }       
            std::vector<unsigned> of_netIds;
            QueryTree queryTree(ckt.defaultCellIds, ckt);
            int of_count = 0;
            for (int i = 0; i < numBatches; ++i){
                if(batches_of_netIds[i].empty()) {
                    continue;
                }
                for (unsigned id : batches_of_netIds[i])
                {
                    Cell *sp = ckt.cells[id];
                    int cell_half_width = sp->width_ / 2;
                    int central_x = sp->cur_x_ + cell_half_width;
                    int central_y = sp->cur_y_ + sp->height_ / 2;
                    int cell_height_num = sp->height_ / defaultH;
                    int xll = std::max(central_x - cell_half_width - step_space, 0);
                    int yll = std::max(central_y - cell_height_num * step_space, 0);
                    int xur = std::min(central_x + cell_half_width + step_space, g_max_x); //just rect boundary, not cell coord
                    int yur = std::min(central_y + cell_height_num * step_space, g_max_y);
                    Rect<int> rect(xll, yll, xur, yur);
                    queryTree.updateSet(rect, of_count);  
                    ++of_count;
                }
            }
            if(of_count != 0) {
                //init of_netIds 
                for (unsigned i : ckt.defaultCellIds)
                {
                    Cell* sp = ckt.cells[ i ];  
                    if(sp->isFixed_) {
                        continue;
                    }      
                    Rect<int> rect(sp->cur_x_, sp->cur_y_, sp->cur_x_ + sp->width_, sp->cur_y_ + sp->height_);
                    int rectId =  queryTree.conflictedId(rect, -1);
                    if(rectId != -1) {
                        of_netIds.push_back(i);
                    }       
                }
                // max_disp_p = 6.0;
                sort(of_netIds.begin(), of_netIds.end(), cmp_congestion);
                iter_num = 600; //for big case
                int of_cnt = 0;
                for (int iter = 0; iter < iter_num; ++iter){    
                    p_factor = 1.0 + 0.7*exp(-10*exp(-0.01*(iter-300)));
                    for (auto i : of_netIds)
                    {
                        Cell* sp = ckt.cells[ i ]; 
                        assert(!sp->isFixed_);   
                        if(!isStripCongested(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, 1, sp->regionId_) and (iter > 0)) {
                        continue;
                        }
                        reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                        route(i);                 
                    } 
                    of_cnt = countOverflows(of_netIds);
                    if(of_cnt == 0 or (iter == iter_num - 1)) {
                        calDisplacment();
                        log()<<"  iteration: "<<iter<<" of_cnt : "<<of_cnt
                            <<"; s_am : "<<s_am/defaultH<<"; m_max : "<<max_disp/defaultH
                            <<"; avg_disp : "<<avg_disp<<"; max_disp : "<<max_disp<<std::endl;
                        break;
                    } 
                    if((iter) % 100 == 0) 
                        sort(of_netIds.begin(), of_netIds.end(), cmp_congestion);
                } 
                if(of_cnt == 0) {
                    return;
                } 
                iter_num = 6000; //for big case
                for (int iter = 0; iter < iter_num; ++iter){    
                    p_factor = 1.0 + 0.7*exp(-10*exp(-0.01*(iter-300)));
                    for (auto i : of_netIds)
                    {
                        Cell* sp = ckt.cells[ i ]; 
                        assert(!sp->isFixed_);   
                        if(!isStripCongested(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, 1, sp->regionId_) and (iter > 0)) {
                        continue;
                        }
                        reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                        route(i);                 
                    } 
                    int of_cnt = countOverflows(of_netIds);
                    if(of_cnt == 0 or (iter == iter_num - 1)) {
                        calDisplacment();
                        log()<<"  iteration : "<<iter<<" of_cnt : "<<of_cnt
                            <<"; s_am : "<<s_am/defaultH<<"; m_max : "<<max_disp/defaultH
                            <<"; avg_disp : "<<avg_disp<<"; max_disp : "<<max_disp<<std::endl;
                        break;
                    } 
                    if((iter) % 100 == 0) //
                        sort(of_netIds.begin(), of_netIds.end(), cmp_congestion);
                } 
            }
            else {
                calDisplacment();
                log()<<"s_am : "<<s_am/defaultH<<"; m_max : "<<max_disp/defaultH
                    <<"; avg_disp : "<<avg_disp<<"; max_disp : "<<max_disp<<std::endl;
            }
        }
    }
    /**********************single thread*********************************/
    else{
        for(unsigned i = 0; i < ckt.fenceRegions.size(); ++i) {
            nallSolver(threshold, ckt.fenceCellIds[i]);
        } 
        for (int iter = 0; iter < iter_num; ++iter){
            p_factor = 1.0 + 0.7*exp(-10*exp(-0.01*(iter-300)));//1.0 + 0.7*exp(-10*exp(-0.01*(iter-300)))
            for (unsigned i : ckt.defaultCellIds)
            {
                Cell* sp = ckt.cells[ i ];  
                if(sp->isFixed_) {
                continue;
                }
                if(!isStripCongested(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, 1, sp->regionId_) and (iter > threshold)) {
                continue;
                }
                if(sp->cur_x_ > 100000000 or sp->cur_x_ < 0) {
                std::cout << sp->name_ << " 0.2 " << sp->init_x_ << " " << sp->init_y_ << std::endl;
                }
                reduceNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);
                route(i);                     
            } 
            int of_cnt;
            if(iter == iter_num - 1) {
                of_cnt = countOverflows(false);  //follow the serial flow.
            }
            else {
                of_cnt = countOverflows(true);
            }
            if(of_cnt == 0 or (iter == iter_num - 1)) {
                calDisplacment();
                log()<<"  iteration : "<<iter<<" of_cnt : "<<of_cnt
                    <<"; s_am : "<<s_am/defaultH<<"; m_max : "<<max_disp/defaultH
                    <<"; avg_disp : "<<avg_disp<<"; max_disp : "<<max_disp<<std::endl;
                break;
            } 
            // preCalHis();
            if((iter) % 100 == 0) {
                sort(ckt.defaultCellIds.begin(), ckt.defaultCellIds.end(), cmp_congestion);
            }
        } 
    }
}





bool Naller::nall() {
    
    timer::timer time;
    log() << "******************" << "legalization start" << "******************" << std::endl;
    // Preprocessing
    infos_resize();
    // Cell Sorting(defaultCellIds)
    auto cmp = [this](int a, int b) {
        return this->cmp(a, b);
    };
    sort(ckt.defaultCellIds.begin(), ckt.defaultCellIds.end(), cmp);
    // Row&Site Alignment, handle fenceRegion & blockRegion & SpNetRegion
    init_adjust();
    // Cell Sorting(fenceCellIds)
    auto cmp_row = [this](int a, int b) {
        return this->cmp_row(a, b);
    };
    for(unsigned i = 0; i < ckt.fenceRegions.size(); ++i)
        sort(ckt.fenceCellIds[i].begin(), ckt.fenceCellIds[i].end(), cmp_row);
    
    // Check Score
    debug_max_print = false;
    calDisplacment();
    log() <<"  after preprocessing: "
            <<"s_am: "<<s_am/defaultH<<";m_max: "<<max_disp/defaultH
            <<";avg_disp: "<<avg_disp<<";max_disp: "<<max_disp<<std::endl;

    // Multi-threading Preprocessing
    if(doParallel) {
        batches.resize(numBatches);
        batches_of_netIds.resize(numBatches);
        batches_max_area.resize(numBatches, 0);
        init_window(20.0, 0.0, g_max_x, 0.0, g_max_y, ckt.defaultCellIds, 0, 1); //20.0, 1 -> 40.0, 2  
        batches_schedule();  
    }
    // Solving Resource Allocation Task
    // NBLG: Main function of Algorithm
    nallSolver(0, doParallel);
    nallSolver(1, doParallel);
    // Post-Processing
    // tech constraints Preprocessing
    init_edgeType();
    // Greedy Searching
    stripShiftingWithTech();
    /*
        // Cell Swapping: defaultCellIds & fenceCellIds
        // it is not necessary to do stripSwapping due to the absence of tech constraints
        stripSwapping_modified(ckt.defaultCellIds);
        for(auto& fenceCellIds : ckt.fenceCellIds) {
            stripSwapping_modified(fenceCellIds);
        }
    */
    init_edgeType();  
    stripShiftingWithTech();

    log() << "******************" << "legalization completed. Total : " << time.elapsed() 
            << " sec ******************" << std::endl;
    rA = checkAlignment(); 
    if(rA) {
        log() << "  row aligned" << std::endl;

    } else {
        log() << "  row not aligned" << std::endl;
        return false;
    }
    checkFenceRegion();
    if(ckt.doTech) {
        checkTech();
    } 
    return true;
}