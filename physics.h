#ifndef PHYSICS_H
#define PHYSICS_H

#include "zmath.h"

namespace Physics {
    // * ======================
    // * Primitives Needed
    // * ======================

    class Ray2D {
        public:
            ZMath::Vec2D origin;
            ZMath::Vec2D dir; // normalized direction of the ray

            /**
             * @brief Construct a new Ray 2D object.
             * 
             * @param origin The origin of the ray.
             * @param dir The direction of the ray as a normalized vector.
             */
            Ray2D(const ZMath::Vec2D &origin, const ZMath::Vec2D &dir) : origin(origin), dir(dir) {};
    };

    class AABB {
        private:
            ZMath::Vec2D halfsize; // half the size of the AABB

        public:
            ZMath::Vec2D pos; // center of the AABB

            /**
             * @brief Create an unrotated rectangle.
             * 
             * @param min The min vertex of the AABB.
             * @param max The max vertex of the AABB.
             */
            AABB(const ZMath::Vec2D &min, const ZMath::Vec2D &max) : halfsize((max - min) * 0.5f), pos(min + halfsize) {};

            ZMath::Vec2D getMin() const { return pos - halfsize; };
            ZMath::Vec2D getMax() const { return pos + halfsize; };
            ZMath::Vec2D getHalfSize() const { return halfsize; };

            // Get the vertices of the AABB.
            // Remember to call delete[] afterwards to free the memory.
            ZMath::Vec2D* getVertices() const {
                ZMath::Vec2D* v = new ZMath::Vec2D[4];

                v[0] = pos - halfsize;
                v[1] = ZMath::Vec2D(pos.x - halfsize.x, pos.y + halfsize.y);
                v[2] = ZMath::Vec2D(pos.x + halfsize.x, pos.y - halfsize.y);
                v[3] = pos + halfsize;

                return v;
            };
    };


    // * ==================
    // * Raycasting
    // * ==================

    /**
     * @brief Determine if a ray intersects an AABB.
     * 
     * @param ray 2D ray.
     * @param aabb 2D AABB.
     * @param dist Float to be modified to equal the distance from the ray until it hits the AABB. Will be set to -1 if no intersection.
     * @param yAxis Bool to be modified to determine the axis of intersection. 1 = on the AABB's y-axis, 0 = x-axis. Junk value if no intersection.
     * @return Is there an intersection? 1 = yes, 0 = no.
     */
    bool raycast(const Ray2D &ray, const AABB &aabb, float &dist, bool &yAxis) {
        // ? We can determine the distance from the ray to a certain edge by dividing a select min or max vector component
        // ?  by the corresponding component from the unit directional vector.
        // ? We know if tMin > tMax, then we have no intersection and if tMax is negative the AABB is behind us and we do not have a hit.

        ZMath::Vec2D dirFrac(1.0f/ray.dir.x, 1.0f/ray.dir.y);
        ZMath::Vec2D min = aabb.getMin(), max = aabb.getMax();

        float t1 = (min.x - ray.origin.x)*dirFrac.x;
        float t2 = (max.x - ray.origin.x)*dirFrac.x;
        float t3 = (min.y - ray.origin.y)*dirFrac.y;
        float t4 = (max.y - ray.origin.y)*dirFrac.y;

        // tMin is the max of the mins and tMax is the min of the maxes
        float tMin = ZMath::max(ZMath::min(t1, t2), ZMath::min(t3, t4));
        float tMax = ZMath::min(ZMath::max(t1, t2), ZMath::max(t3, t4));

        // if tMax < 0 the ray is intersecting behind it. Therefore, we do not actually have a collision.
        if (tMax < 0) {
            dist = -1.0f;
            return 0;
        }

        // ray doesn't intersect the AABB.
        if (tMax < tMin) {
            dist = -1.0f;
            return 0;
        }

        // ray's origin is inside of the AABB.
        // We do not want collisions to occur inside of the AABB so we will return 0.
        if (tMin < 0) {
            dist = -1.0f;
            return 0;
        }

        yAxis = tMin == t3 || tMin == t4;

        dist = tMin;
        return 1;
    };

    // todo add circle vs AABB as that might be better than the raycasting method
}

#endif // !PHYSICS_H
