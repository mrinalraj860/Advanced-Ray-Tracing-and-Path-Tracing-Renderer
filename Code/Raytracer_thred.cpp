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
#include <chrono>
#include <thread>
#include <future>

using Color = Vec3;
using json = nlohmann::json;

Color linearToneMapping(const Color &color, float exposure)
{
    Color mapped = color * exposure; // Scale based on exposure

    // Clamp to [0, 1] to avoid out-of-range values
    return Color(
        fmin(mapped.x, 1.0f),
        fmin(mapped.y, 1.0f),
        fmin(mapped.z, 1.0f));
}

Color reinhardToneMapping(const Color &color, float exposure)
{
    Color mapped = color * exposure; // Scale based on exposure

    // Apply Reinhard tone mapping
    return Color(
        mapped.x / (1.0f + mapped.x),
        mapped.y / (1.0f + mapped.y),
        mapped.z / (1.0f + mapped.z));
}

void write_color_withGamma(std::ostream &out, Color pixel_color, int samples_per_pixel, float exposure)
{
    Color scaled_color = pixel_color / samples_per_pixel;

    Color tone_mapped_color = linearToneMapping(scaled_color, exposure);

    // Divide the color total by the number of samples and gamma correct.
    float gamma_correction = 1.0f / 2.2f;
    auto r = pow(tone_mapped_color.x, gamma_correction);
    auto g = pow(tone_mapped_color.y, gamma_correction);
    auto b = pow(tone_mapped_color.z, gamma_correction);

    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

void write_color(std::ostream &out, Color pixel_color, int samples_per_pixel, float exposure)
{
    auto r = pixel_color.x;
    auto g = pixel_color.y;
    auto b = pixel_color.z;

    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

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

void parseScene(const json &j, hittable_list &world)
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
        else
        {
            // No material specified, use the default material
            material = std::make_shared<Diffuse>(Vec3(1, 0, 0)); // Red Diffuse material
        }

        if (obj.contains("type") && obj["type"] == "sphere" && obj.contains("center") && obj.contains("radius"))
        {
            world.add(std::make_shared<Sphere>(
                Vec3(obj["center"]),
                obj["radius"].get<float>(),
                material));
        }
        else if (obj.contains("type") && obj["type"] == "cylinder" && obj.contains("center") && obj.contains("axis") &&
                 obj.contains("radius") && obj.contains("height"))
        {
            world.add(std::make_shared<Cylinder>(
                Vec3(obj["center"]),
                Vec3(obj["axis"]),
                obj["radius"].get<float>(),
                obj["height"].get<float>(),
                material));
        }
        else if (obj.contains("type") && obj["type"] == "triangle" &&
                 obj.contains("v0") && obj.contains("v1") && obj.contains("v2"))
        {

            world.add(std::make_shared<Triangle>(
                Vec3(obj["v0"]),
                Vec3(obj["v1"]),
                Vec3(obj["v2"]),
                material));
        }
    }
}

Color Binary_Ray_Color(const Ray &r, const hittable_list &world, const Color &background_color)
{
    Hit_record rec;
    if (world.hit(r, 0.001, inf, rec))
    {
        Color lighting(1, 0, 0);
        return lighting;
    }

    return background_color;
}
Color blinn_phong_shading(const Vec3 &view_dir, const Vec3 &light_dir, const Vec3 &normal, const Material &material, const Color &light_intensity)
{
    // Ambient component
    Color ambient = 0.1 * material.diffusecolor; // Adjust ambient factor as needed

    // Diffuse component
    float diff = std::max(0.0f, normal.dot(light_dir));
    Color diffuse = diff * material.kd * material.diffusecolor * light_intensity;

    // Specular component
    Vec3 halfway_dir = (view_dir + light_dir).normalized();
    float spec = std::pow(std::max(0.0f, normal.dot(halfway_dir)), material.specularexponent);
    Color specular = spec * material.ks * material.specularcolor * light_intensity;

    // Combine all components
    return ambient + diffuse + specular;
}

