// Copyright (c) 2015-2023 Mathias Westerdahl
// For LICENSE (MIT), USAGE or HISTORY, see bottom of file

#pragma once
#include "Precompiled.h"
#include "vec.h"
#include "cweeJob.h"
#include "SharedPtr.h"
#include "InterpolatedMatrix.h"
#include "Engineering.h"

/*
ABOUT:

    A fast single file 2D voronoi diagram generator

HISTORY:
    0.9     2023-01-22  - Modified the Delauney iterator creation api
    0.8     2022-12-20  - Added fix for missing border edges
                          More robust removal of duplicate graph edges
                          Added iterator for Delauney edges
    0.7     2019-10-25  - Added support for clipping against convex polygons
                        - Added JCV_EDGE_INTERSECT_THRESHOLD for edge intersections
                        - Fixed issue where the bounds calculation wasn’t considering all points
    0.6     2018-10-21  - Removed JCV_CEIL/JCV_FLOOR/JCV_FABS
                        - Optimizations: Fewer indirections, better beach head approximation
    0.5     2018-10-14  - Fixed issue where the graph edge had the wrong edge assigned (issue #28)
                        - Fixed issue where a point was falsely passing the jcv_is_valid() test (issue #22)
                        - Fixed jcv_diagram_get_edges() so it now returns _all_ edges (issue #28)
                        - Added jcv_diagram_get_next_edge() to skip zero length edges (issue #10)
                        - Added defines JCV_CEIL/JCV_FLOOR/DBL_MAX for easier configuration
    0.4     2017-06-03  - Increased the max number of events that are preallocated
    0.3     2017-04-16  - Added clipping box as input argument (Automatically calculated if needed)
                        - Input points are pruned based on bounding box
    0.2     2016-12-30  - Fixed issue of edges not being closed properly
                        - Fixed issue when having many events
                        - Fixed edge sorting
                        - Code cleanup
    0.1                 Initial version

LICENSE:

    The MIT License (MIT)

    Copyright (c) 2015-2019 Mathias Westerdahl

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.


DISCLAIMER:

    This software is supplied "AS IS" without any warranties and support

USAGE:

    The input points are pruned if

        * There are duplicates points
        * The input points are outside of the bounding box (i.e. fail the clipping test function)
        * The input points are rejected by the clipper's test function

    The input bounding box is optional (calculated automatically)

    The input domain is (-FLT_MAX, FLT_MAX] (for floats)

    The api consists of these functions:

    void jcv_diagram_generate( int num_points, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, jcv_diagram* diagram );
    void jcv_diagram_generate_useralloc( int num_points, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, const jcv_clipper* clipper, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* diagram );
    void jcv_diagram_free( jcv_diagram* diagram );

    const jcv_site* jcv_diagram_get_sites( const jcv_diagram* diagram );
    const jcv_edge* jcv_diagram_get_edges( const jcv_diagram* diagram );
    const jcv_edge* jcv_diagram_get_next_edge( const jcv_edge* edge );

    An example usage:

    #define JC_VORONOI_IMPLEMENTATION
    // If you wish to use doubles
    //#define double double
    //#define JCV_ATAN2 atan2
    //#define DBL_MAX 1.7976931348623157E+308
    #include "jc_voronoi.h"

    void draw_edges(const jcv_diagram* diagram);
    void draw_cells(const jcv_diagram* diagram);

    void generate_and_draw(int numpoints, const jcv_point* points)
    {
        jcv_diagram diagram;
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(count, points, 0, 0, &diagram);

        draw_edges(diagram);
        draw_cells(diagram);

        jcv_diagram_free( &diagram );
    }

    void draw_edges(const jcv_diagram* diagram)
    {
        // If all you need are the edges
        const jcv_edge* edge = jcv_diagram_get_edges( diagram );
        while( edge )
        {
            draw_line(edge->pos[0], edge->pos[1]);
            edge = jcv_diagram_get_next_edge(edge);
        }
    }

    void draw_cells(const jcv_diagram* diagram)
    {
        // If you want to draw triangles, or relax the diagram,
        // you can iterate over the sites and get all edges easily
        const jcv_site* sites = jcv_diagram_get_sites( diagram );
        for( int i = 0; i < diagram->numsites; ++i )
        {
            const jcv_site* site = &sites[i];

            const jcv_graphedge* e = site->edges;
            while( e )
            {
                draw_triangle( site->p, e->pos[0], e->pos[1]);
                e = e->next;
            }
        }
    }

    // Here is a simple example of how to do the relaxations of the cells
    void relax_points(const jcv_diagram* diagram, jcv_point* points)
    {
        const jcv_site* sites = jcv_diagram_get_sites(diagram);
        for( int i = 0; i < diagram->numsites; ++i )
        {
            const jcv_site* site = &sites[i];
            jcv_point sum = site->p;
            int count = 1;

            const jcv_graphedge* edge = site->edges;

            while( edge )
            {
                sum.x += edge->pos[0].x;
                sum.y += edge->pos[0].y;
                ++count;
                edge = edge->next;
            }

            points[site->index].x = sum.x / count;
            points[site->index].y = sum.y / count;
        }
    }

 */
#ifndef JC_VORONOI_H
#define JC_VORONOI_H

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <memory.h>
#include <assert.h>

constexpr int JCV_DIRECTION_LEFT = 0;
constexpr int JCV_DIRECTION_RIGHT = 1;
constexpr double JCV_INVALID_VALUE = (double)-DBL_MAX;
constexpr int JCV_EDGE_LEFT = 1;
constexpr int JCV_EDGE_RIGHT = 2;
constexpr int JCV_EDGE_BOTTOM = 4;
constexpr int JCV_EDGE_TOP = 8;
constexpr double JCV_EPS = DBL_EPSILON;
constexpr double JCV_EDGE_INTERSECT_THRESHOLD = JCV_EPS; //  (double)1.0e-10F;
constexpr int JCV_CORNER_NONE = 0;
constexpr int JCV_CORNER_TOP_LEFT = 1;
constexpr int JCV_CORNER_BOTTOM_LEFT = 2;
constexpr int JCV_CORNER_BOTTOM_RIGHT = 3;
constexpr int JCV_CORNER_TOP_RIGHT = 4;
constexpr double JCV_RAND_BUFFER = 10e-4;

class JCV {
public:
    struct jcv_memoryblock {
        size_t sizefree;
        jcv_memoryblock* next;
        char* memory;
    };
    struct jcv_priorityqueue {
        // Implements a binary heap
        int                         maxnumitems;
        int                         numitems;
        void** items;
    };
    struct jcv_point {
        double x;
        double y;
    };
    struct jcv_rect {
        jcv_point   min;
        jcv_point   max;
    };
    struct jcv_site;
    struct jcv_edge;
    struct jcv_graphedge {
        jcv_graphedge* next;
        jcv_edge* edge;
        jcv_site* neighbor;
        jcv_point               pos[2];
        double                angle;
    };
    struct jcv_site
    {
        jcv_point       p;
        int             index;  // Index into the original list of points
        jcv_graphedge* edges;  // The half edges owned by the cell
    };
    struct jcv_edge { // The coefficients a, b and c are from the general line equation: ax * by + c = 0
        jcv_edge* next;
        jcv_site* sites[2];
        jcv_point           pos[2];
        double            a;
        double            b;
        double            c;
    };
    struct jcv_halfedge {
        jcv_edge* edge;
        jcv_halfedge* left;
        jcv_halfedge* right;
        jcv_point               vertex;
        double                y;
        int                     direction; // 0=left, 1=right
        int                     pqpos;
    };
    struct jcv_delauney_iter {
        const jcv_edge* sentinel;
        const jcv_edge* current;
    };
    struct jcv_delauney_edge {
        const jcv_edge* edge;       // The voronoi edge separating the two sites
        const jcv_site* sites[2];
        jcv_point       pos[2];     // the positions of the two sites
    };    
    struct jcv_clipper;
    class jcv_context_internal;

