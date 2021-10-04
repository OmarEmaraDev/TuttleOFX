#include <memory>

#include "color.hpp"

namespace tuttle
{
namespace common
{

std::shared_ptr<Color> Color::get()
{
    return color;
}

std::shared_ptr<Color> Color::color(new Color);
}
}
