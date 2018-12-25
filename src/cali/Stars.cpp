#include "Stars.h"

#include <ctime>

#include "World.h"
#include "CommonFileSystem.h"
#include "CommonTexture.h"
#include "Constants.h"

namespace cali
{
    namespace utils
    {
        inline float random_float()
        {
            return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }

        inline float random_float(float low, float high)
        {
            return low + static_cast <float>(rand())/(static_cast<float>(RAND_MAX/(high - low)));
        }

        inline IvVector3 get_random_point_on_sphere(const IvVector3& sphere_center, const float sphere_radius)
        {
            IvVector3 point{
                utils::random_float(-1.f, 1.f),
                utils::random_float(-1.f, 1.f),
                utils::random_float(-1.f, 1.f) };

            point.Normalize();

            point *= world::c_star_distance;
            point += world::c_earth_center;

            return point;
        }
    }

    IvVector3 rotate_around_point(const IvVector3& what, const IvVector3& point, const IvVector3& axis, const float angle)
    {
        auto moved_to_origin = what - point;

        IvMatrix33 rotation;
        rotation.Rotation(axis, angle);
        const auto rotated = moved_to_origin * rotation;

        return rotated + point;
    }

    void stars::generate_stars(size_t count)
    {
        srand(static_cast<unsigned int>(time(nullptr)));

        for (size_t i = 0; i < count; ++i)
        {
            star new_star{ 
                utils::get_random_point_on_sphere(
                    world::c_earth_center,
                    world::c_earth_radius),
                world::c_star_visible_size };

            new_star.look_at(world::c_earth_center, constants::c_world_up);

            m_stars.emplace_back(new_star);
        }
    }
    stars::stars()
    {
        std::string vertex_shader_file = construct_shader_path("stars.hlslv");
        std::string pixel_shader_file = construct_shader_path("stars.hlslf");

        auto& renderer = *IvRenderer::mRenderer;
        auto& resman = *renderer.GetResourceManager();

        set_position(cali::world::c_earth_center);

        m_quad = create_quad({ 1.f, 1.f, 1.f }, true, false);

        generate_stars(200);

        m_shader = resman.CreateShaderProgram(
            resman.CreateVertexShaderFromFile(
                vertex_shader_file.c_str(), "main"),
            resman.CreateFragmentShaderFromFile(
                pixel_shader_file.c_str(), "main"));
    }

    stars::~stars()
    {
        auto& renderer = *IvRenderer::mRenderer;
        renderer.GetResourceManager()->Destroy(m_shader);
    }
    void stars::update(float dt)
    {
        for (auto& star : m_stars)
        {
            star.set_position(
                rotate_around_point(
                    star.get_position(),
                    world::c_earth_center,
                    { 1.f, 0.f, 0.f },
                    -0.05f * dt));

            star.look_at(world::c_earth_center, constants::c_world_up);
        }
    }
    void stars::render(IvRenderer & renderer)
    {
        renderer.SetBlendFunc(kOneBlendFunc, kZeroBlendFunc, kAddBlendOp);
        for (const auto& star : m_stars)
        {
            star.set_transformation_matrix(renderer);
            m_quad.render(renderer, m_shader);
        }
    }
}
