#ifndef INTERSECTION_HPP
#define INTERSECTION_HPP

#include "numtype.h"

#include <limits>
#include <memory>

#include "instance_id.hpp"
#include "transform.hpp"
#include "vector.hpp"
#include "color.hpp"
#include "surface_closure.hpp"
#include "closure_union.hpp"
#include "differential_geometry.hpp"

#define DIFFERENTIAL_DOT_EPSILON 0.0000f

/*
 * Contains the information from a ray intersection.
 */
struct Intersection {
	// Whether there's a hit or not
	bool hit {false};

	// The GUID of the object instance that was hit
	InstanceID id;

	// Information about the intersection point
	float t {std::numeric_limits<float>::infinity()}; // T-parameter along the ray at the intersection
	bool backfacing {false}; // Whether it hit the backface of the surface
	float light_pdf {9999.0f};  // Pdf of selecting this hit point and ray via light sampling

	// The space that the intersection took place in, relative to world space.
	Transform space;

	// Differential geometry at the hit point
	DifferentialGeometry geo;

	// Offset for subsequent spawned rays to avoid self-intersection
	// Should be added for reflection, subtracted for transmission
	Vec3 offset {0.0f, 0.0f, 0.0f};

	// The surface closure at the intersection, along with the probability
	// of that closure having been selected amongst multuple possible
	// closures.
	SurfaceClosureUnion surface_closure;
	float closure_prob {1.0f};
};

#endif
