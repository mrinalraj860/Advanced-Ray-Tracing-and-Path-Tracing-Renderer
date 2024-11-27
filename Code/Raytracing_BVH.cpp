#include "json/include/nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "Camera.hpp"
#include "Cylinder.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Hitable.hpp"
#include "HitRecord.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "utility.hpp"
#include "BVH.hpp"
#include <chrono>

using Color = Vec3;
using json = nlohmann::json;

Camera parseCamera(const json &j)
{
    auto cam_data = j["camera"];
    return Camera(Vec3(cam_data["position"]),
                  Vec3(cam_data["lookAt"]),
                  Vec3(cam_data["upVector"]),
                  cam_data["fov"].get<float>(),
                  static_cast<float>(cam_data["width"].get<int>()) / cam_data["height"].get<int>(),
                  cam_data["exposure"].get<float>(),
                  cam_data["width"].get<int>(),
                  cam_data["height"].get<int>());
}

void parseLights(const json &j, std::vector<Light> &lights)
{
    if (j["scene"].contains("lightsources"))
    {
        for (const auto &light : j["scene"]["lightsources"])
        {
            Vec3 position(light["position"]);
            Color intensity(light["intensity"]);
            lights.emplace_back(position, intensity);
        }
    }
}

void parseScene(const json &j, std::vector<std::shared_ptr<Hittable>> &objects)
{
    for (const auto &obj : j["scene"]["shapes"])
    {
        std::shared_ptr<Material> material = std::make_shared<Diffuse>(Vec3(1, 0, 0));

        if (obj.contains("material"))
        {
            const auto &mat_json = obj["material"];
            if (mat_json.contains("isrefractive") && mat_json["isrefractive"].get<bool>())
            {
                material = std::make_shared<Dielectric>(obj["material"]);
            }
            else if (mat_json.contains("isreflective") && mat_json["isreflective"].get<bool>())
            {
                material = std::make_shared<Metal>(obj["material"]);
            }
            else
            {
                material = std::make_shared<Diffuse>(obj["material"]);
            }
        }

        if (obj["type"] == "sphere")
        {
            objects.push_back(std::make_shared<Sphere>(
                Vec3(obj["center"]),
                obj["radius"].get<float>(),
                material));
        }
        else if (obj["type"] == "cylinder")
        {
            objects.push_back(std::make_shared<Cylinder>(
                Vec3(obj["center"]),
                Vec3(obj["axis"]),
                obj["radius"].get<float>(),
                obj["height"].get<float>(),
                material));
        }
        else if (obj["type"] == "triangle")
        {
            objects.push_back(std::make_shared<Triangle>(
                Vec3(obj["v0"]),
                Vec3(obj["v1"]),
                Vec3(obj["v2"]),
                material));
        }
    }
}

Color lerp(const Color &a, const Color &b, float t)
{
    return a * (1 - t) + b * t;
}

Color reinhardToneMapping(const Color &color, float exposure)
{
    Color mapped = color * exposure;
    return Color(
        mapped.x / (1.0f + mapped.x),
        mapped.y / (1.0f + mapped.y),
        mapped.z / (1.0f + mapped.z));
}

