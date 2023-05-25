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

    class Circle {
        public:
            ZMath::Vec2D c; // center
            float r; // radius

            Circle() = default; // to make the compiler happy (and make the program faster)

            Circle(ZMath::Vec2D const &center, float radius) : c(center), r(radius) {};
    };

    class AABB {
        private:
            ZMath::Vec2D halfsize; // half the size of the AABB

        public:
            ZMath::Vec2D pos; // center of the AABB

            AABB() = default; // to make the compiler happy

            /**
             * @brief Create an unrotated rectangle.
             * 
             * @param min The min vertex of the AABB.
             * @param max The max vertex of the AABB.
             */
            AABB(const ZMath::Vec2D &min, const ZMath::Vec2D &max) : halfsize((max - min) * 0.5f), pos(min + halfsize) {};

            ZMath::Vec2D getMin() const { return pos - halfsize; };
            ZMath::Vec2D getMax() const { return pos + halfsize; };
            ZMath::Vec2D getHalfsize() const { return halfsize; };

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

    class Box2D {
        private:
            ZMath::Vec2D halfsize; // half the size of the Box2D

        public:
            ZMath::Vec2D pos;
            float theta;
            
            // Rotation matrix of the Box2D.
            // Rotate anything from this box's local space to global space.
            // Cached for efficiency.
            // todo test to make sure it's local to global and not the other way around.
            ZMath::Mat2D rot;

            Box2D() = default;

            /**
             * @brief Create a rotated rectangle.
             * 
             * @param min 
             * @param max 
             * @param theta 
             */
            Box2D(ZMath::Vec2D const &min, ZMath::Vec2D const &max, float theta) : 
                    halfsize((max - min) * 0.5f), pos(min + halfsize), theta(theta) { rot = ZMath::Mat2D::rotationMat(theta); };

            ZMath::Vec2D getLocalMin() const { return pos - halfsize; };
            ZMath::Vec2D getLocalMax() const { return pos + halfsize; };
            ZMath::Vec2D getHalfsize() const { return halfsize; };

            // Get the vertices of the Box2D.
            // Remember to call delete[] on it.
            ZMath::Vec2D* getVertices() const {
                ZMath::Vec2D* v = new ZMath::Vec2D[4];

                v[0] = -halfsize;
                v[1] = ZMath::Vec2D(-halfsize.x, halfsize.y);
                v[2] = ZMath::Vec2D(halfsize.x, -halfsize.y);
                v[3] = halfsize;

                for (int i = 0; i < 4; ++i) { v[i] = rot * v[i] + pos; }

                return v;
            };
    };


    // * ===========================
    // * Intersection Detection
    // * ===========================

    bool CircleAndCircle(const Circle &c1, const Circle &c2) {
        float r = c1.r + c2.r;
        return c1.c.distSq(c2.c) <= r*r;
    };

    bool CircleAndAABB(const Circle &c, const AABB &a) {
        // ? Determine the closest point of the AABB to the Circle and check if its distance to the center is less than the radius.

        ZMath::Vec2D closest = c.c;
        ZMath::Vec2D min = a.getMin(), max = a.getMax();

        closest = ZMath::clamp(closest, min, max);
        return closest.distSq(c.c) <= c.r*c.r;
    };

    // Normal points away from A towards B.
    // Normal will be a junk value if no collision occurs.
    bool CircleAndAABB(const Circle &c, const AABB &a, ZMath::Vec2D &normal) {
        // ? Determine the closest point of the AABB to the Circle and check if its distance to the center is less than the radius.

        ZMath::Vec2D closest = c.c;
        ZMath::Vec2D min = a.getMin(), max = a.getMax();

        closest = ZMath::clamp(closest, min, max);
        ZMath::Vec2D diff = closest - c.c;

        if (diff.magSq() > c.r*c.r) { return 0; }

        normal = diff.normalize();
        return 1;
    };

    bool CircleAndBox2D(const Circle &c, const Box2D &b) {
        // ? Same as CircleAndAABB except we first rotate the circle into the box's local space.

        ZMath::Vec2D closest = c.c - b.pos;
        ZMath::Vec2D min = b.getLocalMin(), max = b.getLocalMax();

        closest = b.rot.transpose() * closest + b.pos;
        closest = ZMath::clamp(closest, min, max);

        return closest.distSq(c.c) <= c.r*c.r;
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

        yAxis = tMin == t1 || tMin == t2;

        dist = tMin;
        return 1;
    };

    // todo add circle vs AABB as that might be better than the raycasting method
}

#endif // !PHYSICS_H