    /// Tests if a point is inside the final shape
    typedef int (*jcv_clip_test_point_fn)(const jcv_clipper* clipper, const jcv_point p);  
    /** Given an edge, and the clipper, calculates the e->pos[0] and e->pos[1]
    * Returns 0 if not successful
    */
    typedef int (*jcv_clip_edge_fn)(const jcv_clipper* clipper, jcv_edge* e);
    /** Given the clipper, the site and the last edge,
    * closes any gaps in the polygon by adding new edges that follow the bounding shape
    * The internal context is use when allocating new edges.
    */
    typedef void (*jcv_clip_fillgap_fn)(const jcv_clipper* clipper, jcv_context_internal* allocator, jcv_site* s);
    typedef void* (*FJCVAllocFn)(void* userctx, size_t size);
    typedef void (*FJCVFreeFn)(void* userctx, void* p);

    struct jcv_clipper {
        jcv_clip_test_point_fn  test_fn;
        jcv_clip_edge_fn        clip_fn;
        jcv_clip_fillgap_fn     fill_fn;
        jcv_point               min;        // The bounding rect min
        jcv_point               max;        // The bounding rect max
        void* ctx;        // User defined context
    };
    class jcv_context_internal {
    public:
        jcv_context_internal() :
            mem(nullptr),
            edges(nullptr),
            beachline_start(nullptr),
            beachline_end(nullptr),
            last_inserted(nullptr),
            eventqueue(nullptr),
            sites(nullptr),
            bottomsite(nullptr),
            numsites(0),
            currentsite(0),
            _padding(0),
            memblocks(nullptr),
            edgepool(nullptr),
            halfedgepool(nullptr),
            eventmem(nullptr),
            clipper(),
            memctx(nullptr),
            alloc(),
            free(),
            rect()
        {}
        jcv_context_internal(jcv_context_internal const&) = delete;
        jcv_context_internal(jcv_context_internal&&) = delete;
        jcv_context_internal& operator=(jcv_context_internal const&) = delete;
        jcv_context_internal& operator=(jcv_context_internal&&) = delete;

        // jcv_edge
        // jcv_halfedge
        // jcv_graphedge

        void* mem;
        jcv_edge* edges;
        jcv_halfedge* beachline_start;
        jcv_halfedge* beachline_end;
        jcv_halfedge* last_inserted;
        jcv_priorityqueue* eventqueue;

        jcv_site* sites;
        jcv_site* bottomsite;
        int                 numsites;
        int                 currentsite;
        int                 _padding;

        jcv_memoryblock* memblocks;
        jcv_edge* edgepool;
        jcv_halfedge* halfedgepool;
        void** eventmem;
        jcv_clipper         clipper;

        void* memctx; // Given by the user
        FJCVAllocFn         alloc;
        FJCVFreeFn          free;

        jcv_rect            rect;
    }; 
    class jcv_diagram {
    public:
        jcv_diagram() : internal(nullptr), numsites{ 0 }, min{ 0, 0 }, max{ 0, 0 } {};
        jcv_diagram(jcv_diagram const&) = delete;
        jcv_diagram(jcv_diagram&&) = delete;
        jcv_diagram& operator=(jcv_diagram const&) = delete;
        jcv_diagram& operator=(jcv_diagram&&) = delete;

        jcv_context_internal*   internal;
        int                     numsites;
        jcv_point               min;
        jcv_point               max;
    };

    union jcv_cast_align_struct {
        char* charp;
        void** voidpp;
    };

