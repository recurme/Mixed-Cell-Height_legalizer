#ifndef QUEUE_H_
#define QUEUE_H_
#include "global.h"


template<typename _Key, typename _Compare>
class HeapPriorityQueue {
  public:
   HeapPriorityQueue() {
     compare_ = _Compare();
     std::make_heap(elements_.begin(), elements_.end(), compare_);
   }



   bool empty() const { return elements_.empty(); }
   

   int size() const { return static_cast<int>(elements_.size()); }



   _Key& top() { 
    return elements_.front(); }

   void push(const int id, const double cost, const double shiftCost)
   {
     elements_.emplace_back(id, cost, shiftCost);
     std::push_heap(elements_.begin(), elements_.end(), compare_);
   }

   void pop() {

     std::pop_heap(elements_.begin(), elements_.end(), compare_);
     elements_.pop_back();
   }


  void clear() {

    elements_.clear();
    
  }

  private:
   std::vector<_Key> elements_;
   _Compare compare_;

};

class PQStrip {
 public:
  PQStrip() : group_(-1) { }
  PQStrip( const int group,  const double cost,
    const double shiftCost)
      :  group_(group),  cost_(cost), shiftCost_(shiftCost) { }

 public:
  int group_;
  double cost_;
  double shiftCost_;
};

#endif
