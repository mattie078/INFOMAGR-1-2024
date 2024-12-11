//
// Created by Martin Gano on 3.5.2022.
//

#include "Scene.h"
#include <math.h>
#include <bits/std_thread.h>
#include "globals.h"

using namespace std;

Ray Ray::reflect(const cv::Vec3f &axis_unit_v) const
{
    cv::Vec3f projection = axis_unit_v * direction_unit_v_.dot(axis_unit_v);
    cv::Vec3f step = projection - direction_unit_v_;
    return {origin_c_, direction_unit_v_ + step * 2};
}

Color Scene::rayTracing(Ray ray, vector<cv::Vec3f> &vertexes, int depth)
{
    if (depth <= 0)
    {
        return Color(0, 0, 0);
    }

    Color result(0, 0, 0);

    Triangle *object;
    Material material;
    Intersection intersection;

    bool found;
    if (KDTREE)
    {
        found = KDTree::traverse_tree(kd_tree, ray, &object, &intersection, vertexes);
    }
    else
    {
        found = kd_tree->get_intersection(ray, &object, &intersection, vertexes);
    }
    if (!found)
    {
        cv::Vec3f unit_direction = ray.direction_unit_v_;
        float t = 0.5f * (unit_direction[1] + 1.0f); // Scale Y component to [0, 1]
        Color white(1.0f, 1.0f, 1.0f);
        Color blue(1.0f, 0.7f, 0.5f);         // Convert from RGB to BGR
        return white * (1.0f - t) + blue * t; // Linear interpolation
    }

    material = object->material_;

    // Calculate ambient
    Color ambient = ambient_color_ * material.ambient_k_;

    // Calculate diffuse and specular
    Color diffuse(0, 0, 0);
    Color specular(0, 0, 0);
    cv::Vec3f reflected = -ray.reflect(intersection.normal_unit_v_).direction_unit_v_;
    for (auto light : lights)
    {
        Ray light_ray;
        cv::Vec3f light_origin = light->origin_c_;
        light_ray.origin_c_ = light_origin;
        light_ray.direction_unit_v_ = unit_vec(intersection.contact_c_ - light_origin);
        Intersection light_intersection;
        Triangle *object_temp;

        if (KDTREE)
        {
            found = KDTree::traverse_tree(kd_tree, light_ray, &object_temp, &light_intersection, vertexes);
        }
        else
        {
            found = kd_tree->get_intersection(light_ray, nullptr, &light_intersection, vertexes);
        }

        if (!found || !light_intersection.exists || vec_length(light_intersection.contact_c_ - intersection.contact_c_) > 0.0001)
            continue;

        float falloff = vec_length(intersection.contact_c_ - light_origin);
        falloff *= falloff;

        float diffusion = intersection.normal_unit_v_.dot(unit_vec(light_origin - intersection.contact_c_));
        if (diffusion > 0)
            diffuse += light->color_ * material.diffuse_f_ * material.diffuse_k_ * light->intensity_ * diffusion *
                       (1 / falloff);

        float specularity = reflected.dot(-light_ray.direction_unit_v_);
        if (specularity > 0)
            specular += light->color_ * material.specular_f_ * material.specular_k_ * light->intensity_ *
                        pow(specularity, material.shininess_) * (1 / falloff);
    }

    result = ambient + diffuse + specular;

    // Always assume lambertian reflection for now

    Ray scattered;
    Color attenuation;

    auto random_unit_vector = []() -> cv::Vec3f
    {
        const double pi = 3.1415926535897932385;

        float a = static_cast<float>(rand()) / RAND_MAX * 2.0f * pi;
        float z = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
        float r = sqrt(1.0f - z * z);
        return cv::Vec3f(r * cos(a), r * sin(a), z);
    };

    auto scatter_dir = intersection.normal_unit_v_ + random_unit_vector();
    scattered = {intersection.contact_c_, scatter_dir};
    attenuation = material.diffuse_f_;
    result += attenuation * rayTracing(scattered, vertexes, depth - 1);
    result.clip();

    return result;
}