    static double jcv_abs(double v) { return (v < 0) ? -v : v; };
    static bool double_eq(double a, double b) { return jcv_abs(a - b) < JCV_EPS; };
    static double double_to_int(double v) { return (sizeof(double) == 4) ? (double)(int)v : (double)(long long)v; };
    static double jcv_floor(double v) { double i = double_to_int(v); return (v < i) ? i - 1 : i; };
    static double jcv_ceil(double v) { double i = double_to_int(v); return (v > i) ? i + 1 : i; };
    static double jcv_min(double a, double b) { return a < b ? a : b; };
    static double jcv_max(double a, double b) { return a > b ? a : b; };
    static int jcv_point_cmp(const void* p1, const void* p2) {
        const jcv_point* s1 = (const jcv_point*)p1;
        const jcv_point* s2 = (const jcv_point*)p2;
        return (s1->y != s2->y) ? (s1->y < s2->y ? -1 : 1) : (s1->x < s2->x ? -1 : 1);
    };
    static bool jcv_point_less(const jcv_point* pt1, const jcv_point* pt2) { return (pt1->y == pt2->y) ? (pt1->x < pt2->x) : pt1->y < pt2->y; };
    static bool jcv_point_eq(const jcv_point* pt1, const jcv_point* pt2) { return double_eq(pt1->y, pt2->y) && double_eq(pt1->x, pt2->x); };
    static bool jcv_point_on_box_edge(const jcv_point* pt, const jcv_point* min, const jcv_point* max) {
        return pt->x == min->x || pt->y == min->y || pt->x == max->x || pt->y == max->y;
    };
    static int jcv_get_edge_flags(const jcv_point* pt, const jcv_point* min, const jcv_point* max) {
        int flags = 0;
        if (pt->x == min->x)   flags |= JCV_EDGE_LEFT;
        else if (pt->x == max->x)   flags |= JCV_EDGE_RIGHT;
        if (pt->y == min->y)   flags |= JCV_EDGE_BOTTOM;
        else if (pt->y == max->y)   flags |= JCV_EDGE_TOP;
        return flags;
    };
    static int jcv_edge_flags_to_corner(int edge_flags) {
    #define TEST_FLAGS(_FLAGS, _RETVAL) if ( (_FLAGS) == edge_flags ) return _RETVAL
        TEST_FLAGS(JCV_EDGE_TOP | JCV_EDGE_LEFT, JCV_CORNER_TOP_LEFT);
        TEST_FLAGS(JCV_EDGE_TOP | JCV_EDGE_RIGHT, JCV_CORNER_TOP_RIGHT);
        TEST_FLAGS(JCV_EDGE_BOTTOM | JCV_EDGE_LEFT, JCV_CORNER_BOTTOM_LEFT);
        TEST_FLAGS(JCV_EDGE_BOTTOM | JCV_EDGE_RIGHT, JCV_CORNER_BOTTOM_RIGHT);
    #undef TEST_FLAGS
        return 0;
    };
    static bool jcv_is_corner(int corner) { return corner != 0; };
    static int jcv_corner_rotate_90(int corner) {
        corner--;
        corner = (corner + 1) % 4;
        return corner + 1;
    };
    static jcv_point jcv_corner_to_point(int corner, const jcv_point* min, const jcv_point* max) {
        jcv_point p;
        if (corner == JCV_CORNER_TOP_LEFT) { p.x = min->x; p.y = max->y; }
        else if (corner == JCV_CORNER_TOP_RIGHT) { p.x = max->x; p.y = max->y; }
        else if (corner == JCV_CORNER_BOTTOM_LEFT) { p.x = min->x; p.y = min->y; }
        else if (corner == JCV_CORNER_BOTTOM_RIGHT) { p.x = max->x; p.y = min->y; }
        else { p.x = JCV_INVALID_VALUE; p.y = JCV_INVALID_VALUE; }
        return p;
    };
    static double jcv_point_dist_sq(const jcv_point* pt1, const jcv_point* pt2) {
        double diffx = pt1->x - pt2->x;
        double diffy = pt1->y - pt2->y;
        return diffx * diffx + diffy * diffy;
    };
    static double jcv_point_dist(const jcv_point* pt1, const jcv_point* pt2) {
        return (double)(std::sqrt(jcv_point_dist_sq(pt1, pt2)));
    };
    static void jcv_free_internal(jcv_context_internal* internal) {
        void* memctx = internal->memctx;
        FJCVFreeFn freefn = internal->free;
        while (internal->memblocks) {
            jcv_memoryblock* p = internal->memblocks;
            internal->memblocks = internal->memblocks->next;
            freefn(memctx, (void*)p);
        }
        freefn(memctx, internal->mem);
        delete internal;
    };
    static void jcv_diagram_free(jcv_diagram* d) {
        jcv_free_internal(d->internal);
        d->internal = nullptr;
    };
    static const jcv_site* jcv_diagram_get_sites(const jcv_diagram* diagram) {
        return diagram->internal->sites;
    };
    static const jcv_edge* jcv_diagram_get_next_edge(const jcv_edge* edge) {
        const jcv_edge* e = edge->next;
        while (e != 0 && jcv_point_eq(&e->pos[0], &e->pos[1])) {
            e = e->next;
        }
        return e;
    }; 
    static const jcv_edge* jcv_diagram_get_edges(const jcv_diagram* diagram) {
        jcv_edge e;
        e.next = diagram->internal->edges;
        return jcv_diagram_get_next_edge(&e);
    };
    static void jcv_delauney_begin(const jcv_diagram* diagram, jcv_delauney_iter* iter) {
        iter->current = 0;
        iter->sentinel = jcv_diagram_get_edges(diagram);
    };
    static bool jcv_delauney_next(jcv_delauney_iter* iter, jcv_delauney_edge* next) {
        if (iter->sentinel)
        {
            iter->current = iter->sentinel;
            iter->sentinel = 0;
        }
        else {
            // Note: If we use the raw edges, we still get a proper delauney triangulation
            // However, the result looks less relevant to the cells contained within the bounding box
            // E.g. some cells that look isolated from each other, suddenly still are connected,
            // because they share an edge outside of the bounding box
            iter->current = jcv_diagram_get_next_edge(iter->current);
        }

        while (iter->current && (iter->current->sites[0] == nullptr || iter->current->sites[1] == nullptr))
        {
            iter->current = jcv_diagram_get_next_edge(iter->current);
        }

        if (!iter->current)
            return 0;

        next->edge = iter->current;
        next->sites[0] = next->edge->sites[0];
        next->sites[1] = next->edge->sites[1];
        next->pos[0] = next->sites[0]->p;
        next->pos[1] = next->sites[1]->p;
        return 1;
    };
    static void* jcv_align(void* value, size_t alignment) {
        // RG, memory already comes aligned.
        return value;
        // return (void*)(((uintptr_t)value + (alignment - 1)) & ~(alignment - 1));
    };
    template<typename T>
    static T* jcv_alloc(jcv_context_internal* internal) {
        size_t size = sizeof(T);
        while (!internal->memblocks || internal->memblocks->sizefree < (size + sizeof(void*))) {
            size_t blocksize = 16 * 1024;
            jcv_memoryblock* block = (jcv_memoryblock*)internal->alloc(internal->memctx, blocksize);
            size_t offset = sizeof(jcv_memoryblock);
            block->sizefree = blocksize - offset;
            block->next = internal->memblocks;
            block->memory = ((char*)block) + offset;
            internal->memblocks = block;
        }
        void* p_raw = internal->memblocks->memory;
        void* p_aligned = jcv_align(p_raw, sizeof(void*));
        size += (uintptr_t)p_aligned - (uintptr_t)p_raw;
        internal->memblocks->memory += size;
        internal->memblocks->sizefree -= size;
        return static_cast<T*>(p_aligned);
    };
    static jcv_edge* jcv_alloc_edge(jcv_context_internal* internal) {
        return jcv_alloc< jcv_edge>(internal);
    };
    static jcv_halfedge* jcv_alloc_halfedge(jcv_context_internal* internal) {
        if (internal->halfedgepool)
        {
            jcv_halfedge* edge = internal->halfedgepool;
            internal->halfedgepool = internal->halfedgepool->right;
            return edge;
        }
        return jcv_alloc< jcv_halfedge>(internal);
    };
    static jcv_graphedge* jcv_alloc_graphedge(jcv_context_internal* internal) {
        return jcv_alloc<jcv_graphedge>(internal);
    };
    static void* jcv_alloc_fn(void* memctx, size_t size) {
        (void)memctx;
        return Mem_ClearedAlloc(size, memTag_t::TAG_TEMP); // zero's out
    };
    static void jcv_free_fn(void* memctx, void* p) {
        (void)memctx;
        Mem_Free(p);
    };
    static int jcv_is_valid(const jcv_point* p) {
        return (p->x != JCV_INVALID_VALUE || p->y != JCV_INVALID_VALUE) ? 1 : 0;
    };
    static void jcv_edge_create(jcv_edge* e, jcv_site* s1, jcv_site* s2) {
        e->next = 0;
        e->sites[0] = s1;
        e->sites[1] = s2;
        e->pos[0].x = JCV_INVALID_VALUE;
        e->pos[0].y = JCV_INVALID_VALUE;
        e->pos[1].x = JCV_INVALID_VALUE;
        e->pos[1].y = JCV_INVALID_VALUE;

        double dx = s2->p.x - s1->p.x;
        double dy = s2->p.y - s1->p.y;
        int dx_is_larger = (dx * dx) > (dy * dy); // instead of fabs

        // Simplify it, using dx and dy
        e->c = dx * (s1->p.x + dx * (double)0.5) + dy * (s1->p.y + dy * (double)0.5);

        if (dx_is_larger)
        {
            e->a = (double)1;
            e->b = dy / dx;
            e->c /= dx;
        }
        else
        {
            e->a = dx / dy;
            e->b = (double)1;
            e->c /= dy;
        }
    };
    static int jcv_boxshape_test(const jcv_clipper* clipper, const jcv_point p) {
        return p.x >= clipper->min.x && p.x <= clipper->max.x && p.y >= clipper->min.y && p.y <= clipper->max.y;
    };
    static int jcv_boxshape_clip(const jcv_clipper* clipper, jcv_edge* e) {
        double pxmin = clipper->min.x;
        double pxmax = clipper->max.x;
        double pymin = clipper->min.y;
        double pymax = clipper->max.y;

        double x1, y1, x2, y2;
        jcv_point* s1;
        jcv_point* s2;
        if (e->a == (double)1 && e->b >= (double)0)
        {
            s1 = jcv_is_valid(&e->pos[1]) ? &e->pos[1] : 0;
            s2 = jcv_is_valid(&e->pos[0]) ? &e->pos[0] : 0;
        }
        else
        {
            s1 = jcv_is_valid(&e->pos[0]) ? &e->pos[0] : 0;
            s2 = jcv_is_valid(&e->pos[1]) ? &e->pos[1] : 0;
        }

        if (e->a == (double)1) // delta x is larger
        {
            y1 = pymin;
            if (s1 != 0 && s1->y > pymin)
            {
                y1 = s1->y;
            }
            if (y1 > pymax)
            {
                y1 = pymax;
            }
            x1 = e->c - e->b * y1;
            y2 = pymax;
            if (s2 != 0 && s2->y < pymax)
                y2 = s2->y;

            if (y2 < pymin)
            {
                y2 = pymin;
            }
            x2 = (e->c) - (e->b) * y2;
            // Never occurs according to lcov
            // if( ((x1 > pxmax) & (x2 > pxmax)) | ((x1 < pxmin) & (x2 < pxmin)) )
            // {
            //     return 0;
            // }
            if (x1 > pxmax)
            {
                x1 = pxmax;
                y1 = (e->c - x1) / e->b;
            }
            else if (x1 < pxmin)
            {
                x1 = pxmin;
                y1 = (e->c - x1) / e->b;
            }
            if (x2 > pxmax)
            {
                x2 = pxmax;
                y2 = (e->c - x2) / e->b;
            }
            else if (x2 < pxmin)
            {
                x2 = pxmin;
                y2 = (e->c - x2) / e->b;
            }
        }
        else // delta y is larger
        {
            x1 = pxmin;
            if (s1 != 0 && s1->x > pxmin)
                x1 = s1->x;
            if (x1 > pxmax)
            {
                x1 = pxmax;
            }
            y1 = e->c - e->a * x1;
            x2 = pxmax;
            if (s2 != 0 && s2->x < pxmax)
                x2 = s2->x;
            if (x2 < pxmin)
            {
                x2 = pxmin;
            }
            y2 = e->c - e->a * x2;
            // Never occurs according to lcov
            // if( ((y1 > pymax) & (y2 > pymax)) | ((y1 < pymin) & (y2 < pymin)) )
            // {
            //     return 0;
            // }
            if (y1 > pymax)
            {
                y1 = pymax;
                x1 = (e->c - y1) / e->a;
            }
            else if (y1 < pymin)
            {
                y1 = pymin;
                x1 = (e->c - y1) / e->a;
            }
            if (y2 > pymax)
            {
                y2 = pymax;
                x2 = (e->c - y2) / e->a;
            }
            else if (y2 < pymin)
            {
                y2 = pymin;
                x2 = (e->c - y2) / e->a;
            }
        }

        e->pos[0].x = x1;
        e->pos[0].y = y1;
        e->pos[1].x = x2;
        e->pos[1].y = y2;

        // If the two points are equal, the result is invalid
        return (x1 == x2 && y1 == y2) ? 0 : 1;
    };
    static int jcv_edge_clipline(jcv_context_internal* internal, jcv_edge* e) {
        return internal->clipper.clip_fn(&internal->clipper, e);
    };
    static jcv_edge* jcv_edge_new(jcv_context_internal* internal, jcv_site* s1, jcv_site* s2) {
        jcv_edge* e = jcv_alloc_edge(internal);
        jcv_edge_create(e, s1, s2);
        return e;
    };
    static void jcv_halfedge_link(jcv_halfedge* edge, jcv_halfedge* newedge) {
        newedge->left = edge;
        newedge->right = edge->right;
        edge->right->left = newedge;
        edge->right = newedge;
    };
    static void jcv_halfedge_unlink(jcv_halfedge* he) {
        he->left->right = he->right;
        he->right->left = he->left;
        he->left = 0;
        he->right = 0;
    };
    static jcv_halfedge* jcv_halfedge_new(jcv_context_internal* internal, jcv_edge* e, int direction) {
        jcv_halfedge* he = jcv_alloc_halfedge(internal);
        he->edge = e;
        he->left = 0;
        he->right = 0;
        he->direction = direction;
        he->pqpos = 0;
        // These are set outside
        //he->y
        //he->vertex
        return he;
    };
    static void jcv_halfedge_delete(jcv_context_internal* internal, jcv_halfedge* he) {
        he->right = internal->halfedgepool;
        internal->halfedgepool = he;
    };
    static jcv_site* jcv_halfedge_leftsite(const jcv_halfedge* he) {
        return he->edge->sites[he->direction];
    };
    static jcv_site* jcv_halfedge_rightsite(const jcv_halfedge* he) {
        return he->edge ? he->edge->sites[1 - he->direction] : 0;
    };
    static int jcv_halfedge_rightof(const jcv_halfedge* he, const jcv_point* p) {
        const jcv_edge* e = he->edge;
        const jcv_site* topsite = e->sites[1];

        int right_of_site = (p->x > topsite->p.x) ? 1 : 0;
        if (right_of_site && he->direction == JCV_DIRECTION_LEFT)
            return 1;
        if (!right_of_site && he->direction == JCV_DIRECTION_RIGHT)
            return 0;

        double dxp, dyp, dxs, t1, t2, t3, yl;

        int above;
        if (e->a == (double)1)
        {
            dyp = p->y - topsite->p.y;
            dxp = p->x - topsite->p.x;
            int fast = 0;
            if ((!right_of_site & (e->b < (double)0)) | (right_of_site & (e->b >= (double)0)))
            {
                above = dyp >= e->b * dxp;
                fast = above;
            }
            else
            {
                above = (p->x + p->y * e->b) > e->c;
                if (e->b < (double)0)
                    above = !above;
                if (!above)
                    fast = 1;
            }
            if (!fast)
            {
                dxs = topsite->p.x - e->sites[0]->p.x;
                above = e->b * (dxp * dxp - dyp * dyp)
                    < dxs * dyp * ((double)1 + (double)2 * dxp / dxs + e->b * e->b);
                if (e->b < (double)0)
                    above = !above;
            }
        }
        else // e->b == 1
        {
            yl = e->c - e->a * p->x;
            t1 = p->y - yl;
            t2 = p->x - topsite->p.x;
            t3 = yl - topsite->p.y;
            above = t1 * t1 > (t2 * t2 + t3 * t3);
        }
        return (he->direction == JCV_DIRECTION_LEFT ? above : !above);
    };
    static int jcv_halfedge_compare(const jcv_halfedge* he1, const jcv_halfedge* he2) {
        return  (he1->y == he2->y) ? he1->vertex.x > he2->vertex.x : he1->y > he2->y;
    };
    static int jcv_halfedge_intersect(const jcv_halfedge* he1, const jcv_halfedge* he2, jcv_point* out) {
        const jcv_edge* e1 = he1->edge;
        const jcv_edge* e2 = he2->edge;

        double d = e1->a * e2->b - e1->b * e2->a;
        if (((double)-JCV_EDGE_INTERSECT_THRESHOLD < d && d < (double)JCV_EDGE_INTERSECT_THRESHOLD))
        {
            return 0;
        }
        out->x = (e1->c * e2->b - e1->b * e2->c) / d;
        out->y = (e1->a * e2->c - e1->c * e2->a) / d;

        const jcv_edge* e;
        const jcv_halfedge* he;
        if (jcv_point_less(&e1->sites[1]->p, &e2->sites[1]->p))
        {
            he = he1;
            e = e1;
        }
        else
        {
            he = he2;
            e = e2;
        }

        int right_of_site = out->x >= e->sites[1]->p.x;
        if ((right_of_site && he->direction == JCV_DIRECTION_LEFT) || (!right_of_site && he->direction == JCV_DIRECTION_RIGHT))
        {
            return 0;
        }

        return 1;
    };
    static int jcv_pq_moveup(jcv_priorityqueue* pq, int pos) {
        jcv_halfedge** items = (jcv_halfedge**)pq->items;
        jcv_halfedge* node = items[pos];

        for (int parent = (pos >> 1);
            pos > 1 && jcv_halfedge_compare(items[parent], node);
            pos = parent, parent = parent >> 1)
        {
            items[pos] = items[parent];
            items[pos]->pqpos = pos;
        }

        node->pqpos = pos;
        items[pos] = node;
        return pos;
    };
    static int jcv_pq_maxchild(jcv_priorityqueue* pq, int pos) {
        int child = pos << 1;
        if (child >= pq->numitems)
            return 0;
        jcv_halfedge** items = (jcv_halfedge**)pq->items;
        if ((child + 1) < pq->numitems && jcv_halfedge_compare(items[child], items[child + 1]))
            return child + 1;
        return child;
    };
    static int jcv_pq_movedown(jcv_priorityqueue* pq, int pos) {
        jcv_halfedge** items = (jcv_halfedge**)pq->items;
        jcv_halfedge* node = items[pos];

        int child = jcv_pq_maxchild(pq, pos);
        while (child && jcv_halfedge_compare(node, items[child]))
        {
            items[pos] = items[child];
            items[pos]->pqpos = pos;
            pos = child;
            child = jcv_pq_maxchild(pq, pos);
        }

        items[pos] = node;
        items[pos]->pqpos = pos;
        return pos;
    };
    static void jcv_pq_create(jcv_priorityqueue* pq, int capacity, void** buffer) {
        pq->maxnumitems = capacity;
        pq->numitems = 1;
        pq->items = buffer;
    };
    static int jcv_pq_empty(jcv_priorityqueue* pq) {
        return pq->numitems == 1 ? 1 : 0;
    };
    static int jcv_pq_push(jcv_priorityqueue* pq, void* node) {
        assert(pq->numitems < pq->maxnumitems);
        int n = pq->numitems++;
        pq->items[n] = node;
        return jcv_pq_moveup(pq, n);
    };
    static void* jcv_pq_pop(jcv_priorityqueue* pq) {
        void* node = pq->items[1];
        pq->items[1] = pq->items[--pq->numitems];
        jcv_pq_movedown(pq, 1);
        return node;
    };
    static void* jcv_pq_top(jcv_priorityqueue* pq) {
        return pq->items[1];
    };
    static void jcv_pq_remove(jcv_priorityqueue* pq, jcv_halfedge* node) {
        if (pq->numitems == 1)
            return;
        int pos = node->pqpos;
        if (pos == 0)
            return;

        jcv_halfedge** items = (jcv_halfedge**)pq->items;

        items[pos] = items[--pq->numitems];
        if (jcv_halfedge_compare(node, items[pos]))
            jcv_pq_moveup(pq, pos);
        else
            jcv_pq_movedown(pq, pos);
        node->pqpos = pos;
    };
    static jcv_site* jcv_nextsite(jcv_context_internal* internal) {
        return (internal->currentsite < internal->numsites) ? &internal->sites[internal->currentsite++] : 0;
    };
    static jcv_halfedge* jcv_get_edge_above_x(jcv_context_internal* internal, const jcv_point* p) {
        // Gets the arc on the beach line at the x coordinate (i.e. right above the new site event)

        // A good guess it's close by (Can be optimized)
        jcv_halfedge* he = internal->last_inserted;
        if (!he)
        {
            if (p->x < (internal->rect.max.x - internal->rect.min.x) / 2)
                he = internal->beachline_start;
            else
                he = internal->beachline_end;
        }

        //
        if (he == internal->beachline_start || (he != internal->beachline_end && jcv_halfedge_rightof(he, p)))
        {
            do {
                he = he->right;
            } while (he != internal->beachline_end && jcv_halfedge_rightof(he, p));

            he = he->left;
        }
        else
        {
            do {
                he = he->left;
            } while (he != internal->beachline_start && !jcv_halfedge_rightof(he, p));
        }

        return he;
    };
    static int jcv_check_circle_event(const jcv_halfedge* he1, const jcv_halfedge* he2, jcv_point* vertex) {
        jcv_edge* e1 = he1->edge;
        jcv_edge* e2 = he2->edge;
        if (e1 == nullptr || e2 == nullptr || e1->sites[1] == e2->sites[1])
        {
            return 0;
        }

        return jcv_halfedge_intersect(he1, he2, vertex);
    };
    static void jcv_site_event(jcv_context_internal* internal, jcv_site* site) {
        jcv_halfedge* left = jcv_get_edge_above_x(internal, &site->p);
        jcv_halfedge* right = left->right;
        jcv_site* bottom = jcv_halfedge_rightsite(left);
        if (!bottom)
            bottom = internal->bottomsite;

        jcv_edge* edge = jcv_edge_new(internal, bottom, site);
        edge->next = internal->edges;
        internal->edges = edge;

        jcv_halfedge* edge1 = jcv_halfedge_new(internal, edge, JCV_DIRECTION_LEFT);
        jcv_halfedge* edge2 = jcv_halfedge_new(internal, edge, JCV_DIRECTION_RIGHT);

        jcv_halfedge_link(left, edge1);
        jcv_halfedge_link(edge1, edge2);

        internal->last_inserted = right;

        jcv_point p;
        if (jcv_check_circle_event(left, edge1, &p))
        {
            jcv_pq_remove(internal->eventqueue, left);
            left->vertex = p;
            left->y = p.y + jcv_point_dist(&site->p, &p);
            jcv_pq_push(internal->eventqueue, left);
        }
        if (jcv_check_circle_event(edge2, right, &p))
        {
            edge2->vertex = p;
            edge2->y = p.y + jcv_point_dist(&site->p, &p);
            jcv_pq_push(internal->eventqueue, edge2);
        }
    }
    static double jcv_determinant(const jcv_point* a, const jcv_point* b, const jcv_point* c) {
        return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    };
    static double jcv_calc_sort_metric(const jcv_site* site, const jcv_graphedge* edge) {
        // We take the average of the two points, since we can better distinguish between very small edges
        double half = 1 / (double)2;
        double x = (edge->pos[0].x + edge->pos[1].x) * half;
        double y = (edge->pos[0].y + edge->pos[1].y) * half;
        double diffy = y - site->p.y;
        double angle = std::atan2(diffy, x - site->p.x);
        if (diffy < 0)
            angle = angle + 2 * cweeMath::PI;
        return (double)angle;
    };
    static int jcv_graphedge_eq(jcv_graphedge* a, jcv_graphedge* b) {
        return double_eq(a->angle, b->angle) && jcv_point_eq(&a->pos[0], &b->pos[0]) && jcv_point_eq(&a->pos[1], &b->pos[1]);
    };
    static void jcv_sortedges_insert(jcv_site* site, jcv_graphedge* edge) {
        // Special case for the head end
        jcv_graphedge* prev = 0;
        if (site->edges == nullptr || site->edges->angle >= edge->angle)
        {
            edge->next = site->edges;
            site->edges = edge;
        }
        else
        {
            // Locate the node before the point of insertion
            jcv_graphedge* current = site->edges;
            while (current->next != 0 && current->next->angle < edge->angle)
            {
                current = current->next;
            }
            prev = current;
            edge->next = current->next;
            current->next = edge;
        }

        // check to avoid duplicates
        if (prev && jcv_graphedge_eq(prev, edge))
        {
            prev->next = edge->next;
        }
        else if (edge->next && jcv_graphedge_eq(edge, edge->next))
        {
            edge->next = edge->next->next;
        }
    };
    static void jcv_finishline(jcv_context_internal* internal, jcv_edge* e) {
        if (!jcv_edge_clipline(internal, e)) {
            return;
        }

        // Make sure the graph edges are CCW
        int flip = jcv_determinant(&e->sites[0]->p, &e->pos[0], &e->pos[1]) > (double)0 ? 0 : 1;

        for (int i = 0; i < 2; ++i) {
            jcv_graphedge* ge = jcv_alloc_graphedge(internal);

            ge->edge = e;
            ge->next = 0;
            ge->neighbor = e->sites[1 - i];
            ge->pos[flip] = e->pos[i];
            ge->pos[1 - flip] = e->pos[1 - i];
            ge->angle = jcv_calc_sort_metric(e->sites[i], ge);

            jcv_sortedges_insert(e->sites[i], ge);
        }
    };
    static void jcv_endpos(jcv_context_internal* internal, jcv_edge* e, const jcv_point* p, int direction) {
        e->pos[direction] = *p;

        if (!jcv_is_valid(&e->pos[1 - direction]))
            return;

        jcv_finishline(internal, e);
    };
    static void jcv_create_corner_edge(jcv_context_internal* internal, const jcv_site* site, jcv_graphedge* current, jcv_graphedge* gap) {
        gap->neighbor = 0;
        gap->pos[0] = current->pos[1];

        if (current->pos[1].x < internal->rect.max.x && current->pos[1].y == internal->rect.min.y)
        {
            gap->pos[1].x = internal->rect.max.x;
            gap->pos[1].y = internal->rect.min.y;
        }
        else if (current->pos[1].x > internal->rect.min.x && current->pos[1].y == internal->rect.max.y)
        {
            gap->pos[1].x = internal->rect.min.x;
            gap->pos[1].y = internal->rect.max.y;
        }
        else if (current->pos[1].y > internal->rect.min.y && current->pos[1].x == internal->rect.min.x)
        {
            gap->pos[1].x = internal->rect.min.x;
            gap->pos[1].y = internal->rect.min.y;
        }
        else if (current->pos[1].y < internal->rect.max.y && current->pos[1].x == internal->rect.max.x)
        {
            gap->pos[1].x = internal->rect.max.x;
            gap->pos[1].y = internal->rect.max.y;
        }

        gap->angle = jcv_calc_sort_metric(site, gap);
    };
    static jcv_edge* jcv_create_gap_edge(jcv_context_internal* internal, jcv_site* site, jcv_graphedge* ge) {
        jcv_edge* edge{ jcv_alloc_edge(internal) };
        edge->pos[0] = ge->pos[0];
        edge->pos[1] = ge->pos[1];
        edge->sites[0] = site;
        edge->sites[1] = 0;
        edge->a = edge->b = edge->c = 0;
        
        edge->next = internal->edges;
        internal->edges = edge;

        return edge;
    };
    static void jcv_boxshape_fillgaps(const jcv_clipper* clipper, jcv_context_internal* allocator, jcv_site* site) {
        // They're sorted CCW, so if the current->pos[1] != next->pos[0], then we have a gap
        jcv_point corner; 
        int current_edge_flags, next_edge_flags, corner_flag;
        jcv_graphedge *gap, *current, *next;

        current = site->edges;
        if (!current) {
            // No edges, then it should be a single cell
            assert(allocator->numsites == 1);

            gap = jcv_alloc_graphedge(allocator);
            gap->neighbor = 0;
            gap->pos[0] = clipper->min;
            gap->pos[1].x = clipper->max.x;
            gap->pos[1].y = clipper->min.y;
            gap->angle = jcv_calc_sort_metric(site, gap);
            gap->next = 0;
            gap->edge = jcv_create_gap_edge(allocator, site, gap);

            current = gap;
            site->edges = gap;
        }

        next = current->next;
        if (!next) {
            gap = jcv_alloc_graphedge(allocator);
            jcv_create_corner_edge(allocator, site, current, gap);
            gap->edge = jcv_create_gap_edge(allocator, site, gap);

            gap->next = current->next;
            current->next = gap;
            current = gap;
            next = site->edges;
        }

        while (current && next) {
            current_edge_flags = jcv_get_edge_flags(&current->pos[1], &clipper->min, &clipper->max);
            if (current_edge_flags && !jcv_point_eq(&current->pos[1], &next->pos[0])) {
                // Cases:
                //  Current and Next on the same border
                //  Current on one border, and Next on another border
                //  Current on the corner, Next on the border
                //  Current on the corner, Next on another border (another corner in between)

                next_edge_flags = jcv_get_edge_flags(&next->pos[0], &clipper->min, &clipper->max);
                if (current_edge_flags & next_edge_flags) {
                    // Current and Next on the same border
                    gap = jcv_alloc_graphedge(allocator);
                    gap->neighbor = 0;
                    gap->pos[0] = current->pos[1];
                    gap->pos[1] = next->pos[0];
                    gap->angle = jcv_calc_sort_metric(site, gap);
                    gap->edge = jcv_create_gap_edge(allocator, site, gap);

                    gap->next = current->next;
                    current->next = gap;
                }
                else {
                    // Current and Next on different borders
                    corner_flag = jcv_edge_flags_to_corner(current_edge_flags);
                    if (corner_flag)
                    {
                        // we are already at one corner, so we need to find the next one
                        corner_flag = jcv_corner_rotate_90(corner_flag);
                    }
                    else
                    {
                        // we are on the middle of a border
                        // we need to find the adjacent corner, following the borders CCW
                        if (current_edge_flags == JCV_EDGE_TOP) { corner_flag = JCV_CORNER_TOP_LEFT; }
                        else if (current_edge_flags == JCV_EDGE_LEFT) { corner_flag = JCV_CORNER_BOTTOM_LEFT; }
                        else if (current_edge_flags == JCV_EDGE_BOTTOM) { corner_flag = JCV_CORNER_BOTTOM_RIGHT; }
                        else if (current_edge_flags == JCV_EDGE_RIGHT) { corner_flag = JCV_CORNER_TOP_RIGHT; }

                    }
                    corner = jcv_corner_to_point(corner_flag, &clipper->min, &clipper->max);

                    gap = jcv_alloc_graphedge(allocator);
                    gap->neighbor = 0;
                    gap->pos[0] = current->pos[1];
                    gap->pos[1] = corner;
                    gap->angle = jcv_calc_sort_metric(site, gap);
                    gap->edge = jcv_create_gap_edge(allocator, site, gap);

                    gap->next = current->next;
                    current->next = gap;
                }
            }

            current = current->next;
            if (current)
            {
                next = current->next;
                if (!next) next = site->edges;
            }
        }
    };
    static void jcv_fillgaps(jcv_diagram* diagram) {
        int i; jcv_context_internal* internal;

        internal = diagram->internal;
        if (!internal->clipper.fill_fn)
            return;

        for (i = 0; i < internal->numsites; ++i) {
            internal->clipper.fill_fn(&internal->clipper, internal, &internal->sites[i]);
        }
    };
    static void jcv_circle_event(jcv_context_internal* internal) {
        jcv_halfedge* left = (jcv_halfedge*)jcv_pq_pop(internal->eventqueue);

        jcv_halfedge* leftleft = left->left;
        jcv_halfedge* right = left->right;
        jcv_halfedge* rightright = right->right;
        jcv_site* bottom = jcv_halfedge_leftsite(left);
        jcv_site* top = jcv_halfedge_rightsite(right);

        jcv_point vertex = left->vertex;
        jcv_endpos(internal, left->edge, &vertex, left->direction);
        jcv_endpos(internal, right->edge, &vertex, right->direction);

        internal->last_inserted = rightright;

        jcv_pq_remove(internal->eventqueue, right);
        jcv_halfedge_unlink(left);
        jcv_halfedge_unlink(right);
        jcv_halfedge_delete(internal, left);
        jcv_halfedge_delete(internal, right);

        int direction = JCV_DIRECTION_LEFT;
        if (bottom->p.y > top->p.y)
        {
            jcv_site* temp = bottom;
            bottom = top;
            top = temp;
            direction = JCV_DIRECTION_RIGHT;
        }

        jcv_edge* edge = jcv_edge_new(internal, bottom, top);
        edge->next = internal->edges;
        internal->edges = edge;

        jcv_halfedge* he = jcv_halfedge_new(internal, edge, direction);
        jcv_halfedge_link(leftleft, he);
        jcv_endpos(internal, edge, &vertex, JCV_DIRECTION_RIGHT - direction);

        jcv_point p;
        if (jcv_check_circle_event(leftleft, he, &p))
        {
            jcv_pq_remove(internal->eventqueue, leftleft);
            leftleft->vertex = p;
            leftleft->y = p.y + jcv_point_dist(&bottom->p, &p);
            jcv_pq_push(internal->eventqueue, leftleft);
        }
        if (jcv_check_circle_event(he, rightright, &p))
        {
            he->vertex = p;
            he->y = p.y + jcv_point_dist(&bottom->p, &p);
            jcv_pq_push(internal->eventqueue, he);
        }
    };
    static void jcv_rect_union(jcv_rect* rect, const jcv_point* p) {
        rect->min.x = jcv_min(rect->min.x, p->x);
        rect->min.y = jcv_min(rect->min.y, p->y);
        rect->max.x = jcv_max(rect->max.x, p->x);
        rect->max.y = jcv_max(rect->max.y, p->y);
    };
    static void jcv_rect_round(jcv_rect* rect) {
        rect->min.x = jcv_floor(rect->min.x);
        rect->min.y = jcv_floor(rect->min.y);
        rect->max.x = jcv_ceil(rect->max.x);
        rect->max.y = jcv_ceil(rect->max.y);
    };
    static void jcv_rect_inflate(jcv_rect* rect, double amount) {
        rect->min.x -= amount;
        rect->min.y -= amount;
        rect->max.x += amount;
        rect->max.y += amount;
    };
    static int jcv_prune_duplicates(jcv_context_internal* internal, jcv_rect* rect) {
        int num_sites = internal->numsites;
        jcv_site* sites = internal->sites;

        jcv_rect r;
        r.min.x = r.min.y = DBL_MAX;
        r.max.x = r.max.y = -DBL_MAX;

        int offset = 0;
        // Prune duplicates first
        for (int i = 0; i < num_sites; i++)
        {
            const jcv_site* s = &sites[i];
            // Remove duplicates, to avoid anomalies
            if (i > 0 && jcv_point_eq(&s->p, &sites[i - 1].p))
            {
                offset++;
                continue;
            }

            sites[i - offset] = sites[i];

            jcv_rect_union(&r, &s->p);
        }
        internal->numsites -= offset;
        if (rect) {
            *rect = r;
        }
        return offset;
    };
    static int jcv_prune_not_in_shape(jcv_context_internal* internal, jcv_rect* rect) {
        int num_sites = internal->numsites;
        jcv_site* sites = internal->sites;

        jcv_rect r;
        r.min.x = r.min.y = DBL_MAX;
        r.max.x = r.max.y = -DBL_MAX;

        int offset = 0;
        for (int i = 0; i < num_sites; i++)
        {
            const jcv_site* s = &sites[i];

            if (!internal->clipper.test_fn(&internal->clipper, s->p))
            {
                offset++;
                continue;
            }

            sites[i - offset] = sites[i];

            jcv_rect_union(&r, &s->p);
        }
        internal->numsites -= offset;
        if (rect) {
            *rect = r;
        }
        return offset;
    };
    static jcv_context_internal* jcv_alloc_internal(int num_points, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn) {
        // Interesting limits from Euler's equation
        // Slide 81: https://courses.cs.washington.edu/courses/csep521/01au/lectures/lecture10slides.pdf
        // Page 3: https://sites.cs.ucsb.edu/~suri/cs235/Voronoi.pdf
        size_t eventssize = (size_t)(num_points * 2) * sizeof(void*); // beachline can have max 2*n-5 parabolas
        size_t sitessize = (size_t)num_points * sizeof(jcv_site);
        size_t memsize = sizeof(jcv_priorityqueue) + eventssize + sitessize + 16u; // 16 bytes padding for alignment

        char* originalmem = (char*)allocfn(userallocctx, memsize);
        memset(originalmem, 0, memsize);

        // align memory
        char* mem = (char*)jcv_align(originalmem, sizeof(void*));

        jcv_context_internal* internal = new jcv_context_internal();
        internal->mem = originalmem;
        internal->memctx = userallocctx;
        internal->alloc = allocfn;
        internal->free = freefn;

        mem = (char*)jcv_align(mem, sizeof(void*));
        internal->sites = (jcv_site*)mem;
        mem += sitessize;

        mem = (char*)jcv_align(mem, sizeof(void*));
        internal->eventqueue = (jcv_priorityqueue*)mem;
        mem += sizeof(jcv_priorityqueue);
        assert(((uintptr_t)mem & (sizeof(void*) - 1)) == 0);

        jcv_cast_align_struct tmp;
        tmp.charp = mem;
        internal->eventmem = tmp.voidpp;

        assert((mem + eventssize) <= (originalmem + memsize));

        return internal;
    }; 
    static void jcv_diagram_generate_useralloc(int num_points, const jcv_point* points, jcv_rect* rect, jcv_clipper* clipper, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* d) {
        if (d->internal)
            jcv_diagram_free(d);

        jcv_context_internal* internal = jcv_alloc_internal(num_points, userallocctx, allocfn, freefn);

        internal->beachline_start = jcv_halfedge_new(internal, 0, 0);
        internal->beachline_end = jcv_halfedge_new(internal, 0, 0);

        internal->beachline_start->left = 0;
        internal->beachline_start->right = internal->beachline_end;
        internal->beachline_end->left = internal->beachline_start;
        internal->beachline_end->right = 0;

        internal->last_inserted = 0;

        int max_num_events = num_points * 2; // beachline can have max 2*n-5 parabolas
        jcv_pq_create(internal->eventqueue, max_num_events, (void**)internal->eventmem);

        internal->numsites = num_points;
        jcv_site* sites = internal->sites;

        for (int i = 0; i < num_points; ++i)
        {
            sites[i].p = points[i];
            sites[i].edges = 0;
            sites[i].index = i;
        }

        //cweeThreadedList<_type_>& Sort(std::function<bool(_type_ const&, _type_ const&)> func) {
        //    if (num >= 2) {
        //        auto* first = &list[0];
        //        auto* last = first + num;
        //        std::sort(first, last, func);
        //    }
        //    return *this;
        //};
        if (num_points >= 2) {
            auto* first = &sites[0];
            auto* last = first + num_points;
            std::sort(first, last, [](jcv_site const& s1, jcv_site const& s2)-> bool {
                return (s1.p.y != s2.p.y) ? (s1.p.y < s2.p.y ? true : false) : (s1.p.x < s2.p.x ? true : false);
            });
        }
        // qsort(sites, (size_t)num_points, sizeof(jcv_site), jcv_point_cmp);

        if (clipper) {
            internal->clipper = *clipper;
        } 
        else {
            internal->clipper.test_fn = jcv_boxshape_test;
            internal->clipper.clip_fn = jcv_boxshape_clip;
            internal->clipper.fill_fn = jcv_boxshape_fillgaps;
        }

        jcv_rect tmp_rect;
        tmp_rect.min.x = tmp_rect.min.y = DBL_MAX;
        tmp_rect.max.x = tmp_rect.max.y = -DBL_MAX;
        jcv_prune_duplicates(internal, &tmp_rect);

        // Prune using the test second
        if (internal->clipper.test_fn)
        {
            // e.g. used by the box clipper in the test_fn
            internal->clipper.min = rect ? rect->min : tmp_rect.min;
            internal->clipper.max = rect ? rect->max : tmp_rect.max;

            jcv_prune_not_in_shape(internal, &tmp_rect);

            // The pruning might have made the bounding box smaller
            if (!rect) {
                // In the case of all sites being all on a horizontal or vertical line, the
                // rect area will be zero, and the diagram generation will most likely fail
                jcv_rect_round(&tmp_rect);
                jcv_rect_inflate(&tmp_rect, 10);

                internal->clipper.min = tmp_rect.min;
                internal->clipper.max = tmp_rect.max;
            }
        }

        internal->rect = rect ? *rect : tmp_rect;

        d->min = internal->rect.min;
        d->max = internal->rect.max;
        d->numsites = internal->numsites;
        d->internal = internal;

        internal->bottomsite = jcv_nextsite(internal);

        jcv_priorityqueue* pq = internal->eventqueue;
        jcv_site* site = jcv_nextsite(internal);

        int finished = 0;
        while (!finished)
        {
            jcv_point lowest_pq_point;
            if (!jcv_pq_empty(pq))
            {
                jcv_halfedge* he = (jcv_halfedge*)jcv_pq_top(pq);
                lowest_pq_point.x = he->vertex.x;
                lowest_pq_point.y = he->y;
            }

            if (site != 0 && (jcv_pq_empty(pq) || jcv_point_less(&site->p, &lowest_pq_point)))
            {
                jcv_site_event(internal, site);
                site = jcv_nextsite(internal);
            }
            else if (!jcv_pq_empty(pq))
            {
                jcv_circle_event(internal);
            }
            else
            {
                finished = 1;
            }
        }

        for (jcv_halfedge* he = internal->beachline_start->right; he != internal->beachline_end; he = he->right)
        {
            jcv_finishline(internal, he->edge);
        }

        jcv_fillgaps(d);
    };
    static void jcv_diagram_generate(int num_points, const jcv_point* points, jcv_rect* rect, jcv_clipper* clipper, jcv_diagram* d) {        
        jcv_diagram_generate_useralloc(num_points, points, rect, clipper, nullptr, jcv_alloc_fn, jcv_free_fn, d);
    };
};

