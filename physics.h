#ifndef PHYSICS_H
#define PHYSICS_H

#include "zmath.h"

namespace Primitives {
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
}

namespace Physics {
    bool raycast(const Ray2D &ray, const AABB &aabb) const;
}

#endif // !PHYSICS_H
