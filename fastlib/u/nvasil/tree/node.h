#ifndef NODE_H_
#define NODE_H_
#include <new>
#include <limits>
#include "u/nvasil/loki/TypeTraits.h"
#include "u/nvasil/loki/Typelist.h"
#include "fastlib/fastlib.h"
#include "point.h"
#include "point_identity_discriminator.h"
#include "computations_counter.h"
#include "u/nvasil/dataset/binary_dataset.h"

template<typename TYPELIST,
         bool diagnostic>
class Node {
 public:
  typedef typename Loki::TL::TypeAt<TYPELIST, 0>::Result Precision_t;
	typedef typename Loki::TL::TypeAt<TYPELIST, 1>::Result Allocator_t;
  typedef typename Loki::TL::TypeAt<TYPELIST, 2>::Result Metric_t;
	typedef typename Loki::TL::TypeAt<TYPELIST, 3>::Result BoundingBox_t;
	typedef typename Loki::TL::TypeAt<TYPELIST, 4>::Result NodeCachedStatistics_t;
	typedef Allocator_t::template ArrayPtr<Precision_t> Array_t;
  typedef Node<TYPELIST, diagnostic> Node_t;
	typedef Allocator_t::template Ptr<Node> NodePtr_t;
	typedef Point<Precision_t, Allocator_t> Point_t;
	friend class NodeTest<TYPELIST, diagnostic>; 
	struct NNResult {
		NNResult() : point_id_(0),
      distance_(numeric_limits<Precision_t>::max()) {
		}
		bool operator<(const NNResult &other) const {
		  if (distance_==other.distance_ || true) {
			  return point_id_<other.point_id_;  
			}
			return distance_<other.distance_;
		}
		Precision_t get_distance() const {
		  return distance_;
		}
		index_t get_point_id() {
		  return point_id_; 
		}
	  index_t point_id_;
    Point_t nearest_;
		Precision_t	distance_;
	};
	
	Node() {	
	}
  // Use this for node
  Init(const BoundingBox_t &box, 
	     const NodeCachedStatistics_t &statistics,		
			 index_t node_id,
			 index_t num_of_points);  
  // Use this for leaf
  Init(const BoundingBox_t &box,
			 const NodeCachedStatistics_t &statistics,
			 index_t node_id,
			 index_t num_of_points,
       BinaryDataset<Precision_t> *dataset,
			 index_t start,
			 int32 dimension); 
  ~Node();
  static void *operator new(size_t size);
  static void  operator delete(void *p);
  bool IsLeaf() {
		return !points_.IsNULL();
	}
	template<typename POINTTYPE>                   	
  pair<Allocator_t::template Ptr<NODETYPE>, 
		   Allocator_t::template Ptr<NODETYPE> > 
			 ClosestChild(POINTTYPE point, int32 dimension,
					 ComputationsCounter<diagnostic> &comp);
  pair<pair<Allocator_t::template Ptr<NODETYPE>, Precision_t>, 
	     pair<Allocator_t::template Ptr<NODETYPE>, Precision_t> > 
			 ClosestNode(Allocator_t::template Ptr<NODETYPE>,
					         Allocator_t::template Ptr<NODETYPE>,
									 int32 dimension,
							     ComputationsCounter<diagnostic> &comp);

// This one is using a custom discriminator  
// We use this for timit experiments so that we exclude points
// from the same speaker
	template<typename POINTTYPE, typename NEIGHBORTYPE>
  void FindNearest(POINTTYPE query_point, 
			             vector<pair<Precision_t, Point_t> &nearest, 
                   Precision_t &distance, NEIGHBORTYPE range, int32 dimension,
                   PointIdentityDiscriminator<IDPrecision_t> &discriminator,
									 ComputationsCounter<diagnostic> &comp);

// This one store the results directly on a memmory mapped file
// for k-nearest neighbors and to a normal file for range nearest neighbors
// very efficient for large datasets
// Uses a custom descriminator
  template<typename NEIGHBORTYPE>
  void FindAllNearest(Allocator_t::template  Ptr<NODETYPE> query_node,
                      Precision_t &max_neighbor_distance,
                      int32 range,
                      int32 dimension,
                      PointIdentityDiscriminator<IDPrecision_t> &discriminator,
                      ComputationsCounter<diagnostic> &comp);
  
  Allocator_t::template Ptr<NODETYPE>& get_left() {
  	return left_;
  }
  Allocator_t::template Ptr<NODETYPE>& get_right() {
  	return right_;
  }
  BoundingBox_t &get_box() {
	  return box_;
	}
  Allocator_t::template ArrayPtr<Point_t>&
	get_points() {
		  return points_;
	}
	index_t get_num_of_points() {
	  return num_of_points_;
	}
	
	NNResult *get_kneighbors() {
	  return kneighbors_;  
	}
	void set_kneighbors(NNResult *chunk, uint32 knns) {
	  kneighbors_=chunk;
		for(index_t i=0; i< num_of_points; i++) {
		  for(index_t j=0; j<knns; j++) {
	      kneighbors_[i*range+j].	kneighbors_[i*range+j].point_id_ =
				index_[i];	
			}
		}
	}
	
	void set_range_neighbors(FILE *fp) {
    range_nn_fp=fp;	
	}
	FILE *get_range_nn_fp() {
	  return range_nn_fp_;
	}
	Precision_t get_min_dist_so_far() {
	  return min_dist_so_far_;
	}
	void set_min_dist_so_far(Precision_t distance) {
	  min_dist_so_far_=distance;
	}
 private:
	BoundingBox_t box_;
  index_t node_id_;
  NodePtr_t left_;
  NodePtr_t right_;
	Allocator_t::template ArrayPtr<index_t> index_;
  Allocator_t::template ArrayPtr<Precision_t> points_;
	Allocator_t::template Ptr<NodeCachedStatistics> statistics_;
  index_t num_of_points_;
	union {
    NNResult *kneighbors_;
		FILE *range_nn_fp_;
	}
	Precision_t  min_dist_so_far_;	
};

#include "node_impl.h"
#endif /*NODE_H_*/