inline double random_double()
{
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

#include <atomic>
#include <iostream>
#include <iomanip>
#include <queue>

void Scene::render(Image *image, vector<cv::Vec3f> &vertexes, int samples, int depth)
{
    const double pi = 3.1415926535897932385;
    float vof = 40;

    int width = image->getWidth();
    int height = image->getHeight();
    cv::Vec3f normal = unit_vec(camera->view_plane_point_c - camera->origin_c);
    cv::Vec3f up = unit_vec(cv::Vec3f(0, 0, 1).cross(normal));
    if (cv::norm(up) == 0)
    {
        up = cv::Vec3f(0, 1, 0); // Fall back if viewing direction aligns with Z-axis
    }

    // Add vof like BVH branch
    float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    float d = cv::norm(camera->view_plane_point_c - camera->origin_c);
    camera->view_plane_height = 2.0f * d * tan((vof * 0.5f) * (pi / 180.0f));
    camera->view_plane_width = camera->view_plane_height * aspect_ratio;

    float plane_left_length = camera->view_plane_width / (float)width;
    float plane_up_length = camera->view_plane_height / (float)height;

    cv::Vec3f plane_left = up.cross(unit_vec(normal)) * plane_left_length;
    cv::Vec3f plane_right = -plane_left;
    cv::Vec3f plane_up = normal.cross(unit_vec(plane_left)) * plane_up_length;
    cv::Vec3f plane_down = -plane_up;

    // Coordinate
    cv::Vec3f plane_top_left_corner_coord = camera->view_plane_point_c +
                                            (plane_left * static_cast<float>(width) / 2.0f) +
                                            (plane_up * static_cast<float>(height) / 2.0f);

    // cv::Vec3f from camera
    cv::Vec3f plane_top_left_corner = plane_top_left_corner_coord - camera->origin_c;

    std::atomic<int> processed_pixels(0);
    const int total_pixels = width * height;

    // Mutex for work stealing
    std::mutex work_mutex;
    std::queue<std::pair<int, int>> task_queue;

    auto render_pixel = [&](int w, int h)
    {
        float r = 0, g = 0, b = 0;

        for (int s = 0; s < samples; s++)
        {
            D(num_primary_rays++);
            Color pixel_color(0, 0, 0);
            Ray ray;
            ray.origin_c_ = camera->origin_c;
            ray.direction_unit_v_ = unit_vec(plane_top_left_corner +
                                             plane_right * static_cast<float>(w + random_double() / width) +
                                             plane_down * static_cast<float>(h + random_double() / height));

            pixel_color += rayTracing(ray, vertexes, depth);

            r += pixel_color.r();
            g += pixel_color.g();
            b += pixel_color.b();
        }

        // Somehow averaging the color using the class itself will cause disco effect :) WHY? (probably because hidden rounding)
        r /= samples;
        g /= samples;
        b /= samples;

        image->set(w, h, static_cast<unsigned char>(r * 255),
                   static_cast<unsigned char>(g * 255),
                   static_cast<unsigned char>(b * 255),
                   255);

        processed_pixels.fetch_add(1);
    };

    auto worker = [&]()
    {
        while (true)
        {
            std::pair<int, int> task;

            // Lock to fetch task from queue so we dont have multiple threads working on the same task
            {
                std::lock_guard<std::mutex> lock(work_mutex);
                if (task_queue.empty())
                    break; // = no more work

                task = task_queue.front();
                task_queue.pop();
            }

            render_pixel(task.first, task.second);

            int progress = static_cast<int>((static_cast<float>(processed_pixels.load()) / total_pixels) * 100);
            if (VERBOSE)
            {
                // Print progress bar every 1% change
                static int last_progress = -1;
                if (progress != last_progress)
                {
                    last_progress = progress;
                    std::cerr << "\rProgress: [" << std::setw(3) << progress << "%] "
                              << std::string(progress / 2, '=') << std::string(50 - progress / 2, ' ') << std::flush;
                }
            }
        }
    };

    for (int h = 0; h < height; ++h)
    {
        for (int w = 0; w < width; ++w)
        {
            task_queue.push({w, h});
        }
    }

    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.push_back(std::thread(worker));
    }

    for (auto &th : threads)
    {
        th.join();
    }

    if (VERBOSE)
    {
        std::cerr << "\nRendering complete!" << std::endl;
    }
}

void Scene::setAmbientColor(const Color &ambientColor)
{
    ambient_color_ = ambientColor;
}