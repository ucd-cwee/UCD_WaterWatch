/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#if 1
#pragma once
#include "Precompiled.h"
//#include "BalancedTree.h"
//#include "cwee_math.h"
#include "vec.h"
//#include "Engineering.h"
#include "cweeJob.h"
#include "SharedPtr.h"
#include "InterpolatedMatrix.h"
#include "cweeThreadedMap.h"
#include "Voronoi.h"
#include "Geocoding.h"
#include <ppl.h>
#include "KMeans.h"

class cweeBoundary {
public:
	static cwee_units::foot_t Distance(vec2d const& LongLat1, vec2d const& LongLat2, bool geographic = true) {		
		if (!geographic) {
			return LongLat1.Distance(LongLat2);
		} 
		else {
			double  lat_old, lat_new, lat_diff, lng_diff, a, c;
			if (LongLat1.x >= -180.0 && LongLat1.x <= 180.0 && LongLat1.y >= -90.0 && LongLat1.y <= 90.0 &&
				LongLat2.x >= -180.0 && LongLat2.x <= 180.0 && LongLat2.y >= -90.0 && LongLat2.y <= 90.0) {
				lat_old = LongLat1.y * cweeMath::PI / 180.0;
				lat_new = LongLat2.y * cweeMath::PI / 180.0;
				lat_diff = (LongLat2.y - LongLat1.y) * cweeMath::PI / 180.0;
				lng_diff = (LongLat2.x - LongLat1.x) * cweeMath::PI / 180.0;

				a = std::sin(lat_diff / 2.0) * std::sin(lat_diff / 2.0) + std::cos(lat_new) * std::cos(lat_old) * std::sin(lng_diff / 2.0) * std::sin(lng_diff / 2.0);
				c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

				return units::length::meter_t(6372797.56085 * c);
			}
			else {
				return LongLat1.Distance(LongLat2);
			}
		}
	};
	static vec2d ClosestPoint(vec2d const& pointCoord, cweeList<vec2d> const& lineCoords, bool geographic = true) {
		if (lineCoords.Num() == 0) {
			return pointCoord;
		}
		else if (lineCoords.Num() == 1) {
			return lineCoords[0];
		}
		else {
			cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();
			vec2d toReturn = lineCoords[0];
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				// Calculate the vector AB (direction of the line segment)
				double ABx = B[0] - A[0];
				double ABy = B[1] - A[1];

				// Calculate the vector AP (from point A to point P)
				double APx = P[0] - A[0];
				double APy = P[1] - A[1];

				// Calculate the dot product of AB and AP
				double dotProduct = ABx * APx + ABy * APy;

				// Calculate the squared length of AB
				double lengthABsq = ABx * ABx + ABy * ABy;

				// Calculate the parameter t (projection of AP onto AB)
				double t = dotProduct / lengthABsq;

				// If t is outside the segment [0, 1], find the distance to the closest endpoint
				if (t < 0.0) {
					return A; // Distance to point A
				}
				else if (t > 1.0) {
					return B; // Distance to point B
				}

				// Calculate the closest point on the line segment
				double closestX = A[0] + t * ABx;
				double closestY = A[1] + t * ABy;

				return vec2d(closestX, closestY);
			};
			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end = Distance(pointCoord, lineCoords[0], geographic);
			cwee_units::foot_t dist_perp;

			for (int i = 1; i < lineCoords.Num(); i += 1) {
				dist_start = dist_end;
				dist_end = Distance(pointCoord, lineCoords[i], geographic);
				auto closest = closestPointOnLineSegment(lineCoords[i - 1], lineCoords[i], pointCoord);
				dist_perp = Distance(pointCoord, closest, geographic);

				if (out > dist_perp) {
					out = dist_perp;
					toReturn = closest;
				}
				if (out > dist_end) {
					out = dist_end;
					toReturn = lineCoords[i];
				}
				if (out > dist_start) {
					out = dist_start;
					toReturn = lineCoords[i - 1];
				}
			}
			return toReturn;
		}
	};
	static cwee_units::foot_t Distance_Point_Line(vec2d const& pointCoord, cweeList<vec2d> const& line, bool geographic = true) {
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();

		auto numLinePoints = line.Num();

		if (numLinePoints == 0) {
			out = std::numeric_limits<decltype(out)>::max();
			return out;
		}
		auto lineCoords0{ line[0] };

		if (numLinePoints == 1) {
			out = Distance(pointCoord, lineCoords0, geographic);
		}
		else {
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				// Calculate the vector AB (direction of the line segment)
				double ABx = B[0] - A[0];
				double ABy = B[1] - A[1];

				// Calculate the vector AP (from point A to point P)
				double APx = P[0] - A[0];
				double APy = P[1] - A[1];

				// Calculate the dot product of AB and AP
				double dotProduct = ABx * APx + ABy * APy;

				// Calculate the squared length of AB
				double lengthABsq = ABx * ABx + ABy * ABy;

				// Calculate the parameter t (projection of AP onto AB)
				double t = dotProduct / lengthABsq;

				// If t is outside the segment [0, 1], find the distance to the closest endpoint
				if (t < 0.0) {
					return A; // Distance to point A
				}
				else if (t > 1.0) {
					return B; // Distance to point B
				}

				// Calculate the closest point on the line segment
				double closestX = A[0] + t * ABx;
				double closestY = A[1] + t * ABy;

				return vec2d(closestX, closestY);
			};
			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end = Distance(pointCoord, lineCoords0, geographic);
			cwee_units::foot_t dist_perp;
			vec2d prevPoint = lineCoords0;
			vec2d currentPoint;
			for (int i = 1; i < numLinePoints; i += 1) {
				currentPoint = line[i];
				dist_start = dist_end;
				dist_end = Distance(pointCoord, currentPoint, geographic);
				dist_perp = Distance(pointCoord, closestPointOnLineSegment(prevPoint, currentPoint, pointCoord), geographic);
				out = cwee_units::math::fmin(cwee_units::math::fmin(cwee_units::math::fmin(out, dist_perp), dist_end), dist_start);
				prevPoint = currentPoint;
			}
		}
		return out;
	};
	static cwee_units::foot_t Distance_Point_Polygon(vec2d const& pointCoord, cweeList<vec2d> const& line, bool geographic = true) {
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();

		auto numLinePoints = line.Num();

		if (numLinePoints == 0) {
			out = std::numeric_limits<decltype(out)>::max();
			return out;
		}
		auto lineCoords0{ line[0] };

		if (numLinePoints == 1) {
			out = Distance(pointCoord, lineCoords0, geographic);
		}
		else {
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				// Calculate the vector AB (direction of the line segment)
				double ABx = B[0] - A[0];
				double ABy = B[1] - A[1];

				// Calculate the vector AP (from point A to point P)
				double APx = P[0] - A[0];
				double APy = P[1] - A[1];

				// Calculate the dot product of AB and AP
				double dotProduct = ABx * APx + ABy * APy;

				// Calculate the squared length of AB
				double lengthABsq = ABx * ABx + ABy * ABy;

				// Calculate the parameter t (projection of AP onto AB)
				double t = dotProduct / lengthABsq;

				// If t is outside the segment [0, 1], find the distance to the closest endpoint
				if (t < 0.0) {
					return A; // Distance to point A
				}
				else if (t > 1.0) {
					return B; // Distance to point B
				}

				// Calculate the closest point on the line segment
				double closestX = A[0] + t * ABx;
				double closestY = A[1] + t * ABy;

				return vec2d(closestX, closestY);
			};

			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end = Distance(pointCoord, lineCoords0, geographic);
			cwee_units::foot_t dist_perp;
			vec2d prevPoint = lineCoords0;
			vec2d currentPoint;
			for (int i = 1; i < numLinePoints; i += 1) {
				currentPoint = line[i];
				dist_start = dist_end;
				dist_end = Distance(pointCoord, currentPoint, geographic);
				dist_perp = Distance(pointCoord, closestPointOnLineSegment(prevPoint, currentPoint, pointCoord), geographic);
				out = cwee_units::math::fmin(cwee_units::math::fmin(cwee_units::math::fmin(out, dist_perp), dist_end), dist_start);
				prevPoint = currentPoint;
			}
		}
		{
			cweeList<vec2d> coords;
			coords.SetGranularity(numLinePoints + 1);
			for (int i = 0; i < numLinePoints; i++) {
				coords.Append(line[i]);
			}
			if (cweeEng::IsPointInPolygon(coords, pointCoord)) {
				out = cwee_units::foot_t(0);
			}
		}

		return out;
	};
	static bool Overlaps(cweeList<vec2d> const& coords1, cweeList<vec2d> const& coords2) {
		bool out{ false };

		for (auto& coord : coords2) {
			if (cweeEng::IsPointInPolygon(coords1, coord)) {
				out = true;
				break;
			}
		}

		return out;
	};
	static cwee_units::foot_t Distance(vec2d const& pointCoord, cweeList<vec2d> const& lineCoords, bool geographic = true) {
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();
		if (lineCoords.Num() == 0) out = std::numeric_limits<decltype(out)>::max();		
		else if (lineCoords.Num() == 1) out = Distance(pointCoord, lineCoords[0], geographic);		
		else {
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				double
					ABx{ B.x - A.x },  // Calculate the vector AB (direction of the line segment)
					ABy{ B.y - A.y },
					APx{ P.x - A.x },  // Calculate the vector AP (from point A to point P)
					APy{ P.y - A.y };							
				double
					dotProduct{ ABx * APx + ABy * APy }, // Calculate the dot product of AB and AP
					lengthABsq{ ABx * ABx + ABy * ABy };  // Calculate the squared length of AB								
				double 
					t{ dotProduct / lengthABsq }; // Calculate the parameter t (projection of AP onto AB)
				
				if (t < 0.0) return A;				
				else if (t > 1.0) return B;				
				else return vec2d(A[0] + t * ABx, A[1] + t * ABy);
			};
			
			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end{ Distance(pointCoord, lineCoords[0], geographic) };
			cwee_units::foot_t dist_perp;

			for (int i = 1; i < lineCoords.Num(); i += 1) {
				dist_start = dist_end;
				dist_end = Distance(pointCoord, lineCoords[i], geographic);
				dist_perp = Distance(pointCoord, closestPointOnLineSegment(lineCoords[i - 1], lineCoords[i], pointCoord), geographic);
				out = cwee_units::math::fmin(cwee_units::math::fmin(cwee_units::math::fmin(out, dist_perp), dist_end), dist_start);
			}
		}
		return out;
	};
	static cwee_units::foot_t Distance(cweeList<vec2d> const& coords1, cweeList<vec2d> const& coords2, bool geographic = true) {
		using namespace cwee_units;

		cwee_units::foot_t 
			out = std::numeric_limits<cwee_units::foot_t>::max(), 
			d;

		if (coords1.Num() == 0 || coords2.Num() == 0) return out;
		if (coords1.Num() == 1) return Distance(coords1[0], coords2, geographic);
		else if (coords2.Num() == 1) return Distance(coords2[0], coords1, geographic);

		// intersections of lines (Does not test if end-points overlap)
		//			.
		//	........*.....
		//			.
		//			.
		if (true) {
			// Return true if line segments AB and CD intersect
			auto ccw = [](vec2d const& A, vec2d const& B, vec2d const& C) {
				return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
			};
			auto intersect = [ccw](vec2d const& A, vec2d const& B, vec2d const& C, vec2d const& D) {
				return ccw(A, C, D) != ccw(B, C, D) and ccw(A, B, C) != ccw(A, B, D);
			};
			for (int i = 1; i < coords1.Num(); i++) {
				for (int j = 1; j < coords2.Num(); j++) {
					if (intersect(coords1[i - 1], coords1[i], coords2[j - 1], coords2[j])) {
						return 0_ft;
					}
				}
			}
		}

		// distance between end-points.
		//		*..........*
		//				
		//				*....*
		for (int i = 0; i < coords1.Num(); i++) {
			for (int j = 0; j < coords2.Num(); j++) {
				if (coords1[i] == coords2[j]) { return 0_ft; }
				d = Distance(coords1[i], coords2[j], geographic);
				if (d < out) {
					d = out;
				}
			}
		}

		// nearest perpendicular points for end-points
		//		.....*......
		//			  
		//			 *				
		//			.
		//		   .
		//        .
		for (int i = coords1.Num() - 1; i >= 0; --i) {
			d = Distance(coords1[i], coords2, geographic);
			if (d < out) {
				out = d;
			}
		}
		for (int i = coords2.Num() - 1; i >= 0; --i) {
			d = Distance(coords2[i], coords1, geographic);
			if (d < out) {
				out = d;
			}
		}
		return out;
	};
	static cwee_units::foot_t Distance(vec2d const& pointCoord, cweeBoundary const& bound, bool geographic = true) {		
		if (Contains(bound, pointCoord)) {
			return 0;
		}
		else {
			if (geographic) {
				// have to deal with curve of the earth
				return Distance_Point_Polygon(pointCoord, bound.GetCoords(), geographic);
			} else {
				if (pointCoord.x < bound.bottomLeft.x) {
					if (pointCoord.y < bound.bottomLeft.y) // BELOW LEFT						
						return Distance(pointCoord, bound.bottomLeft, false);
					else if (pointCoord.y > bound.topRight.y) // ABOVE LEFT						
						return Distance(pointCoord, vec2d(bound.bottomLeft.x, bound.topRight.y), false);
					else // CENTER LEFT						
						return bound.bottomLeft.x - pointCoord.x;
				}
				else if (pointCoord.x > bound.topRight.x) {
					if (pointCoord.y < bound.bottomLeft.y) // BELOW RIGHT						
						return Distance(pointCoord, vec2d(bound.topRight.x, bound.bottomLeft.y), false);
					else if (pointCoord.y > bound.topRight.y) // ABOVE RIGHT						
						return Distance(pointCoord, bound.topRight, false);
					else // CENTER RIGHT						
						return pointCoord.x - bound.topRight.x;
				}
				else {
					// point is above or below the boundary. 
					if (pointCoord.y < bound.bottomLeft.y) // BELOW CENTER						
						return bound.bottomLeft.y - pointCoord.y;
					else if (pointCoord.y > bound.topRight.y) // ABOVE CENTER						
						return pointCoord.y - bound.topRight.y;
					else // INSIDE						
						return 0;
				}
			}
		}
	};