#endif // JC_VORONOI_H

class Voronoi {
public:
    class Cell {
    public:
        vec2d center;
        cweeList<vec2d> points;
        vec2d bottomLeft = vec2d(cweeMath::INF, cweeMath::INF);
        vec2d topRight = vec2d(-cweeMath::INF, -cweeMath::INF);

    public:
        Cell() = default;
        Cell(Cell const&) = default;
        Cell(Cell&&) = default;
        Cell& operator=(Cell&&) = default;
        Cell& operator=(Cell const&) = default;
        void AddPoint(vec2d const& p) {
            points.Append(p);
            if (bottomLeft.x > p.x) bottomLeft.x = p.x;
            if (bottomLeft.y > p.y) bottomLeft.y = p.y;
            if (topRight.x < p.x) topRight.x = p.x;
            if (topRight.y < p.y) topRight.y = p.y;
        }

        void Clear() {
            center = vec2d();
            points.Clear();
            bottomLeft = vec2d(cweeMath::INF, cweeMath::INF);
            topRight = vec2d(-cweeMath::INF, -cweeMath::INF);
        };
        bool overlaps(vec2d const& point) const {
            if (bottomLeft.x <= point.x && topRight.x >= point.x && topRight.y >= point.y && bottomLeft.y <= point.y) {
                return cweeEng::IsPointInPolygon(points, point);
            }
            else {
                return false;
            }
        };
        
