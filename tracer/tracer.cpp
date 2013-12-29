#include "tracer.hpp"

#include <limits>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <assert.h>

#include "array.hpp"
#include "slice.hpp"
#include "counting_sort.hpp"

#include "ray.hpp"
#include "intersection.hpp"
#include "scene.hpp"

#define RAY_STATE_SIZE scene->world.ray_state_size()
#define MAX_POTINT 2
#define RAY_JOB_SIZE (1024*4)


uint32_t Tracer::trace(const Slice<Ray> rays_, Slice<Intersection> intersections_)
{
	// Get rays
	rays.init_from(rays_);

	// Get and initialize intersections
	intersections.init_from(intersections_);
	std::fill(intersections.begin(), intersections.end(), Intersection());

	// Print number of rays being traced
	//std::cout << "\tTracing " << rays.size() << " rays" << std::endl;

	// Allocate and clear out ray states
	states.resize(rays.size()*RAY_STATE_SIZE);
	std::fill(states.begin(), states.end(), 0);

	// Allocate and init rays_active flags
	rays_active.resize(rays.size());
	std::fill(rays_active.begin(), rays_active.end(), true);

	// Allocate potential intersection buffer
	potential_intersections.resize(rays.size()*MAX_POTINT);

	// Trace potential intersections
	while (accumulate_potential_intersections()) {
		sort_potential_intersections();
		trace_potential_intersections();

		/*
		// Print number of active rays
		size_t active_rays = 0;
		for (auto r: rays_active) {
			if (r)
				active_rays++;
		}
		std::cout << "Active rays: " << active_rays << std::endl;
		*/
	}

	return rays.size();
}


// Job function for accumulating potential intersections,
// for use in accumulate_potential_intersections() below.
void job_accumulate_potential_intersections(Tracer *tracer, size_t start_i, size_t end_i)
{
	size_t potint_ids[MAX_POTINT];

	for (size_t i = start_i; i < end_i; i++) {
		if (tracer->rays_active[i]) {
			const size_t pc = tracer->scene->world.get_potential_intersections(tracer->rays[i], tracer->intersections[i].t, MAX_POTINT, potint_ids, &(tracer->states[i*tracer->RAY_STATE_SIZE]));
			tracer->rays_active[i] = (pc > 0);

			for (size_t j = 0; j < pc; j++) {
				tracer->potential_intersections[(i*MAX_POTINT)+j].valid = true;
				tracer->potential_intersections[(i*MAX_POTINT)+j].object_id = potint_ids[j];
				tracer->potential_intersections[(i*MAX_POTINT)+j].ray_index = i;
			}
		}
	}
}

size_t Tracer::accumulate_potential_intersections()
{
	// Clear out potential intersection buffer
	potential_intersections.resize(rays.size()*MAX_POTINT);
	const size_t spi = potential_intersections.size();
	for (size_t i = 0; i < spi; i++)
		potential_intersections[i].valid = false;


	// Accumulate potential intersections
	for (size_t i = 0; i < rays.size(); i += RAY_JOB_SIZE) {

		// Dole out jobs
		size_t start = i;
		size_t end = i + RAY_JOB_SIZE;
		if (end > rays.size())
			end = rays.size();


		job_accumulate_potential_intersections(this, start, end);

	}


	// Compact the potential intersections
	size_t pii = 0;
	size_t last = 0;
	size_t i = 0;
	while (i < potential_intersections.size()) {
		while (last < potential_intersections.size() && potential_intersections[last].valid)
			last++;

		if (potential_intersections[i].valid) {
			pii++;
			if (i > last) {
				potential_intersections[last] = potential_intersections[i];
				potential_intersections[i].valid = false;
			}
		}

		i++;
	}
	potential_intersections.resize(pii);

	// Return the total number of potential intersections accumulated
	return pii;
}


/**
 * Uses a counting sort to sort the potential intersections by
 * object_id.
 */
void Tracer::sort_potential_intersections()
{
#if 1
	std::sort(potential_intersections.begin(), potential_intersections.end());
#else
	CountingSort::sort<PotentialInter>(&(*(potential_intersections.begin())),
	                                   potential_intersections.size(),
	                                   scene->world.max_primitive_id(),
	                                   &index_potint);
#endif
	return;
}


void job_trace_potential_intersections(Tracer *tracer, size_t start, size_t end)
{
	for (size_t i = start; i < end; i++) {
		// Shorthand references
		PotentialInter &potential_intersection = tracer->potential_intersections[i];
		const Ray& ray = tracer->rays[potential_intersection.ray_index];
		Intersection& intersection = tracer->intersections[potential_intersection.ray_index];
		size_t& id = potential_intersection.object_id;

		// Trace
		if (ray.is_shadow_ray) {
			if (!intersection.hit)
				intersection.hit |= tracer->scene->world.get_primitive(id).intersect_ray(ray, nullptr);
		} else {
			intersection.hit |= tracer->scene->world.get_primitive(id).intersect_ray(ray, &intersection);
		}
	}
}

void Tracer::trace_potential_intersections()
{
#define BLARGYFACE 10000
	for (size_t i = 0; i < potential_intersections.size(); i += BLARGYFACE) {
		// Dole out jobs
		size_t start = i;
		size_t end = i + BLARGYFACE;
		if (end > potential_intersections.size())
			end = potential_intersections.size();

		job_trace_potential_intersections(this, start, end);
	}
}