// Function to render a range of rows

// Color rayColor_Phong(const Ray &r, const hittable_list &world, const std::vector<Light> &lights, const Color &background_color, int depth)
// {
//     if (depth <= 0)
//         return Color(0, 0, 0);

//     Hit_record rec;
//     if (world.hit(r, 0.001, inf, rec))
//     {
//         Color lighting(0, 0, 0);
//         Vec3 view_dir = -r.direction.normalized();

//         for (const auto &light : lights)
//         {
//             Vec3 light_dir = (light.position - rec.p).normalized();
//             Ray shadow_ray(rec.p, light_dir);
//             Hit_record shadow_rec;

//             if (!world.hit(shadow_ray, 0.001, (light.position - rec.p).length(), shadow_rec))
//             {
//                 lighting += blinn_phong_shading(view_dir, light_dir, rec.normal, *rec.mat_ptr, light.intensity);
//             }
//         }

//         return lighting;
//     }

//     // Background color
//     return background_color;
// }

Color lerp(const Color &a, const Color &b, float t)
{
    return a * (1 - t) + b * t;
}

Color rayColor_Phong(const Ray &r, const hittable_list &world, const std::vector<Light> &lights,
                     const Color &background_color, int depth)
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
                // Use the blinn_phong_shading function for each light
                lighting += blinn_phong_shading(view_dir, light_dir, rec.normal, *rec.mat_ptr, light.intensity);
            }
        }

        // Reflection handling
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

Color rayColor(const Ray &r, const hittable_list &world, const std::vector<Light> &lights,
               const Color &background_color, int depth)
{
    if (depth <= 0)
        return Color(0, 0, 0);

    Hit_record rec;
    if (world.hit(r, 0.001, inf, rec))
    {
        Color lighting(0, 0, 0);

        // Loop over each light source for diffuse and specular calculation
        for (const auto &light : lights)
        {
            Vec3 light_dir = (light.position - rec.p).normalized();
            Ray shadow_ray(rec.p, light_dir);

            // Shadow check
            Hit_record shadow_rec;
            if (!world.hit(shadow_ray, 0.001, (light.position - rec.p).length(), shadow_rec))
            {
                // Lambertian (Diffuse) Shading
                float diff = fmax(0.0, rec.normal.dot(light_dir));
                Color diffuse = diff * rec.mat_ptr->kd * rec.mat_ptr->diffusecolor * light.intensity;

                // Blinn-Phong Specular Shading
                Vec3 view_dir = -r.direction.normalized();
                Vec3 halfway_dir = (light_dir + view_dir).normalized();
                float spec_angle = fmax(0.0, rec.normal.dot(halfway_dir));
                float spec = pow(spec_angle, rec.mat_ptr->specularexponent);
                Color specular = spec * rec.mat_ptr->ks * rec.mat_ptr->specularcolor * light.intensity;

                lighting += diffuse + specular;
            }
        }

        // Add ambient lighting
        Color ambient(0.25, 0.25, 0.25); // This can be tweaked or passed as a scene parameter
        lighting += ambient * rec.mat_ptr->diffusecolor;

        // Reflection with Fresnel Effect for reflective materials
        if (rec.mat_ptr->isreflective && depth > 0)
        {
            Vec3 reflected_dir = reflect(r.direction.normalized(), rec.normal);
            Ray reflected_ray(rec.p, reflected_dir);

            // Schlick's approximation for Fresnel reflectance
            float cos_theta = fmax(-reflected_dir.dot(rec.normal), 0.0);
            float fresnel = rec.mat_ptr->reflectivity + (1.0 - rec.mat_ptr->reflectivity) * pow(1.0 - cos_theta, 5);

            // Obtain reflection color by tracing the reflected ray
            Color reflected_color = rayColor(reflected_ray, world, lights, background_color, depth - 1);

            // Modulate reflection by Fresnel term
            lighting = lerp(lighting, reflected_color, fresnel);
        }

        return lighting;
    }

    return background_color;
}