public:
	mutable cweeList<vec2d> coords;
	bool geographic; 
	vec2d topRight;
	vec2d bottomLeft;
	
	cweeBoundary() : coords(), geographic(true), topRight(-cweeMath::INF, -cweeMath::INF), bottomLeft(cweeMath::INF, cweeMath::INF) {};
	cweeBoundary(cweeBoundary const& a) : coords(a.coords), geographic(a.geographic), topRight(a.topRight), bottomLeft(a.bottomLeft) {  };
	cweeBoundary(cweeBoundary&& a) : coords(a.coords), geographic(a.geographic), topRight(a.topRight), bottomLeft(a.bottomLeft) {  };
	cweeBoundary& operator=(cweeBoundary const&) = default;
	cweeBoundary& operator=(cweeBoundary&&) = default;

	vec2d& operator[](int i) {
		switch (i) {
		case 0: return topRight;
		case 1: return bottomLeft;
		default: throw(std::runtime_error("Bad Index for Boundary"));
		}
	};
	const vec2d& operator[](int i) const {
		switch (i) {
		case 0: return topRight;
		case 1: return bottomLeft;
		default: throw(std::runtime_error("Bad Index for Boundary"));
		}
	};
	cweeList<vec2d>& GetCoords() const {
		if (coords.Num() == 0) {
			coords.Append(topRight);
			coords.Append(vec2d(topRight.x, bottomLeft.y));
			coords.Append(bottomLeft);
			coords.Append(vec2d(bottomLeft.x, topRight.y));
			coords.Append(topRight);
		}
		return coords;
	};
	vec2d Center() const { return vec2d((topRight.x + bottomLeft.x) / 2.0, (topRight.y + bottomLeft.y) / 2.0); };
	bool IsPoint() const { return topRight == bottomLeft; };
	static bool Contains(cweeBoundary const& DoesThis, vec2d const& ContainThis) {
		return (DoesThis.topRight >= ContainThis) && (DoesThis.bottomLeft <= ContainThis);
	};
	static bool Contains(cweeBoundary const& DoesThis, cweeBoundary const& ContainThis) {
		return (DoesThis.topRight >= ContainThis.topRight) && (DoesThis.bottomLeft <= ContainThis.bottomLeft);
	};
	static bool Overlaps(cweeBoundary const& a, cweeBoundary const& b) {
		if (a.Contains(b) || b.Contains(a)) return true;
		else if (a.Contains(b.topRight) || b.Contains(a.topRight)) return true;
		else if (a.Contains(b.bottomLeft) || b.Contains(a.bottomLeft)) return true;
		else return Overlaps(a.GetCoords(), b.GetCoords());		
	};
	static cwee_units::foot_t Distance(cweeBoundary const& a, cweeBoundary const& b) {
		// optimization
		if (a.IsPoint()) {
			if (b.IsPoint()) {
				return Distance(a.bottomLeft, b.bottomLeft, b.geographic && a.geographic);
			}
			else {
				// b is not a point
				return Distance(a.bottomLeft, b, b.geographic && a.geographic);
			}
		}
		else if (b.IsPoint()) {
			// a is not a point
			return Distance(b.bottomLeft, a, b.geographic && a.geographic);
		}
		else {
			if (Overlaps(a, b))
				return cwee_units::foot_t(0);
			else
				return Distance(a.GetCoords(), b.GetCoords(), a.geographic && b.geographic);
		}
	};

	bool Contains(vec2d const& a) const {
		return Contains(*this, a);
	};
	bool Contains(cweeBoundary const& a) const {
		return Contains(*this, a);
	};
	bool Overlaps(cweeBoundary const& a) const {
		return Overlaps(*this, a);
	};
	cwee_units::foot_t Distance(cweeBoundary const& a) const {
		return Distance(*this, a);
	};
};