        bool overlaps(Cell const& cell) const {
            if (bottomLeft.x <= cell.topRight.x && topRight.x >= cell.bottomLeft.x && topRight.y >= cell.bottomLeft.y && bottomLeft.y <= cell.topRight.y) {
                return cweeEng::PolygonsOverlap(cell.points, points);
            }
            else {
                return false;
            }
        };

        bool overlaps(vec2d const& topRight_p, vec2d const& bottomLeft_p) const {
            cweeList<vec2d> points_p;
            points_p.Append(bottomLeft_p);
            points_p.Append(vec2d(bottomLeft_p.x, topRight_p.y));
            points_p.Append(topRight_p);
            points_p.Append(vec2d(topRight_p.x, bottomLeft_p.y));

            if (bottomLeft.x <= topRight_p.x && topRight.x >= bottomLeft_p.x && topRight.y >= bottomLeft_p.y && bottomLeft.y <= topRight_p.y) {
                return cweeEng::PolygonsOverlap(points_p, points);
            }
            else {
                return false;
            }
        };
    };

private:
    cweeSharedPtr<JCV::jcv_diagram> Diagram;
    cweeSharedPtr < cweeList<vec2d>> CoordinateData;

public:
    Voronoi() = default;
    Voronoi(cweeSharedPtr < cweeList<vec2d>> const& coords_src) {
        CoordinateData = coords_src; 
        { // remove duplicates
            cweeInterpolatedMatrix<double> matrx;
            for (int i = CoordinateData->Num() - 1; i >= 0; i--) {
                auto& x = CoordinateData->operator[](i);
                if (matrx.ContainsPosition(x.x, x.y)) CoordinateData->RemoveIndexFast(i);
                else matrx.AddValue(x.x, x.y, 0, true);
            }
        }
        if (CoordinateData->Num() <= 1) {
            throw(std::runtime_error("Voronoi algorithm does not support fewer than 2 unique points."));
        }
        // try to reduce likelihood of three points in a line // TODO -- is there a way to check for this?
        for (auto& x : *CoordinateData) {
            x.x += cweeRandomFloat(-JCV_RAND_BUFFER, JCV_RAND_BUFFER);
            x.y += cweeRandomFloat(-JCV_RAND_BUFFER, JCV_RAND_BUFFER);
        }
        const auto* points = static_cast<const JCV::jcv_point*>((const void*)CoordinateData->Ptr()); // vec2d shares same structure.
        Diagram = cweeSharedPtr<JCV::jcv_diagram>(new JCV::jcv_diagram(), [=](JCV::jcv_diagram* p) {
            JCV::jcv_diagram_free(p); // free children
            CoordinateData = nullptr;
            delete p; // delete ptr
        });
        JCV::jcv_diagram_generate(CoordinateData->Num(), points, nullptr, nullptr, Diagram.get());
    };
    Voronoi(Voronoi const&) = default;
    Voronoi(Voronoi&&) = default;
    Voronoi& operator=(Voronoi&&) = default;
    Voronoi& operator=(Voronoi const&) = default;

