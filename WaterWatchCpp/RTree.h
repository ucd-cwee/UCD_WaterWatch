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

class cweeBoundary {
public:
	static cwee_units::foot_t Distance(vec2d const& LongLat1, vec2d const& LongLat2, bool geographic = true) {
		double  lat_old, lat_new, lat_diff, lng_diff, a, c;
		units::length::meter_t out;

		if (!geographic) {
			out = units::length::foot_t(LongLat1.Distance(LongLat2));
		} else {
			if (LongLat1.x >= -180.0 && LongLat1.x <= 180.0 && LongLat1.y >= -90.0 && LongLat1.y <= 90.0 &&
				LongLat2.x >= -180.0 && LongLat2.x <= 180.0 && LongLat2.y >= -90.0 && LongLat2.y <= 90.0) {
				lat_old = LongLat1.y * cweeMath::PI / 180.0;
				lat_new = LongLat2.y * cweeMath::PI / 180.0;
				lat_diff = (LongLat2.y - LongLat1.y) * cweeMath::PI / 180.0;
				lng_diff = (LongLat2.x - LongLat1.x) * cweeMath::PI / 180.0;

				a = std::sin(lat_diff / 2.0) * std::sin(lat_diff / 2.0) + std::cos(lat_new) * std::cos(lat_old) * std::sin(lng_diff / 2.0) * std::sin(lng_diff / 2.0);
				c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

				out = units::length::meter_t(6372797.56085 * c);
			}
			else {
				out = units::length::foot_t(LongLat1.Distance(LongLat2));
			}
		}
		return out;
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
		if (lineCoords.Num() == 0) {
			out = std::numeric_limits<decltype(out)>::max();
		}
		else if (lineCoords.Num() == 1) {
			out = Distance(pointCoord, lineCoords[0], geographic);
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
			cwee_units::foot_t dist_end = Distance(pointCoord, lineCoords[0], geographic);
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

		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();

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
				out = cwee_units::math::fmin(out, Distance(coords1[i], coords2[j], geographic));
			}
		}

		// nearest perpendicular points for end-points
		//		.....*......
		//			  
		//			 *				
		//			.
		//		   .
		//        .
		for (int i = 0; i < coords1.Num(); i++) out = cwee_units::math::fmin(out, Distance(coords1[i], coords2, geographic));
		for (int i = 0; i < coords2.Num(); i++) out = cwee_units::math::fmin(out, Distance(coords2[i], coords1, geographic));

		return out;
	};