/* Data structure meant to enable high-performance spatial analysis of 2D objects (E.g. nearest neighbor) by creating and caching a large, fairly distributed tree structure. */
template <
	class objType, 
	cweeBoundary const&(*coordinateLookupFunctor)(objType const&),
	cwee_units::foot_t(*DistanceFunction)(objType const&, cweeBoundary const&),
	cwee_units::foot_t(*ObjectDistanceFunction)(objType const&, objType const&) = [](objType const& a, objType const& b)->cwee_units::foot_t { return DistanceFunction(a, coordinateLookupFunctor(b)); }
>
class RTree {
public:
	static cweeBoundary const& GetBoundary(objType const& obj) { return coordinateLookupFunctor(obj); };
	class TreeNode {
	public:
		cweeList<cweeSharedPtr<objType>>
			unhandledObjs;
		cweeList< TreeNode* >
			children;
		cweeBoundary
			bound;
		cweeSharedPtr<objType> 
			object;
		TreeNode*
			parent;
		int 
			parentsChildIndex;
		double
			cached_distance;
		cweeList< TreeNode* >
			cached_children;
		int
			cached_parentsChildIndex;


		TreeNode() : unhandledObjs(), children(), bound(), object(nullptr), parent(nullptr), parentsChildIndex(-1), cached_distance(){};
		TreeNode(TreeNode const&) = default;
		TreeNode(TreeNode&&) = default;
		TreeNode& operator=(TreeNode const&) = default;
		TreeNode& operator=(TreeNode&&) = default;

