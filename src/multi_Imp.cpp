#include "nall.h"

//Danamically complete load balancing (area instead of num): 4 x 4 windows.
void Naller::init_window(double step0, double min_x, double max_x, double min_y, 
                      double max_y, std::vector<unsigned> &sub_cellIds, int group, int level) {
    double min_width = ckt.min_width;
    int dir_x[4][4] = {{0, 1, 0, 1}, {-1, 0, -1, 0}, {0, 1, 0, 1}, {-1, 0, -1, 0}};
    int dir_y[4][4] = {{0, 0, 1, 1}, {0, 0, 1, 1}, {-1, -1, 0, 0}, {-1, -1, 0, 0}};
    double step = step0;
    double CURX = 0.5 * (min_x + max_x);              // ---------
    double CURY = 0.5 * (min_y + max_y);              // |   |   |
    std::vector<std::vector<unsigned>> window;        // |---|---|
    window.resize( 4 );                               // |   |   |
    int last_dir_x = 0;                               // ---------
    int last_dir_y = 0;                      
    double last_ratio = 0.0;                                      
    int count_equal = 0;
    int count_area0 = 0;
    double temp_x = 0.0;
    double temp_y = 0.0;
    double last_x = 0.0;
    double last_y = 0.0;
    // log() << "  total cell_num " << cell_num << std::endl;
    std::vector<int> area_total(4, 0);
    std::vector<int> area_max(4, 0);
    while(1) {
        double space_x = CURX - min_x;  //
        double space_y = CURY - min_y;
        for (unsigned i : sub_cellIds)  //
        {
        Cell* sp = ckt.cells[ i ];
        if(sp->isFixed_){  //noted
            continue;
        }
        int x_index = std::min(int((sp->cur_x_ + 0.0 - min_x) / space_x), 1);
        int y_index = std::min(int((sp->cur_y_ + 0.0 - min_y) / space_y), 1); 

        window[ x_index + 2 * y_index ].push_back(i);
        int cur_area = sp->width_ * sp->height_;
        area_max[ x_index + 2 * y_index ] = std::max(area_max[ x_index + 2 * y_index ], cur_area);
        area_total[ x_index + 2 * y_index ] += sp->width_ * sp->height_;
        }
        //
        int min_num = INT_MAX;
        int min_index = 0;
        int max_num = -1;
        int max_index = 0;
        for (int i = 0; i < 4; ++i)
        {
            int cur_num = area_total[i];//area instead of num.
            if(cur_num < min_num) {
                min_num = cur_num;
                min_index = i;
            }
            if(cur_num > max_num) {
                max_num = cur_num;
                max_index = i;
            }
        } 
        if(min_num == 0) {
        if(max_num <= (10.0 / min_width / min_width)) {
            if(level <= 0)
                log() << "  group : " << group << "  CURX : " << CURX * min_width << " CURY : " << CURY * min_width << std::endl;         
            break;
        }
        if(count_area0++ >= 5) {
            if(dir_x[min_index][max_index] != 0) {
                temp_x = 0.5 * (CURX + last_x);
                last_x = CURX;
                CURX = temp_x;
            }       
            if(dir_y[min_index][max_index] != 0) {
                temp_y = 0.5 * (CURY + last_y);
                last_y = CURY;
                CURY = temp_y;          
            } 
        }
        else {
            if(dir_x[min_index][max_index] == 1) {
                last_x = CURX;
                CURX = 0.5 * (CURX + max_x);
            }
            else if(dir_x[min_index][max_index] == -1) {
                last_x = CURX;
                CURX = 0.5 * (CURX + min_x);
            }
            if(dir_y[min_index][max_index] == 1) {
                last_y = CURY;
                CURY = 0.5 * (CURY + max_y);
            }
            else if(dir_y[min_index][max_index] == -1) {
                last_y = CURY;
                CURY = 0.5 * (CURY + min_y);
            }          
        }

        }
        else {
            count_area0 = 0;
            double ratio = (max_num + 0.0 - min_num)  / min_num;
            // log() << "  ratio :" << ratio << std::endl; 
            if(ratio < 0.1 or count_equal == 5) {
                if(level <= 0)
                log() << "  group : " << group << "  CURX : " << CURX * min_width << " CURY : " << CURY * min_width << std::endl;        
                break;
            }
        
            if(fabs(ratio - last_ratio) < 0.001) {
                ++count_equal;
            }
            else {
                count_equal = 0;
            }
            int dir_x0 = dir_x[min_index][max_index];
            int dir_y0 = dir_y[min_index][max_index];
            //reduce step
            if(dir_x0 * last_dir_x == -1 or dir_y0 * last_dir_y == -1) {
                step = 0.5 * step;
            }
            last_dir_x = dir_x0;
            last_dir_y = dir_y0;
            last_ratio = ratio; 
            double CURX_temp = CURX + step * ratio * dir_x0;
            double CURY_temp = CURY + step * ratio * dir_y0;
            if(CURX_temp > max_x or fabs(CURX_temp - max_x) < 1) {
                step = 0.5 * step;
                CURX = 0.5 * (CURX + max_x);
            }
            else if(CURX_temp < min_x or fabs(CURX_temp - min_x) < 1) {
                step = 0.5 * step;
                CURX = 0.5 * (CURX + min_x);      
            }
            else {
                CURX = CURX_temp; //CURX_temp is safe number
            }
            if(CURY_temp > max_y or fabs(CURY_temp - max_y) < 1) {
                step = 0.5 * step;
                CURY = 0.5 * (CURY + max_y);
            }
            else if(CURY_temp < min_y or fabs(CURY_temp - min_y) < 1) {
                step = 0.5 * step;
                CURY = 0.5 * (CURY + min_y);      
            }
            else {
                CURY = CURY_temp;
            }

        }
        //clear window
        for (int i = 0; i < 4; ++i)
        {
            window[i].clear();
            area_total[i] = 0;
            area_max[i] = 0;
        }       
    }  // end while
    assert(CURX < max_x);
    assert(min_x < CURX);
    assert(min_y < CURY);
    assert(CURY < max_y);
    if(level <= 0) {  //
        for (int i = 0; i < 4; ++i)
        {
            // batches.push_back(window[i]);
            batches_max_area[ group * 4 + i ] = area_max[i];
            batches[group * 4 + i].assign(window[i].begin(), window[i].end());
        }
        return;
    }
    //dividing and recursion
    init_window(0.5 * step0, min_x, CURX, min_y, CURY, window[0], group * 4 + 0, level - 1);
    init_window(0.5 * step0, CURX, max_x, min_y, CURY, window[1], group * 4 + 1, level - 1);
    init_window(0.5 * step0, min_x, CURX, CURY, max_y, window[2], group * 4 + 2, level - 1);
    init_window(0.5 * step0, CURX, max_x, CURY, max_y, window[3], group * 4 + 3, level - 1);  
}

