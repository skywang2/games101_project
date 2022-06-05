//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include "Scene.hpp"
#include "Renderer.hpp"
using namespace std;


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;

    // for(auto& obj : scene.objects) {
    //     if(obj->hasEmit()) cout << obj << endl;
    // }

    // change the spp value to change sample ammount
    std::fstream frameFile("framebuffer.txt", std::fstream::out);
    /*
    int spp = 1;
    std::cout << "SPP: " << spp << "\n";
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

            Vector3f dir = normalize(Vector3f(-x, y, 1));
            for (int k = 0; k < spp; k++){
                framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp; 
                // if(frameFile.is_open()) frameFile << framebuffer[m] << std::endl; 
            }
            m++;
        }
        UpdateProgress(j / (float)scene.height);
        frameFile << flush;
    }
    UpdateProgress(1.f);
*/

    //thread
    vector<thread> threads;
    const uint32_t rowNum = 8;
    for (uint32_t j = 0; j < scene.height;) {
        thread th = thread(&Renderer::ThreadRender, this, j, j + rowNum, ref(scene), ref(framebuffer), m);
        j += rowNum;
        m += rowNum * scene.width;
        threads.push_back(move(th));
    }

    int count = 1;
    for(auto& th : threads)
    {
        if(th.joinable()) th.join();
        UpdateProgress(count++ / (float)threads.size());
    }
    UpdateProgress(1.f);
    if(frameFile.is_open()) frameFile.close();

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}

void Renderer::ThreadRender(const int rowStart, const int rowEnd, const Scene& scene, std::vector<Vector3f>& framebuffer, int m)
{
    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int spp = 16;

    for (uint32_t j = rowStart; (j < scene.height) && (j < rowEnd); ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) 
        {
            // generate primary ray direction
            float x = (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
            Vector3f dir = normalize(Vector3f(-x, y, 1));
            for (int k = 0; k < spp; k++){
                if(m > framebuffer.size()) return;
                framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp; 
            }
            m++;
        }
    }
}

