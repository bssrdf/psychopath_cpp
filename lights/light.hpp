#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "bbox.hpp"
#include "vector.hpp"
#include "color.hpp"

/**
 * @brief An interface for light sources.
 */
class Light
{
public:
	virtual ~Light() {}

	/**
	 * @brief Samples the light source for a given point to be illuminated.
	 *
	 * @param arr The point to be illuminated.
	 * @param u Random parameter U.
	 * @param v Random parameter V.
	 * @param time The time to sample at.
	 * @param[out] shadow_vec The world-space direction to cast a shadow ray
	 *               for visibility testing.  It's length determines the extent
	 *               that the shadow ray should have, unless the light source
	 *               is infinite (see is_infinite()) in which case the extent
	 *               should be infinite.  This vector also doubles to inform
	 *               What direction the light is arriving from (just invert
	 *               the vector).
	 *
	 * @returns The light arriving at the point arr.
	 */
	virtual Color sample(const Vec3 &arr, float u, float v, float time,
	                     Vec3 *shadow_vec) const = 0;


	/**
	 * @brief Returns the color emitted in the given direction from the
	 * given parameters on the light.
	 *
	 * @param dir The direction of the outgoing light.
	 * @param u Random parameter U.
	 * @param v Random parameter V.
	 * @param time The time to sample at.
	 */
	virtual Color outgoing(const Vec3 &dir, float u, float v, float time) const = 0;


	/**
	 * @brief Returns the color that will arrive at the given point from the
	 * given parameters of the light source.
	 *
	 * This does _not_ account for shadowing at all.  It presumes the point
	 * is fully visible to the light source.
	 *
	 * @param arr The point that the light is arriving at.
	 * @param u Random parameter U.
	 * @param v Random parameter V.
	 * @param time The time to sample at.
	 */
	virtual Color arriving(const Vec3 &arr, float u, float v, float time) const {
		// Default implementation
		Vec3 temp;
		return sample(arr, u, v, time, &temp);
	}


	/**
	 * @brief Returns whether the light has a delta distribution.
	 *
	 * If a light has not chance of a ray hitting it through random process
	 * then it is a delta light source.  For example, point light sources,
	 * lights that only emit in a single direction, etc.
	 */
	virtual bool is_delta() const = 0;


	/**
	 * @brief Returns whether the light is infinite.
	 *
	 * Sun light sources, sky dome lights, etc.  Basically, any light that
	 * cannot be thought of as existing within the 3d scene, and thus where
	 * only the direction of the light matters.
	 */
	virtual bool is_infinite() const = 0;

	virtual float total_energy() const = 0;

	virtual BBox bounds() const = 0;
};

#endif // LIGHT_HPP