		TreeNode* Next_Cached() {
			TreeNode* out{ nullptr };
			if (parent && (cached_parentsChildIndex >= 0)) {
				if (parent->cached_children.Num() > (cached_parentsChildIndex + 1)) {
					out = parent->cached_children[cached_parentsChildIndex + 1];
				}
			}
			return out;
		};
		TreeNode* Next() {
			TreeNode* out{ nullptr };
			if (parent && (parentsChildIndex >= 0)) {
				if (parent->children.Num() > (parentsChildIndex + 1)) {
					out = parent->children[parentsChildIndex + 1];
				}
			}
			return out;
		};
		TreeNode* Prev() {
			TreeNode* out{ nullptr };
			if (parent && (parentsChildIndex >= 1)) {
				if ((parentsChildIndex - 1) >= 0) {
					out = parent->children[parentsChildIndex - 1];
				}
			}
			return out;
		};

		TreeNode* AddChild(TreeNode* ptr) {
			if (ptr) {
				children.Append(ptr);

				for (int i = 0; i < 2; i++) {
					if (this->bound.topRight[i] < ptr->bound.topRight[i]) {
						this->bound.topRight[i] = ptr->bound.topRight[i];
					}
					if (this->bound.bottomLeft[i] > ptr->bound.bottomLeft[i]) {
						this->bound.bottomLeft[i] = ptr->bound.bottomLeft[i];
					}
				}
			}
			return ptr;
		};
	};
	cweeAlloc<TreeNode, 10> 
		nodeAllocator;
	cweeList<cweeSharedPtr<objType>>
		objects;
	TreeNode*
		root;

public:
	RTree() : nodeAllocator(), objects(), root(nullptr)  {};
	RTree(RTree const& obj) : nodeAllocator(), objects(obj.objects), root(nullptr) {};
	RTree& operator=(const RTree& obj) { nodeAllocator.Clear(); root = nullptr; objects = obj.objects; return *this; };
	RTree& operator=(RTree&& obj) { nodeAllocator.Clear(); root = nullptr; objects = obj.objects; return *this; };
	~RTree() { nodeAllocator.Clear(); root = nullptr; };
	
