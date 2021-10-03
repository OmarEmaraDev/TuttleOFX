// Copyright 2008 Chung-Lin Wen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/*************************************************************************************************/

#ifndef GIL_LAB_H
#define GIL_LAB_H

////////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Support for CIE Lab color space
/// \author Chung-Lin Wen \n
////////////////////////////////////////////////////////////////////////////////////////

#include <boost/cast.hpp>
#include <boost/gil.hpp>

namespace boost
{
namespace gil
{

/// \addtogroup ColorNameModel
/// \{
namespace lab_color_space
{
/// \brief Luminance
struct luminance_t
{
};
/// \brief a Color Component
struct a_color_opponent_t
{
};
/// \brief b Color Component
struct b_color_opponent_t
{
};
}
/// \}

/// \ingroup ColorSpaceModel
typedef mpl::vector3<lab_color_space::luminance_t, lab_color_space::a_color_opponent_t, lab_color_space::b_color_opponent_t>
    lab_t;

/// \ingroup LayoutModel
typedef layout<lab_t> lab_layout_t;

GIL_DEFINE_ALL_TYPEDEFS(32f, lab);

/// \ingroup ColorConvert
/// \brief RGB to LAB
template <>
struct default_color_converter_impl<rgb_t, lab_t>
{
    template <typename P1, typename P2>
    void operator()(const P1& src, P2& dst) const
    {
        using namespace lab_color_space;

        // only float32_t for lab is supported
        float32_t temp_red = channel_convert<float32_t>(get_color(src, red_t()));
        float32_t temp_green = channel_convert<float32_t>(get_color(src, green_t()));
        float32_t temp_blue = channel_convert<float32_t>(get_color(src, blue_t()));

        // first, transfer to xyz color space
        float32_t normalized_r = temp_red / 255.f;
        float32_t normalized_g = temp_green / 255.f;
        float32_t normalized_b = temp_blue / 255.f;

        if(normalized_r > 0.04045f)
        {
            normalized_r = pow(((normalized_r + 0.055f) / 1.055f), 2.4f);
        }
        else
        {
            normalized_r /= 12.92f;
        }

        if(normalized_g > 0.04045f)
        {
            normalized_g = pow(((normalized_g + 0.055f) / 1.055f), 2.4f);
        }
        else
        {
            normalized_g /= 12.92f;
        }

        if(normalized_b > 0.04045f)
        {
            normalized_b = pow(((normalized_b + 0.055f) / 1.055f), 2.4f);
        }
        else
        {
            normalized_b /= 12.92f;
        }

        normalized_r *= 100.f;
        normalized_g *= 100.f;
        normalized_b *= 100.f;

        float32_t x, y, z;
        x = normalized_r * 0.4124f + normalized_g * 0.3576f + normalized_b * 0.1805f;
        y = normalized_r * 0.2126f + normalized_g * 0.7152f + normalized_b * 0.0722f;
        z = normalized_r * 0.0193f + normalized_g * 0.1192f + normalized_b * 0.9505f;

        // then, transfer to lab color space
        float32_t ref_x = 95.047f;
        float32_t ref_y = 100.000f;
        float32_t ref_z = 108.883f;
        float32_t normalized_x = x / ref_x;
        float32_t normalized_y = y / ref_y;
        float32_t normalized_z = z / ref_z;

        if(normalized_x > 0.008856f)
        {
            normalized_x = pow(normalized_x, 0.333f);
        }
        else
        {
            normalized_x = (7.787f * normalized_x) + (16.f / 116.f);
        }

        if(normalized_y > 0.008856f)
        {
            normalized_y = pow(normalized_y, 0.333f);
        }
        else
        {
            normalized_y = (7.787f * normalized_y) + (16.f / 116.f);
        }

        if(normalized_z > 0.008856f)
        {
            normalized_z = pow(normalized_z, 0.333f);
        }
        else
        {
            normalized_z = (7.787f * normalized_z) + (16.f / 116.f);
        }

        float32_t luminance, a_color_opponent, b_color_opponent;
        luminance = (116.f * normalized_y) - 16.f;
        a_color_opponent = 500.f * (normalized_x - normalized_y);
        b_color_opponent = 200.f * (normalized_y - normalized_z);

        get_color(dst, luminance_t()) = luminance;
        get_color(dst, a_color_opponent_t()) = a_color_opponent;
        get_color(dst, b_color_opponent_t()) = b_color_opponent;
    }
};

/// \ingroup ColorConvert
/// \brief LAB to RGB
template <>
struct default_color_converter_impl<lab_t, rgb_t>
{
    template <typename P1, typename P2>
    void operator()(const P1& src, P2& dst) const
    {
        using namespace lab_color_space;

        float32_t luminance = get_color(src, luminance_t());
        float32_t a_color_opponent = get_color(src, a_color_opponent_t());
        float32_t b_color_opponent = get_color(src, b_color_opponent_t());

        // first, transfer to xyz color space
        float32_t normalized_y = (luminance + 16.f) / 116.f;
        float32_t normalized_x = (a_color_opponent / 500.f) + normalized_y;
        float32_t normalized_z = normalized_y - (b_color_opponent / 200.f);

        if(pow(normalized_y, 3.f) > 0.008856f)
        {
            normalized_y = pow(normalized_y, 3.f);
        }
        else
        {
            normalized_y = (normalized_y - 16.f / 116.f) / 7.787f;
        }

        if(pow(normalized_x, 3.f) > 0.008856f)
        {
            normalized_x = pow(normalized_x, 3.f);
        }
        else
        {
            normalized_x = (normalized_x - 16.f / 116.f) / 7.787f;
        }

        if(pow(normalized_z, 3.f) > 0.008856f)
        {
            normalized_z = pow(normalized_z, 3.f);
        }
        else
        {
            normalized_z = (normalized_z - 16.f / 116.f) / 7.787f;
        }

        float32_t reference_x = 95.047f;
        float32_t reference_y = 100.000f;
        float32_t reference_z = 108.883f;
        float32_t x, y, z;
        x = reference_x * normalized_x;
        y = reference_y * normalized_y;
        z = reference_z * normalized_z;

        // then, transfer to rgb color space
        normalized_x = x / 100.f;
        normalized_y = y / 100.f;
        normalized_z = z / 100.f;

        float32_t result_r = normalized_x * 3.2406f + normalized_y * -1.5372f + normalized_z * -0.4986f;
        float32_t result_g = normalized_x * -0.9689f + normalized_y * 1.8758f + normalized_z * 0.0415f;
        float32_t result_b = normalized_x * 0.0557f + normalized_y * -0.2040f + normalized_z * 1.0570f;

        if(result_r > 0.0031308f)
        {
            result_r = 1.055f * pow(result_r, 1.f / 2.4f) - 0.055f;
        }
        else
        {
            result_r = 12.92f * result_r;
        }

        if(result_g > 0.0031308f)
        {
            result_g = 1.055f * pow(result_g, 1.f / 2.4f) - 0.055f;
        }
        else
        {
            result_g = 12.92f * result_g;
        }

        if(result_b > 0.0031308f)
        {
            result_b = 1.055f * pow(result_b, 1.f / 2.4f) - 0.055f;
        }
        else
        {
            result_b = 12.92f * result_b;
        }

        float32_t red, green, blue;
        red = result_r * 255.f;
        green = result_g * 255.f;
        blue = result_b * 255.f;

        get_color(dst, red_t()) = channel_convert<typename color_element_type<P2, red_t>::type>(red);
        get_color(dst, green_t()) = channel_convert<typename color_element_type<P2, green_t>::type>(green);
        get_color(dst, blue_t()) = channel_convert<typename color_element_type<P2, blue_t>::type>(blue);
    }
};
}
} // namespace boost::gil

#endif // GIL_LAB_H