void Naller::batches_schedule() {
    std::vector<int> batches_group_id(4);
    for (int i = 0; i < 4; ++i)// {0 <- max{0, 4, 8, 12}, 1 <- max{1, 5, 9, 13}, ...}
    {
        batches_group_id[i] = i;
        for (int j = 1; j < numThreads; ++j)   //4 threads-> 16 threads
        {
            batches_max_area[i] = std::max(batches_max_area[i], batches_max_area[i + j * 4]);  
        }
    }
    sort(batches_group_id.begin(), batches_group_id.end(), [&](int lhs, int rhs){
        return batches_max_area[lhs] < batches_max_area[rhs];
    });
    log() << "  task_group_order: " << batches_group_id[0] << " -> " << batches_group_id[1] << " -> " <<
    batches_group_id[2] << " -> " << batches_group_id[3] << std::endl;
    task_id.resize(4);
    for (int i = 0; i < 4; ++i)//
    {
        for (int j = 0; j < numThreads; ++j) //4 threads-> 16 threads
        {
            task_id[i].push_back(batches_group_id[i]+ j * 4);  //mabye { {1, 5, 9, 13},{0, 4, 8, 12}, ...}
            // printlog(LOG_INFO, "task_id[%d][%d] is %d", i, j, batches_group_id[i]+ j * 4);
        }
    }
}
