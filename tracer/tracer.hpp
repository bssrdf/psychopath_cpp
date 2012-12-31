/*
 * This file and tracer.cpp define a Tracer class, which manages the tracing
 * of rays in a scene.
 */
#ifndef TRACER_HPP
#define TRACER_HPP

#include "numtype.h"
#include "array.hpp"
#include "job_queue.hpp"

#include <vector>

#include "ray.hpp"
#include "rayinter.hpp"
#include "potentialinter.hpp"
#include "scene.hpp"

struct IndexRange {
	uint_i start, end;

	IndexRange() {
		start=0;
		end=0;
	}
	IndexRange(uint_i start_, uint_i end_) {
		start=start_;
		end=end_;
	}
};

struct PotintJob {
	RayInter *ray_inters;
	uint_i start, end, size;

	PotintJob() {
		start=0;
		end=0;
		ray_inters = NULL;
	}
	PotintJob(uint_i start_, uint_i end_, RayInter *ray_inters_) {
		start=start_;
		end=end_;
		size = end - start;
		ray_inters = ray_inters_;
	}
};

/**
 * @brief Traces rays in a scene.
 *
 * The Tracer is responsible for doing the actual ray-tracing in a scene.
 * It does _not_ manage the specific integration algorithm, or shading.  Only
 * the tracing of rays and calculating the relevant information about ray
 * hits.
 *
 * It is specifically designed to handle tracing a large number of rays
 * (ideally > a million, as ram allows) simultaneously to gain efficiency
 * in various ways.  The rays do not need to be related to each other or
 * coherent in any way.
 *
 * It is, of course, also capable of tracing a single ray at a time or a small
 * number of rays at a time if necessary. But doing so may be far less
 * efficient depending on the scene.
 *
 * The simplest usage is to add a bunch of rays to the Tracer's queue with
 * queue_rays(), and then trace them all by calling trace_rays().  The
 * resulting intersection data is stored in the rays' data structures directly.
 * Wash, rinse, repeat.
 */
class Tracer
{
public:
	Scene *scene;
	int thread_count;

	Array<RayInter *> ray_inters; // Ray/Intersection data
	Array<byte> states; // Ray states, for interrupting and resuming traversal
	Array<PotentialInter> potential_inters; // "Potential intersection" buffer

	// Arrays for the counting sort
	Array<uint32> item_counts;
	Array<uint32> item_start_indices;
	Array<uint32> item_fill_counts;

	Tracer(Scene *scene_, int thread_count_=1) {
		scene = scene_;
		thread_count = thread_count_;

		item_counts.resize(scene->world.max_primitive_id()+1);
		item_start_indices.resize(scene->world.max_primitive_id()+1);
		item_fill_counts.resize(scene->world.max_primitive_id()+1);
	}

	/**
	 * Adds a set of rays to the ray queue for tracing.
	 * Returns the number of rays currently queued for tracing (includes
	 * the rays queued with the call, so e.g. queuing the first five rays will
	 * return 5, the second five rays will return 10, etc.).
	 */
	uint32 queue_rays(const Array<RayInter *> &rayinters_);

	/**
	 * Traces all queued rays, and returns the number of rays traced.
	 */
	uint32 trace_rays();

private:
	/**
	 * Accumulates potential intersections into the potential_inters buffer.
	 * The buffer is sized appropriately and sorted by the time this method
	 * finished.
	 *
	 * @returns The total number of potential intersections accumulated.
	 */
	uint_i accumulate_potential_intersections();

	// Thread helper for accumulate_potential_intersections()
	void accumulation_consumer(JobQueue<IndexRange> *job_queue);

	/**
	 * Sorts the accumulated potential intersections by primitive
	 * id, and creates a table of the starting index and number of
	 * potential intersections for each primitive.
	 */
	void sort_potential_intersections();

	/**
	 * Traces all of the potential intersections in the potential_inters buffer.
	 * This method assumes the the buffer is properly sorted by object id,
	 * and that it is sized properly so that there are no empty potential
	 * intersections.
	 */
	void trace_potential_intersections();

	// Thread helper
	void trace_potints_consumer(JobQueue<PotintJob> *job_queue);
};

#endif // TRACER_HPP