void write_color_withGamma(std::ostream &out, Color pixel_color, int samples_per_pixel, float exposure)
{
    Color scaled_color = pixel_color / samples_per_pixel;
    Color tone_mapped_color = reinhardToneMapping(scaled_color, exposure);

    float gamma_correction = 1.0f / 2.2f;
    auto r = pow(tone_mapped_color.x, gamma_correction);
    auto g = pow(tone_mapped_color.y, gamma_correction);
    auto b = pow(tone_mapped_color.z, gamma_correction);

    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

Color blinn_phong_shading(const Vec3 &view_dir, const Vec3 &light_dir, const Vec3 &normal, const Material &material, const Color &light_intensity)
{
    // Increase ambient factor for better vibrancy
    Color ambient = 0.2 * material.diffusecolor;

    // Diffuse component
    float diff = std::max(0.0f, normal.dot(light_dir));
    Color diffuse = diff * material.kd * material.diffusecolor * light_intensity;

    // Specular component
    Vec3 halfway_dir = (view_dir + light_dir).normalized();
    float spec = std::pow(std::max(0.0f, normal.dot(halfway_dir)), material.specularexponent);
    Color specular = spec * material.ks * material.specularcolor * light_intensity;

    return ambient + diffuse + specular;
}

Color rayColor_Phong(const Ray &r, const BVHNode &world, const std::vector<Light> &lights, const Color &background_color, int depth)
{
    if (depth <= 0)
        return Color(0, 0, 0);

    Hit_record rec;
    if (world.hit(r, 0.001, inf, rec))
    {
        Color lighting(0, 0, 0);
        Vec3 view_dir = -r.direction.normalized();

        for (const auto &light : lights)
        {
            Vec3 light_dir = (light.position - rec.p).normalized();
            Ray shadow_ray(rec.p, light_dir);
            Hit_record shadow_rec;

            if (!world.hit(shadow_ray, 0.001, (light.position - rec.p).length(), shadow_rec))
            {
                lighting += blinn_phong_shading(view_dir, light_dir, rec.normal, *rec.mat_ptr, light.intensity);
            }
        }

        // Reflection handling for reflective materials
        if (rec.mat_ptr->isreflective && depth > 0)
        {
            Vec3 reflected_dir = reflect(r.direction.normalized(), rec.normal);
            Ray reflected_ray(rec.p, reflected_dir);

            float cos_theta = std::max(-reflected_dir.dot(rec.normal), 0.0f);
            float fresnel = rec.mat_ptr->reflectivity + (1.0f - rec.mat_ptr->reflectivity) * std::pow(1.0f - cos_theta, 5);

            Color reflected_color = rayColor_Phong(reflected_ray, world, lights, background_color, depth - 1);
            lighting = lerp(lighting, reflected_color, fresnel);
        }

        return lighting;
    }

    return background_color;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Please provide a JSON file as input." << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    json j;
    file >> j;

    Camera camera = parseCamera(j);
    Color background_color = j["scene"].contains("backgroundcolor") ? Color(j["scene"]["backgroundcolor"]) : Color(0.25, 0.25, 0.25);

    // Parse lights
    std::vector<Light> lights;
    parseLights(j, lights);

    // Parse objects into a list and build BVH tree from it
    std::vector<std::shared_ptr<Hittable>> objects;
    parseScene(j, objects);

    // Initialize BVH from objects
    BVHNode bvh_tree(objects, 0, objects.size(), 0.0, 0);

    int width = j["camera"]["width"];
    int height = j["camera"]["height"];
    int samples_per_pixel = 10;
    int max_depth = 5;
    std::vector<Color> framebuffer(width * height);

    auto start = std::chrono::high_resolution_clock::now();

    // Render loop

    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            Color pixel_color(0, 0, 0);

            // Multiple samples for anti-aliasing
            for (int s = 0; s < samples_per_pixel; ++s)
            {
                float u = (x + random_double()) / (width - 1);
                float v = (y + random_double()) / (height - 1);
                Ray ray = camera.get_ray(u, v);

                pixel_color += rayColor_Phong(ray, bvh_tree, lights, background_color, max_depth);
            }

            framebuffer[y * width + x] = pixel_color;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Render Time: " << elapsed.count() << " seconds\n";

    std::ofstream out(argv[2]);
    out << "P3\n"
        << width << ' ' << height << "\n255\n";
    for (const Color &color : framebuffer)
    {

        write_color_withGamma(out, color, samples_per_pixel, j["camera"]["exposure"]);
    }
    out.close();

    std::cout << "Rendering complete. Image saved to 'output.ppm'" << std::endl;
    return 0;
}