/*
 * BranchSegment.h
 *
 *  Created on: Aug 16, 2009
 *      Author: smitty
 */

#ifndef _BRANCH_SEGMENT_H_
#define _BRANCH_SEGMENT_H_

//#include <vector>

class RateModel;

#include "vector_node_object.h"
#include "superdouble.h"


class BranchSegment{
private:
    double duration;
    int period;
    RateModel * model;
    std::vector<int> fossilareaindices;
    int startdistint;
    
public:
    BranchSegment(double dur,int per);
    void setModel(RateModel * mod);
    //void setStartDist(vector<int> sd);
    void clearStartDist();
    double getDuration();
    int getPeriod();
    //vector<int> getStartDist();
    void set_start_dist_int(int d);
    int get_start_dist_int();
    RateModel * getModel();
    std::vector<int> getFossilAreas();
    void setFossilArea(int area);
    std::vector<Superdouble> * distconds;
    std::vector<Superdouble> alphas; // alpha for the entire branch -- stored in the 0th segment for anc calc
    std::vector<Superdouble> seg_sp_alphas; // alpha for this specific segment, stored for the stoch map
    std::vector<Superdouble> seg_sp_stoch_map_revB_time; //segment specific rev B, combining the tempA and the ENLT
    std::vector<Superdouble> seg_sp_stoch_map_revB_number; //segment specific rev B, combining the tempA and the ENLT
    std::vector<Superdouble> * ancdistconds;//for ancestral state reconstructions
};

#endif /* _BRANCH_SEGMENT_H_ */