    cweeList< cweePair<vec2d, vec2d> > GetEdges() const {
        cweeList < cweePair<vec2d, vec2d> > out;
        if (Diagram) {
            cweePair<vec2d, vec2d> row;
            const auto* e = JCV::jcv_diagram_get_edges(Diagram.get());
            while (e) {
                row.first.x = e->pos[0].x;
                row.first.y = e->pos[0].y;
                row.second.x = e->pos[1].x;
                row.second.y = e->pos[1].y;

                out.Append(row);

                e = JCV::jcv_diagram_get_next_edge(e);
            }
        }
        return out;
    };
    cweeList < Cell > GetCells() const {
        cweeList < Cell > out;
        Cell cell;
        if (Diagram) {
            const auto* sites = JCV::jcv_diagram_get_sites(Diagram.get());
            for (int i = 0; i < Diagram->numsites; ++i) {
                cell.Clear();
                const auto* site = &sites[i];
                const auto* e = site->edges;
                cell.center = *((vec2d*)(void*)(&(site->p)));  // casting structs with same contents. Unsafe code but fast.
                while (e) {
                    cell.AddPoint(*((vec2d*)(void*)(&(e->pos[0])))); // casting structs with same contents. Unsafe code but fast.
                    e = e->next;
                }
                out.Append(cell);
            }
        }

        return out;
    };
    void Clear() {
        Diagram = nullptr;
        CoordinateData = nullptr;
    };
};