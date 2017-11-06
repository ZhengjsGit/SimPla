//
// Created by salmon on 17-11-6.
//

#ifndef SIMPLA_SHELL_H
#define SIMPLA_SHELL_H
#include <memory>
#include "Surface.h"
namespace simpla {
namespace geometry {
struct Shape;
struct Shell : public Surface {
    SP_GEO_OBJECT_HEAD(Shell, Surface)
   public:
    explicit Shell(std::shared_ptr<const Shape> const &);
    std::shared_ptr<const Shape> GetShape() const;

   private:
    std::shared_ptr<const Shape> m_shape_ = nullptr;
};
}  // namespace geometry{
}  // namespace simpla{
#endif  // SIMPLA_SHELL_H