	/* Direct (100% accurate) approach to clustering. Very fast, but becomes a serious memory-performance problem with large datasets (>1000 objects). */
	static cweeList< cweeList<cweeSharedPtr<objType> > > kmeans_cluster(int k, cweeList<vec2d> const& data, cweeList<cweeSharedPtr<objType>> const& objs, cweeList<vec2d>& new_centers) {
		cweeList< cweeList<cweeSharedPtr<objType>> > out;
		out.SetNum(k);

		int m = data.size(), n = 2, i, j, l, label;
		double min_dist, dist;
		bool converged;

		auto toReturn = make_cwee_shared<cweeList<vec2d>>();
		cweeList<vec2d>& centers = *toReturn;
		centers.SetNum(k);

		std::vector<int> labels(m, -1);
		new_centers.SetNum(k); // if it has enough room already, it just reduces the visible amount.
		std::vector<int> counts(k);

		for (i = 0; i < k; ++i) centers[i] = data[i];
		while (true) {
			for (i = 0; i < k; ++i) new_centers[i].Zero();
			for (i = 0; i < k; ++i) counts[i] = 0;

			{
				int j_parallel, i_parallel; double min_dist_parallel, dist_parallel;
				for (i_parallel = 0; i_parallel < m; ++i_parallel) {
					min_dist_parallel = std::numeric_limits<double>::max();
					for (j_parallel = 0; j_parallel < k; j_parallel++) {
						dist_parallel = (data[i_parallel] - centers[j_parallel]).LengthSqr();

						if (dist_parallel < min_dist_parallel) {
							min_dist_parallel = dist_parallel;
							labels[i_parallel] = j_parallel;
						}
					}
				};
			}

			// accumulate in-line
			{
				int l_parallel; int label_parallel;
				for (i = 0; i < m; ++i) {
					counts[labels[i]]++;
					label_parallel = labels[i];
					for (l_parallel = 0; l_parallel < n; ++l_parallel)
						new_centers[label_parallel][l_parallel] += data[i][l_parallel];
				}
			}

			converged = true;
			for (i = 0; i < k; ++i) {
				if (counts[i] == 0) {
					continue;
				}
				for (l = 0; l < n; ++l) {
					new_centers[i][l] /= counts[i];
					if (new_centers[i][l] != centers[i][l]) {
						converged = false;
					}
					centers[i][l] = new_centers[i][l];
				}
			}
			if (converged) {
				break;
			}
		}

		for (i = 0; i < k; ++i) 
			out[i].SetGranularity(counts[i] + 1);
		for (i = 0; i < m; ++i) 
			out[labels[i]].Append(objs[i]);

		return out;
	};
	/* Approximate approach to clustering. Very fast for large datasets. Repeatedly analyzes the square root of the number of objects, and should only be applied to larger datasets (>150). */
	static cweeList< cweeList<cweeSharedPtr<objType> > > kmeans_cluster_fast(int numClusters, cweeList<vec2d> const& coord_data, cweeList<cweeSharedPtr<objType>> const& objs) {
		cweeList< cweeList<cweeSharedPtr<objType>> > out;
		out.SetGranularity(numClusters + 1);

		auto newCenters = KMeans().GetClusters(coord_data, numClusters);
		if (newCenters->Num() <= 1) { for (auto& x : objs) { if (x) out.Alloc().Append(x); } }
		else {
			try {
				// Voronoi algorithm may throw an error if it results in a single cell, any of the input centers are identical, or if the voronoi (rarely) is invalid. 
				// Invalid voronoi can happen if 3 or more "centers" form a perfect straight line. This is prevented with small jittering by the algorithm, but the jitter is not perfect and can (RARELY) fail to protect you. 
				auto cells = Voronoi(newCenters).GetCells();
				newCenters = nullptr;

				int i;
				cweeList<int> obj_indexes;
				obj_indexes.SetNum(objs.Num());
				for (i = obj_indexes.Num() - 1; i >= 0; --i) obj_indexes[i] = i;

				for (auto& cell : cells) {
					cweeList<cweeSharedPtr<objType>>* cellChildren = &out.Alloc();
					cellChildren->SetGranularity(1 + ((obj_indexes.Num()) / (cells.Num() + 1)) * 2);
					for (i = obj_indexes.Num() - 1; i >= 0; i--) {
						auto& obj = objs[obj_indexes[i]];
						if (obj) {
							if (cell.overlaps(coordinateLookupFunctor(*obj).Center())) {
								cellChildren->Append(obj);
								obj_indexes.RemoveIndexFast(i);
							}
						}
					}
					if (cellChildren->Num() <= 0) out.RemoveIndexFast(out.Num() - 1);
				}
			}
			catch (std::runtime_error) {
				out.Clear();
				if (objs.Num() < numClusters) for (auto& x : objs) if (x) out.Alloc().Append(x);
			}
		}

		return out;
	};
	/* Clusters a list of objects into X clusters, using a combination of approaches (direct for small datasets, or, approximate & voronoi for large datasets). */
	static cweeList< cweeList<cweeSharedPtr<objType>> > Cluster(int numClusters, cweeList<cweeSharedPtr<objType>> const& objs, cweeList<vec2d>& centers_cache) {
		if (objs.Num() == 0) throw(std::runtime_error("Cannot cluster zero objects in RTree.")); // if this happens, something went wrong earlier in the algorithm.

		numClusters = cweeMath::max(2, numClusters);
		// while ((objs.Num() / numClusters) > 5000) { numClusters += 1; } 

		if (objs.Num() <= numClusters) { // less objects than desired clusters -- return objects as-is, w/o clustering.
			cweeList< cweeList<cweeSharedPtr<objType>> > out;
			out.SetGranularity(numClusters + 1);
			for (auto& x : objs) { if (x) out.Alloc().Append(x); } 
			return out;
		} 
		else { // do clustering
			cweeList<vec2d> coord_data;
			for (auto& x : objs) if (x) coord_data.Append(GetBoundary(*x).Center());
			if (objs.Num() <= 150) {
				// explores 100% of datapoints, and assigns them to the center(s) automatically. 
				return kmeans_cluster(numClusters, coord_data, objs, centers_cache);
			}
			else {
				return kmeans_cluster_fast(numClusters, coord_data, objs);
			}
		}		
	};
	/* Recreates and caches the tree structure. Tree structure is invalidated if any objects are added or removed. Called automatically when traversing the tree. */
	void CreateTree(TreeNode* node, cweeList<cweeSharedPtr<objType>> const& objs) {
		TreeNode* newNode; int index;
		int targetClusterCount = 10;

		/* Reload the tree at the root (the provided 'node') */
		node->unhandledObjs = objs;
		node->parent = nullptr;
		cweeList<vec2d> new_centers_cache;

		while (node) {
			if (node->unhandledObjs.Num() > 0) {
				if (node->unhandledObjs.Num() == 1) {
					node->object = node->unhandledObjs[0];
					node->unhandledObjs.Clear();
					if (node->object) { node->bound = GetBoundary(*node->object); }
				}
				else {
					AUTO clusters = Cluster(targetClusterCount, node->unhandledObjs, new_centers_cache);

					if (clusters.Num() > 0) {
						node->unhandledObjs.Clear();
						for (auto& cluster : clusters) {
							index = node->children.Num();
							newNode = nodeAllocator.Alloc();
							newNode->parentsChildIndex = node->children.Num();
							newNode->cached_parentsChildIndex = newNode->parentsChildIndex;
							newNode->unhandledObjs = cluster;
							newNode->parent = node;
							node->children.Append(newNode);
						}
						node->cached_children = node->children;
						node = node->children[0];
					}
					else {
						for (auto& obj : node->unhandledObjs) {
							index = node->children.Num();
							newNode = nodeAllocator.Alloc();
							newNode->parentsChildIndex = node->children.Num();
							newNode->cached_parentsChildIndex = newNode->parentsChildIndex;
							newNode->unhandledObjs.Append(obj);
							newNode->parent = node;
							node->children.Append(newNode);
						}
						node->cached_children = node->children;
						node->unhandledObjs.Clear();
						node = node->children[0];
					}
				}
			}
			else {
				if (node->Next()) {
					node = node->Next();
				}
				else {
					node = node->parent; // finished all children for this node
					if (node) {
						for (auto* ptr : node->children) {
							for (int i = 0; i < 2; i++) {
								if (node->bound.topRight[i] < ptr->bound.topRight[i]) {
									node->bound.topRight[i] = ptr->bound.topRight[i];
								}
								if (node->bound.bottomLeft[i] > ptr->bound.bottomLeft[i]) {
									node->bound.bottomLeft[i] = ptr->bound.bottomLeft[i];
								}
							}
						}
					}
				}
			}
		}
	
		/* After completion, clear the cache at all nodes */
		//node = root;
		//while (node) {
		//	node->cached_parentsChildIndex = -1;
		//	node->cached_children.Clear();
		//	node->cached_distance = 0;
		//	node = GetNext(node);
		//}
	};
	/* Recreates and caches the tree structure. Tree structure is invalidated if any objects are added or removed. Called automatically when traversing the tree. */
	void ReloadTree() {
		nodeAllocator.Clear();
		root = nodeAllocator.Alloc();
		CreateTree(root, objects);
	};
	/* Add an object to the RTree */
	void Add(cweeSharedPtr<objType> const& obj) {
		objects.Append(obj);
		root = nullptr;
	};
	/* Remove an object from the RTree */
	void Remove(cweeSharedPtr<objType> const& obj) {
		objects.Remove(obj);
		root = nullptr; 
	};
	/* goes through all nodes of the tree */
	TreeNode* GetRoot() {
		if (!root) ReloadTree();
		return root;
	};
	/* Get the next node (intermediate or leaf w/object) */
	static TreeNode* GetNext(TreeNode* node) {
		if (node) {
			if (node->children.Num() > 0) {
				node = node->children[0];
			}
			else {
				while (node && node->Next() == nullptr) {
					node = node->parent;
				}
				if (node) node = node->Next();
			}
		}
		return node;
	};
	/* Get the next node (leaf w/object only) */
	static TreeNode* GetNextLeaf(TreeNode* node) {
		node = GetNext(node);
		while (node && !node->object) {
			node = GetNext(node);
		}
		return node;
	};
	/* Try to find a leaf node that satisfies the search criterion */
	TreeNode* TryFindNode(std::function<bool(objType const&)> search) {
		TreeNode* child = GetRoot();
		child = GetNextLeaf(child);
		while (child) {
			if (child->object && search(*child->object)) {
				return child;
			}
			child = GetNextLeaf(child);
		}
		return nullptr;
	};
	/* Try to find an object that satisfies the search criterion */
	cweeSharedPtr<objType> TryFindObject(std::function<bool(objType const&)> search) {
		TreeNode* child = TryFindNode(search);
		if (child) return child->object;
		return nullptr;
	};
	/* Try to find the deepest node (leaf or intermediate) that overlaps with the input boundary */
	TreeNode* DeepestOverlappingNode(cweeBoundary const& point) {
		TreeNode* search = GetRoot();
		TreeNode* out = search;
		while (search) {
			if (search->bound.Contains(point)) {
				bool continueSearch = false;
				for (AUTO child : search->children) {
					if (child && child->bound.Contains(point)) {
						search = child;
						out = search;
						continueSearch = true;
						break;
					}
				}
				if (continueSearch) continue;
				if (search->object) {
					// this node must be a perfect overlap 
					out = search;
					break;
				}
				else {
					// none of the children of this node actually overlapped with our point of interest -- that can happen.
					out = search;
					break;
				}
			}
			else {
				search = search->parent;
			}
		}
		return out;
	};	
	/* Main benefit / point of the RTree structure. Quickly finds the X nearest objects to a point in space. */
	static cweeBalancedCurve< TreeNode* > Near(TreeNode* node, cweeBoundary const& point, int numNear) {
		cweeBalancedCurve< TreeNode* > sortedNodesActual;
		auto& sortedNodes = sortedNodesActual.UnsafeGetValues();

		int manualCount = 0;
		double distance;
		double distance_threshold = std::numeric_limits<double>::max();
		int i;
		
		if (node) node->cached_distance = point.Distance(node->bound)(); // initiate the cached distance
		while (node) {
			if (node->object) {
				distance = DistanceFunction(*node->object, point)();

				sortedNodes.Add(node, distance, false);

				manualCount++;
				if (manualCount >= numNear && distance < distance_threshold) {
					auto* p = sortedNodes.NodeFindByIndex(numNear - 1);
					if (p) {
						distance_threshold = p->key;
					}
				}
			}
			else {
				if (manualCount < numNear || node->cached_distance < distance_threshold) {
					for (i = node->cached_children.Num() - 1; i >= 0; --i) node->cached_children[i]->cached_distance = point.Distance(node->cached_children[i]->bound)();
					node->cached_children.Sort([&point](TreeNode* const& a, TreeNode* const& b)->bool {
						return a->cached_distance < b->cached_distance;
					});
					for (i = node->cached_children.Num() - 1; i >= 0; --i) node->cached_children[i]->cached_parentsChildIndex = i;					
				} else {
					// skip this entire node, including it's children. 					
					while (node && node->Next_Cached() == nullptr) {
						node = node->parent;
					}
					if (node) node = node->Next_Cached();
					continue;
				}
			}
			if (node) {
				if (node->cached_children.Num() > 0) {
					node = node->cached_children[0];
				}
				else {
					while (node && node->Next_Cached() == nullptr) {
						node = node->parent;
					}
					if (node) node = node->Next_Cached();
				}
			}
		}
		
		return sortedNodesActual;
	};
	/* Finds the X nearest leaf nodes to the input boundary. Overlaps are included as a perfect 0 distance. */
	cweeList<TreeNode*> Near(cweeBoundary const& point, int numNear = 1) {
		AUTO sortedNodes = Near(GetRoot(), point, numNear);
		cweeList<TreeNode*> out; out.SetGranularity(numNear + 1);
		for (auto& x : sortedNodes.GetValueKnotSeries()) {
			if (out.Num() < numNear) {
				out.Append(x);
			}
			else {
				break;
			}
		}
		return out;
	};
	/* Finds the X nearest leaf nodes to the input object. Overlaps are included as a perfect 0 distance. Note that this list will likely include the search object itself. */
	cweeList<TreeNode*> Near(objType const& point, int numNear = 1) {
		return Near(coordinateLookupFunctor(point), numNear);
	};
};

#endif