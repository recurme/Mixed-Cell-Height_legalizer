#pragma once
#include "queue.h"
#include "rtree.h"
#include "parser.h"  //inquire macroType

class PQStripCompare {
public:
  bool operator() (const PQStrip& na, const PQStrip& nb) const {
    //return na.cost_ > nb.cost_;
  return na.cost_ - nb.cost_ >=  EPSILON ||           
           (fabs(nb.cost_ - na.cost_) < EPSILON &&
            na.shiftCost_ > nb.shiftCost_);
  }
};

typedef HeapPriorityQueue<PQStrip, PQStripCompare> pqueue_t;
// map grid
class NodeInfo {
public:
    NodeInfo() {
        regionId_ = -1;//default : whiteSpace
        layer_ = -1;//default : whiteSpace
        lEdgeT_ = -1;
        rEdgeT_ = -1;
        usage_ = 0;  //0  ; 
        cap_ = 1;
        pCost_ = 1.0;  //1.0
        hisCost_ = 1.0;  //1.0
        pathCost_ = 1.0;  //1.0
        updated_ = false;
        }
public:
    int regionId_;
    int layer_; //pg rail check for pin short/access. we now only check layer2's error.
    int lEdgeT_;
    int rEdgeT_;
    int usage_;
    int cap_;
    double hisCost_;
    double pCost_;
    double pathCost_;
    bool updated_;
};
// map grids
class Node {
public:
  std::vector< NodeInfo > node_infos;
public:
    void clear() {
        for (int i = 0; i < node_infos.size(); ++i) {
            node_infos[i].usage_ = 0;
            node_infos[i].pCost_ = 1.0;
            node_infos[i].hisCost_ = 1.0;
            node_infos[i].pathCost_ = 1.0;
        }
    }
};


class Naller {
public:
    Naller(circuit& ckt_): ckt(ckt_){
        g_max_x = ckt.g_max_x, g_max_y = ckt.g_max_y, defaultH = ckt.defaultH;
        doParallel = ckt.doParallel;
        rA = false;
        num_unfixed_inst_ = ckt.num_unfixed_inst_;
        min_width = ckt.min_width;
        cell_num = ckt.cell_num;

        max_disp_p = 1.5;
        numThreads = (int)pow(4, 1);  //1 : 4-threads ; 2 : 16threads
        numBatches = numThreads * 4;
        thread_xsize = 8; // 20 * 20
        thread_ysize = 4; // 20 * 20
        p_factor = 1.0;  
        g_alpha = 1.0;
        mi = 1;
    }
    bool nall();
private:
    void infos_resize();
    void init_adjust();
    void addBlockCost();
    void moveIntoFence(Cell *sp, Rect<int> &rect);
    void initFenceRegion();
    void initSpNetRegion();
    int inBlockId(int s_x, int s_y, int width, int height);
    void moveOutBlock(const int cellId, const int blockId);
    void calTempCost(double& cost_temp, const int& s_x, const int& s_y, const int& width, 
                  const int& height, const std::vector<Macro_pin>& signal_pins, int regionId = -1);
    void routeISPD15(const int cellId);
    void route(const int cellId, double& p_factor_pThread);
    void route(const int cellId, const bool moveOut2flag);
    void route(const int cellId);
    bool outBox(const int& s_x, const int& s_y, const int& width, const int& height);
    void addNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height);
    void addNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height, double& p_factor_pThread);
    void reduceNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height);
    void reduceNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height, double& p_factor_pThread);
    bool moveOutFence(Cell *sp, int count_outRegion);
    void moveOutSpNetRegion(int cellId, const int& s_x, const int& s_y, const int& width, 
        const int& height, const std::vector<Macro_pin>& signal_pins);
    void calDisplacment();
    void init_window(double step0, double min_x, double max_x, double min_y, 
                      double max_y, std::vector<unsigned> &sub_cellIds, int group = -1, int level = 1);
    void batches_schedule();
    void nallSolver(const int threshold, bool doParallel);
    bool isStripCongested(const int& s_x, const int& s_y,
                        const int& width, const int& height, const int& lBound, int regionId = -1);

    void nallSolver(const int threshold, const int index);
    void nallSolver(const int threshold, std::vector<unsigned>& batch, int cnt_th = 50);
    int countOverflows(const std::vector<unsigned>& batch);
    int countOverflows(int index, int threshold);
    int countOverflows(bool update_his);
    void calCellOf(const int& s_x, const int& s_y, const int& width, const int& height, 
                    int& of, int& of_cnt, bool update_his = true);
    void init_edgeType();
    void stripShiftingWithTech();
    void reduceNodeCostWithEdgeT(const int& s_x, const int& s_y, 
                          const int& width, const int& height);
    void postRouteWithTech(const int& cellId, const int& max_space);
    void addNodeCostWithEdgeT(const int& s_x, const int& s_y, const int& width, const int& height, 
                        const int& lEdgeT, const int& rEdgeT);
    bool checkAlignment();
    bool checkFenceRegion();
    void checkTech();
    void stripSwapping_modified(std::vector<unsigned int>& cellIds);

    bool cmp(unsigned id1, unsigned id2);
    bool cmp_row(unsigned id1, unsigned id2);
    bool cmp_congestion(int id1, int id2);
    bool cmpT_congestion(unsigned a, unsigned b);
    bool cmp_distance(unsigned id1, unsigned id2);
private:
    static const int dir_array_x[49];
    static const int dir_array_x2[57];
    static const int dir_array_x3[77];
    static const int dir_array_x4[67];
    static const int dir_array_y[49];
    static const int dir_array_y2[57];
    static const int dir_array_y3[77];
    static const int dir_array_y4[67];
    static const int total_direc_num;
    static const int total_direc_num2;
    static const int total_direc_num3;
    static const int total_direc_num4;

    int numThreads;  //1 : 4-threads ; 2 : 16threads
    int numBatches;
    int thread_xsize; // 20 * 20
    int thread_ysize; // 20 * 20
    int tx_size;  //if 1280 * 640 ,then 64 * 32   2240;1168
    int ty_size;  
    int mi;
    int step_space;// multi-thread mode: cell local window (sites) 40 -> 280 (superbiules)
    int step_window;
    double p_factor;  
    double g_alpha;  
    double max_disp_th;  //30
    double max_disp_p;
    double penalty_tech; // 10 per defaultH
    
    std::vector<std::vector<int>> bin;
    std::vector<std::vector<int>> window;  
    std::vector<std::vector<unsigned>> batches;
    std::vector<std::vector<unsigned>> batches_of_netIds;
    std::vector<int> batches_max_area;
    std::vector<std::vector<unsigned>> task_id;
    Node node;
    pqueue_t priority_queue_;
    
    // global variables
    circuit& ckt;
    double s_am;
    double avg_disp;
    double max_disp;
    bool debug_max_print = false;
    std::vector<double> temp_usageVec;
    std::set<int> stripType;  //encoder for different height and width
    int Np;
    int Ne;
    int g_max_x;
    int g_max_y;
    int defaultH;
    bool doParallel;
    bool rA;
    unsigned num_unfixed_inst_;
    double min_width;
    unsigned cell_num;
};