void render_rows(int start_y, int end_y, std::vector<Color> &framebuffer, Camera &camera, const hittable_list &world,
                 const std::vector<Light> &lights, const Color &background_color, int width, int height,
                 int samples_per_pixel, int max_depth, int TraceType)
{
    for (int y = start_y; y < end_y; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s)
            {
                // std::cout << "x : " << x << " y : " << y << " s : " << s << std::endl;
                float u = (x + random_double()) / (width - 1);
                float v = (y + random_double()) / (height - 1);
                Ray ray = camera.get_ray(u, v);

                if (TraceType == 1)
                {
                    pixel_color += Binary_Ray_Color(ray, world, background_color);
                }
                else if (TraceType == 2)
                {
                    pixel_color += rayColor_Phong(ray, world, lights, background_color, max_depth);
                }
                else
                {
                    pixel_color += rayColor(ray, world, lights, background_color, max_depth);
                }
            }
            framebuffer[y * width + x] = pixel_color;
        }
    }
}

std::future<Camera> async_parseCamera(const json &j)
{
    return std::async(std::launch::async, parseCamera, j);
}

std::future<hittable_list> async_parseScene(const json &j)
{
    return std::async(std::launch::async, [](const json &j)
                      {
        hittable_list world;
        parseScene(j, world);
        return world; }, j);
}

std::future<std::vector<Light>> async_parseLights(const json &j)
{
    return std::async(std::launch::async, [](const json &j)
                      {
        std::vector<Light> lights;
        parseLights(j, lights);
        return lights; }, j);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Please provide a JSON file as input." << std::endl;
        return 1;
    }
    std::cout << argv[2] << std::endl;

    std::ifstream file(argv[1]);
    json j;
    file >> j;

    auto camera_future = async_parseCamera(j);
    auto scene_future = async_parseScene(j);
    auto lights_future = async_parseLights(j);

    // Retrieve results from futures
    Camera camera = camera_future.get();
    hittable_list world = scene_future.get();
    std::vector<Light> lights = lights_future.get();

    Color background_color = j["scene"].contains("backgroundcolor") ? Color(j["scene"]["backgroundcolor"]) : Color(0.25, 0.25, 0.25);

    // hittable_list world;
    // parseScene(j, world);
    std::cout << "Press 1 for binary Trace, 3 for Normal and 2 for Bling Phong effect: ";
    int TraceType;
    std::cin >> TraceType;

    int width = j["camera"]["width"];
    int height = j["camera"]["height"];
    int samples_per_pixel = 10;
    int max_depth = 5;
    std::vector<Color> framebuffer(width * height);
    int num_threads = std::thread::hardware_concurrency();
    int rows_per_thread = height / num_threads;
    std::cout << "Num of Threads : " << num_threads << " Rows per thread: " << rows_per_thread << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    // Launch threads
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        int start_y = i * rows_per_thread;
        int end_y = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;
        threads.emplace_back(render_rows, start_y, end_y, std::ref(framebuffer), std::ref(camera), std::cref(world),
                             std::cref(lights), background_color, width, height, samples_per_pixel, max_depth, TraceType);
    }

    // Join threads
    for (auto &thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Render Time: " << elapsed.count() << " seconds\n";

    std::ofstream out1(argv[3]);
    out1 << "P3\n"
         << width << ' ' << height << "\n255\n";

    for (const Color &color : framebuffer)
    {
        write_color_withGamma(out1, color, samples_per_pixel, j["camera"]["exposure"]);
    }
    out1.close();

    std::ofstream out(argv[2]);
    out << "P3\n"
        << width << ' ' << height << "\n255\n";

    for (const Color &color : framebuffer)
    {
        write_color(out, color, samples_per_pixel, j["camera"]["exposure"]);
    }
    out.close();

    std::cout << "Rendering complete. Image saved to 'output.ppm'" << std::endl;
    return 0;
}