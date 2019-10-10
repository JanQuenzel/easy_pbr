#pragma once 

#include <memory>

#include "Shader.h"
#include "GBuffer.h"

#include <Eigen/Geometry>

#include "easy_pbr/Camera.h"
#include "easy_pbr/Mesh.h"

#define CONFIGURU_WITH_EIGEN 1
#define CONFIGURU_IMPLICIT_CONVERSIONS 1
#include <configuru.hpp>


class SpotLight : public std::enable_shared_from_this<SpotLight>, public Camera
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    SpotLight(const configuru::Config& config);

    // void render_to_shadow_map(const MeshCore& mesh);
    void render_mesh_to_shadow_map(MeshGLSharedPtr& mesh);
    void render_points_to_shadow_map(MeshGLSharedPtr& mesh);
    void clear_shadow_map();
    void set_shadow_map_resolution(const int shadow_map_resolution);
    int shadow_map_resolution();
    bool has_shadow_map();
    gl::Texture2D& get_shadow_map_ref();

    // float m_fov_x;
    // float m_fov_y;
    // float m_near;
    // float m_far;
    // Eigen::Vector3f m_pos;

    float m_power;
    Eigen::Vector3f m_color; 
    bool m_create_shadow;

private:

    void init_params(const configuru::Config& config_file);
    void init_opengl();

    // Eigen::Matrix4f get_view_matrix(); // puts the scene or mesh in the coordinate system of the light
    // Eigen::Matrix4f get_proj_matrix(); // projects the mesh from the coordinate system of the light into the light itself

    gl::Shader m_shadow_map_shader;
    gl::GBuffer m_shadow_map_fbo; //fbo that contains only depth maps for usage as a shadow map
    int m_shadow_map_resolution;
  
};