public:
	bool geographic; 
	vec2d topRight;
	vec2d bottomLeft;
	
	cweeBoundary() : geographic(true), topRight(-cweeMath::INF, -cweeMath::INF), bottomLeft(cweeMath::INF, cweeMath::INF) {};
	cweeBoundary(cweeBoundary const& a) : geographic(a.geographic), topRight(a.topRight), bottomLeft(a.bottomLeft) {  };
	cweeBoundary(cweeBoundary&& a) : geographic(a.geographic), topRight(a.topRight), bottomLeft(a.bottomLeft) {  };
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

	vec2d Center() { return vec2d((topRight.x + bottomLeft.x) / 2.0, (topRight.y + bottomLeft.y) / 2.0); };

	static bool Contains(cweeBoundary const& DoesThis, vec2d const& ContainThis) {
		return (DoesThis.topRight >= ContainThis) && (DoesThis.bottomLeft <= ContainThis);
	};
	static bool Contains(cweeBoundary const& DoesThis, cweeBoundary const& ContainThis) {
		return (DoesThis.topRight >= ContainThis.topRight) && (DoesThis.bottomLeft <= ContainThis.bottomLeft);
	};
	static bool Overlaps(cweeBoundary const& a, cweeBoundary const& b) {
		cweeList<vec2d> a_list, b_list;
		
		a_list.Append(a.topRight);
		a_list.Append(vec2d(a.topRight.x, a.bottomLeft.y));
		a_list.Append(a.bottomLeft);
		a_list.Append(vec2d(a.bottomLeft.x, a.topRight.y));
		a_list.Append(a.topRight);
		b_list.Append(b.topRight);
		b_list.Append(vec2d(b.topRight.x, b.bottomLeft.y));
		b_list.Append(b.bottomLeft);
		b_list.Append(vec2d(b.bottomLeft.x, b.topRight.y));
		b_list.Append(b.topRight);

		if (a.Contains(b) || b.Contains(a)) return true;
		else return Overlaps(a_list, b_list);
	};
	static cwee_units::foot_t Distance(cweeBoundary const& a, cweeBoundary const& b) {
		cweeList<vec2d> a_list, b_list;
		{			
			a_list.Append(a.topRight);
			a_list.Append(vec2d(a.topRight.x, a.bottomLeft.y));
			a_list.Append(a.bottomLeft);
			a_list.Append(vec2d(a.bottomLeft.x, a.topRight.y));
			a_list.Append(a.topRight);					
			b_list.Append(b.topRight);
			b_list.Append(vec2d(b.topRight.x, b.bottomLeft.y));
			b_list.Append(b.bottomLeft);
			b_list.Append(vec2d(b.bottomLeft.x, b.topRight.y));
			b_list.Append(b.topRight);

			if (a.Contains(b) || b.Contains(a)) return cwee_units::foot_t(0);
			else if (Overlaps(a_list, b_list)) return cwee_units::foot_t(0);
			else return Distance(a_list, b_list, a.geographic && b.geographic);
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

template <
	class objType, 
	cweeBoundary(*coordinateLookupFunctor)(objType const&), 
	cwee_units::foot_t(*DistanceFunction)(objType const&, cweeBoundary const&),
	cwee_units::foot_t(*ObjectDistanceFunction)(objType const&, objType const&) = [](objType const& a, objType const& b)->cwee_units::foot_t { return DistanceFunction(a, coordinateLookupFunctor(b)); }
>
class RTree {
public:
	static cweeBoundary GetBoundary(objType const& obj) {
		cweeBoundary b = coordinateLookupFunctor(obj);
		return b;
	};
	class TreeNode {
	public:
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

		TreeNode() : children(), bound(), object(nullptr), parent(nullptr), parentsChildIndex(-1) {};
		TreeNode(TreeNode const&) = default;
		TreeNode(TreeNode&&) = default;
		TreeNode& operator=(TreeNode const&) = default;
		TreeNode& operator=(TreeNode&&) = default;

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
	RTree() : nodeAllocator(), objects(), root(nullptr)  { ReloadTree(); };
	RTree(RTree const& obj) : nodeAllocator(), objects(obj.objects), root(nullptr) { ReloadTree(); };
	RTree& operator=(const RTree& obj) { objects = obj.objects; ReloadTree(); return *this; };
	RTree& operator=(RTree&& obj) { objects = obj.objects; ReloadTree(); return *this; };
	~RTree() { nodeAllocator.Clear(); };
	
	static cweeSharedPtr<cweeList<vec2d>> kmeans(int k, std::vector<vec2d> const& data) {
		using namespace concurrency;

		int m = data.size(), n = 2, i, j, l, label;
		double min_dist, dist;
		bool converged;

		auto toReturn = make_cwee_shared<cweeList<vec2d>>();

		cweeList<vec2d>& centers = *toReturn;
		centers.SetNum(k);

		std::vector<int> labels(m, -1);
		std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
		std::vector<cweeSysInterlockedInteger> counts(k);

		for (i = 0; i < k; ++i) centers[i] = data[i];
		while (true) {
			for (i = 0; i < k; ++i) for (j = 0; j < n; ++j) new_centers[i][j] = 0;
			for (i = 0; i < k; ++i) counts[i].SetValue(0);

			std::vector<int> j_parallel(m, 0);
			std::vector<double> min_dist_parallel(m, std::numeric_limits<double>::max());
			std::vector<double> dist_parallel(m, 0);

			// parallelize the triple loop
			parallel_for(int(0), m, [&](int i_parallel) {				
				for (; j_parallel[i_parallel] < k; j_parallel[i_parallel]++) {
					dist_parallel[i_parallel] = (data[i_parallel] - centers[j_parallel[i_parallel]]).LengthSqr();
					//dist_parallel[i_parallel] = 
					//	(data[i_parallel].x - centers[j_parallel[i_parallel]].x) * (data[i_parallel].x - centers[j_parallel[i_parallel]].x) +
					//	(data[i_parallel].y - centers[j_parallel[i_parallel]].y) * (data[i_parallel].y - centers[j_parallel[i_parallel]].y);

					if (dist_parallel[i_parallel] < min_dist_parallel[i_parallel]) {
						min_dist_parallel[i_parallel] = dist_parallel[i_parallel];
						labels[i_parallel] = j_parallel[i_parallel];
					}
				}
				counts[labels[i_parallel]].Increment();
			});

			// accumulate in-line
			{
				int l_parallel; int label_parallel;
				for (i = 0; i < m; ++i) {
					label_parallel = labels[i];
					for (l_parallel = 0; l_parallel < n; ++l_parallel)
						new_centers[label_parallel][l_parallel] += data[i][l_parallel];
				}
			}

			//for (i = 0; i < m; ++i) {
			//	min_dist = std::numeric_limits<double>::max();
			//	label = -1;
			//	for (j = 0; j < k; ++j) {
			//		dist = 0;
			//		for (l = 0; l < n; ++l) {
			//			dist += (data[i][l] - centers[j][l]) * (data[i][l] - centers[j][l]);
			//		}
			//		if (dist < min_dist) {
			//			min_dist = dist;
			//			label = j;
			//		}
			//	}
			//	labels[i] = label;
			//	counts[label]++;
			//	for (l = 0; l < n; ++l) {
			//		new_centers[label][l] += data[i][l];
			//	}
			//}

			converged = true;
			for (i = 0; i < k; ++i) {
				if (counts[i].GetValue() == 0) {
					continue;
				}
				for (l = 0; l < n; ++l) {
					new_centers[i][l] /= counts[i].GetValue();
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

		return toReturn;
	};
	static cweeList< cweeList<cweeSharedPtr<objType>> > Cluster(int numClusters, cweeList<cweeSharedPtr<objType>> const& objs) {
		if (objs.Num() == 0) throw(std::runtime_error("Cannot cluster zero objects in RTree."));

		numClusters = cweeMath::max(2, numClusters);
		while ((objs.Num() / numClusters) > 5000) { numClusters += 1; }

		cweeList< cweeList<cweeSharedPtr<objType>> > out;
		if (objs.Num() < numClusters) { for (auto& x : objs) { if (x) out.Alloc().Append(x); } } // less objects than desired clusters -- return objects as-is, w/o clustering
		else {
			cweeList<vec2d> coord_data;
			for (auto& x : objs) if (x) coord_data.Append(GetBoundary(*x).Center());						
			auto newCenters = kmeans(numClusters, coord_data);
			coord_data.Clear();
			if (newCenters->Num() <= 1) { for (auto& x : objs) { if (x) out.Alloc().Append(x); } }
			else {
				try {
					// NOTE TO SELF: THROWS HERE IF THE VORONOI DIAGRAM HAS A SINGLE CELL AND / OR ALL OF THE "CENTERS" ARE IDENTICAL
					auto voronoi{ Voronoi(newCenters) };
					newCenters = nullptr;
					auto cells = voronoi.GetCells(); // straight data copy
					voronoi.Clear(); // safe to clear
					int cellN = 0;

					cweeList<cweeSharedPtr<objType>> temp_objs = objs;
					for (auto& cell : cells) {
						cweeList<cweeSharedPtr<objType>> cellChildren;
						for (int i = temp_objs.Num() - 1; i >= 0; i--) {
							if (temp_objs[i]) {
								auto b = coordinateLookupFunctor(*temp_objs[i]);
								if (cell.overlaps(b.Center())) {
									cellChildren.Append(temp_objs[i]);
									temp_objs.RemoveIndexFast(i);
								}
							}
						}
						if (cellChildren.Num() > 0) {
							out.Append(cellChildren);
						}
						cell.Clear();
					}
				} 
				catch (std::runtime_error) {
					out.Clear();
					if (objs.Num() < numClusters) for (auto& x : objs) if (x) out.Alloc().Append(x);
				}

			}
		}
		return out;
	};

	TreeNode* CreateNode(TreeNode* parent, TreeNode* node, cweeList<cweeSharedPtr<objType>> const& objs) {
		int index;
		if (node && objs.Num() > 0) {
			if (objs.Num() == 1) {
				node->object = objs[0];
				if (node->object) { node->bound = GetBoundary(*node->object); }
				node->parent = parent;
				if (parent) node->parentsChildIndex = parent->children.Num();				
				else node->parentsChildIndex = -1;				
			} 
			else {
				node->parent = parent;
				for (auto& cluster : Cluster(10, objs)) {
					index = node->children.Num();
					node->AddChild(CreateNode(node, nodeAllocator.Alloc(), cluster))->parentsChildIndex = index;
					cluster.Clear();
				}
			}
		}
		return node;
	};
	AUTO ReloadTree() {
		nodeAllocator.Clear();
		root = nodeAllocator.Alloc();
		return CreateNode(nullptr, root, objects);
	};

	void Add(cweeSharedPtr<objType> const& obj) {
		objects.Append(obj);
		root = nullptr;
	};
	void Remove(cweeSharedPtr<objType> const& obj) {
		objects.Remove(obj);
		root = nullptr; 
	};

	/* goes through all nodes of the tree */
	TreeNode* GetRoot() {
		if (!root) ReloadTree();
		return root;
	};
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
	static TreeNode* GetNextLeaf(TreeNode* node) {
		node = GetNext(node);
		while (node && !node->object) {
			node = GetNext(node);
		}
		return node;
	};
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
	cweeSharedPtr<objType> TryFindObject(std::function<bool(objType const&)> search) {
		TreeNode* child = TryFindNode(search);
		if (child) return child->object;
		return nullptr;
	};
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
	
	static void Near(cweeBalancedCurve< TreeNode* >& sortedNodes, TreeNode* node, cweeBoundary const& point, int numNear, cwee_units::foot_t thisDistance) {
		if (!node) return;

		if (sortedNodes.GetNumValues() >= numNear) {
			cwee_units::foot_t maxDistance = sortedNodes.UnsafeKnotForIndex(numNear - 1).first;
			if (thisDistance >= maxDistance) {
				return; // no point
			}
		}

		if (node->object) {
			sortedNodes.AddValue(thisDistance(), node);
		}

		// sort the children by distance -- do the closest ones first, which reduces likelihood of doing unecessary work later.
		cweeBalancedCurve< TreeNode* > sortedChildren;
		for (auto& child : node->children) {
			if (child) {
				if (child->object) {
					sortedChildren.AddValue(DistanceFunction(*child->object, point)(), child);
				}
				else {
					sortedChildren.AddValue(point.Distance(child->bound)(), child);
				}				
			}
		}

		for (auto& x : sortedChildren.UnsafeGetValues()) {
			if (x.object) {
				Near(sortedNodes, *x.object, point, numNear, x.key);
			}
		}
	};
	static void Near(cweeBalancedCurve< TreeNode* >& sortedNodes, TreeNode* node, objType const& point, int numNear, cwee_units::foot_t thisDistance) {
		if (!node) return;

		if (sortedNodes.GetNumValues() >= numNear) {
			cwee_units::foot_t maxDistance = sortedNodes.UnsafeKnotForIndex(numNear - 1).first;
			if (thisDistance >= maxDistance) {
				return; // no point
			}
		}

		if (node->object) {
			sortedNodes.AddValue(thisDistance(), node);
		}

		// sort the children by distance -- do the closest ones first, which reduces likelihood of doing unecessary work later.		
		cweeBalancedCurve< TreeNode* > sortedChildren;
		for (auto& child : node->children) {
			if (child) {
				if (child->object) {
					sortedChildren.AddValue(ObjectDistanceFunction(*child->object, point)(), child);
				}
				else {
					sortedChildren.AddValue(child->bound.Distance(GetBoundary(point))(), child);
				}
			}
		}

		for (auto& x : sortedChildren.UnsafeGetValues()) {
			if (x.object) {
				Near(sortedNodes, *x.object, point, numNear, x.key);
			}
		}

		//for (int i = 0; i < sortedChildren.Num(); i++) 
		//	Near(sortedNodes, sortedChildren.knots[i].get<1>(), point, numNear, sortedChildren.knots[i].get<0>());
	};
	cweeList<TreeNode*> Near(cweeBoundary const& point, int numNear = 1) {
		cweeBalancedCurve< TreeNode* > sortedNodes; // self-sorted vector of arbitrary Y values by numeric X values
				
		auto* root = GetRoot();
		if (root) {
			Near(sortedNodes, root, point, numNear, point.Distance(root->bound));
		}

		cweeList<TreeNode*> out;

		for (auto& x : sortedNodes.GetValueKnotSeries()) {
			if (out.Num() < numNear) {
				if (x) {
					out.Append(x);
				}
			}
			else {
				break;
			}
		}

		return out;
	};
	cweeList<TreeNode*> Near(objType const& point, int numNear = 1) {
		cweeBalancedCurve< TreeNode* > sortedNodes; // self-sorted vector of arbitrary Y values by numeric X values

		auto* root = GetRoot();
		if (root) {
			if (root->object) {
				Near(sortedNodes, root, point, numNear, ObjectDistanceFunction(point, *root->object));
			}
			else {
				Near(sortedNodes, root, point, numNear, DistanceFunction(point, root->bound));
			}

			
		}

		cweeList<TreeNode*> out;

		for (auto& x : sortedNodes.GetValueKnotSeries()) {
			if (out.Num() < numNear) {
				if (x) {
					out.Append(x);
				}
			}
			else {
				break;
			}
		}

		return out;
	};
};

#endif