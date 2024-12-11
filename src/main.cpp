//
// Created by Martin Gano on 3.5.2022.
//

#include <vector>
#include <iostream>
#include <chrono>
#include <opencv2/highgui.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include "Scene.h"
#include <cmath>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "KDTree.h"
#include "base.h"
#include "Image.h"
#include "Triangle.h"
#include "ObjParser.h"
#include "globals.h"

using namespace std;
using namespace cv;

#define DEBUG

int main(int argc, char **argv)
{
    const int samples_per_pixel = 50;
    const int depth = 5;

    // ObjParser obj_parser = ObjParser("../demo_data/dragon_10k.obj");
    ObjParser obj_parser = ObjParser("../demo_data/combination_scaled.obj");
    vector<Triangle *> objects;

    Material mat;
    mat.diffuse_f_ = Color(1, 1, 1);
    mat.specular_f_ = Color(1, 1, 1);

    mat.ambient_k_ = 0;
    mat.diffuse_k_ = 1.0;
    mat.specular_k_ = 0;
    mat.shininess_ = 0;
    vector<cv::Vec3f> vertexes;
    obj_parser.parseObjects(objects, mat, vertexes, 0.3);

    Scene scene;

    auto start = chrono::steady_clock::now();
    KDTree *kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
    auto end = chrono::steady_clock::now();
    cout << "KD Tree construction took: "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms"
         << endl;

    scene.kd_tree = kdtree_root;

    float plane_height = HEIGHT;
    float plane_width = WIDTH;

    scene.camera = new Camera();
    scene.camera->origin_c = cv::Vec3f(0, -125, 0);
    scene.camera->view_plane_point_c = cv::Vec3f(0, 0, 0);
    scene.camera->view_plane_height = plane_height;
    scene.camera->view_plane_width = plane_width;

    scene.lights.push_back(new Light());
    scene.lights[0]->origin_c_ = cv::Vec3f(0, -150, 1);
    scene.lights[0]->color_ = Color(0, 0, 2);
    scene.lights[0]->intensity_ = INTENSITY;

    scene.setAmbientColor(Color(1, 1, 1));

    int image_width = WIDTH;
    int image_height = HEIGHT;
    Image image(image_width, image_height);
    for (;;)
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        scene.render(&image, vertexes, samples_per_pixel, depth);
        // cout << "Total number of required intersections: " << Triangle::total_intersections << endl;
        auto end_time = std::chrono::high_resolution_clock::now();
        auto time = end_time - start_time;
        std::cout << "Render time: " << time / std::chrono::milliseconds(1) << " ms\n";

#ifdef DEBUG
        std::cout << "Total number of triangles                    : " << num_triangles << "\n";
        std::cout << "Total number of primary rays                 : " << num_primary_rays << "\n";
        std::cout << "Total number of ray-triangles tests          : " << num_ray_triangle_tests << "\n";
        std::cout << "Total number of ray-triangles intersections  : " << num_ray_triangle_intersections << "\n";
        std::cout << "Total number of ray-kdtree node tests        : " << num_ray_kdtree_node_tests << "\n";
        std::cout << "Total number of ray-kdtree node intersections: " << num_ray_kdtree_node_intersections << "\n";
        std::cout << "Total number of kdtree nodes                 : " << num_kdtree_nodes << "\n";
        std::cout << "Total number of kdtree leaf nodes            : " << num_kdtree_leaf_nodes << "\n";
        std::cout << "Total number of ray-kdtree leaf tests        : " << num_ray_kdtree_leaf_tests << "\n";
        std::cout << "Total number of ray-kdtree leaf intersections: " << num_ray_kdtree_leaf_intersections << "\n";
#endif
        std::cout << "Samples per pixel                            : " << samples_per_pixel << "\n";
        std::cout << "Maximum ray depth                            : " << depth << "\n";
        std::cout << "Image Dimensions                             : " << image_width << "x" << image_height << "\n";
        cv::Mat frame = cv::Mat(image_width, image_height, CV_8UC3);
        cv::cvtColor(image.get_mat(), frame, cv::COLOR_BGRA2RGB);

        cv::imshow("Demo App", frame);
        int key = cv::waitKey(0);
        std::cout << key << endl;
        if (key == 27)
        {
            return 0;
        }
        if (key == 3)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(0, 0) = 1.1;
            rotation_matrix(1, 1) = 1.1;
            rotation_matrix(2, 2) = 1.1;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }
        if (key == 2)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(0, 0) = 0.9;
            rotation_matrix(1, 1) = 0.9;
            rotation_matrix(2, 2) = 0.9;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }
        if (key == 97)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(1, 1) = 0.98480775301;
            rotation_matrix(1, 2) = 0.17364817766;
            rotation_matrix(2, 1) = -0.17364817766;
            rotation_matrix(2, 2) = 0.98480775301;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }
        if (key == 115)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(1, 1) = 0.98480775301;
            rotation_matrix(1, 2) = -0.17364817766;
            rotation_matrix(2, 1) = 0.17364817766;
            rotation_matrix(2, 2) = 0.98480775301;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }

        if (key == 100)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(0, 0) = 0.98480775301;
            rotation_matrix(0, 2) = -0.17364817766;
            rotation_matrix(2, 0) = 0.17364817766;
            rotation_matrix(2, 2) = 0.98480775301;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }
        if (key == 102)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(0, 0) = 0.98480775301;
            rotation_matrix(0, 2) = 0.17364817766;
            rotation_matrix(2, 0) = -0.17364817766;
            rotation_matrix(2, 2) = 0.98480775301;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }

        if (key == 103)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(0, 0) = 0.98480775301;
            rotation_matrix(0, 1) = -0.17364817766;
            rotation_matrix(1, 0) = 0.17364817766;
            rotation_matrix(1, 1) = 0.98480775301;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }
        if (key == 104)
        {
            cv::Mat_<float> rotation_matrix = cv::Mat::eye(4, 4, CV_32FC1);
            rotation_matrix(0, 0) = 0.98480775301;
            rotation_matrix(0, 1) = 0.17364817766;
            rotation_matrix(1, 0) = -0.17364817766;
            rotation_matrix(1, 1) = 0.98480775301;
            for (auto &vertexe : vertexes)
            {
                Mat_<float> pm(4, 1);
                pm << vertexe[0], vertexe[1], vertexe[2], 1.0;

                Mat_<float> pr = rotation_matrix * pm;
                vertexe = Vec3f(pr(0), pr(1), pr(2));
            }
            start = chrono::steady_clock::now();
            kdtree_root = KDTree::construct_kdtree(objects, 0, vertexes);
            end = chrono::steady_clock::now();
            cout << "KD Tree construction took: "
                 << chrono::duration_cast<chrono::milliseconds>(end - start).count()
                 << " ms"
                 << endl;
            scene.kd_tree = kdtree_root;
        }
    }
}
