//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include <iostream>
using namespace std;


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){//if object can emit light
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}


Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    Intersection inter = Scene::intersect(ray);

    if(!inter.happened) {
        return Vector3f(0.5, 0.5, 0.5);
    }

    if(inter.m->hasEmission())//ray from light
    {
        if(depth == 0) 
            return inter.m->getEmission();
        else
            return Vector3f();
    }

    Vector3f L_dir, L_indir;

    //sample a ray from light, get hit point on light surface.
    Intersection L_inter;
    float pdf_light = 0.f;
    sampleLight(L_inter, pdf_light);

    Vector3f& N = inter.normal;//normal of p
    Vector3f& NN = L_inter.normal;//normal of L_inter
    Vector3f& lightPos = L_inter.coords;//a ray from p to x, x is hit light point
    Vector3f& objPos = inter.coords;
    Vector3f diff = lightPos - objPos;//from p to x, diff
    Vector3f ws = diff.normalized();
    float lightDistance = dotProduct(diff, diff);

    Ray light(objPos, ws);
    Intersection pTox = Scene::intersect(light);
    // if(pTox.obj == L_inter.obj)
    if(pTox.happened && (pTox.coords - lightPos).norm() < 1e-2)
    {
        Vector3f f_r = inter.m->eval(ray.direction, ws, N);
        L_dir = L_inter.emit * f_r \
            * dotProduct(ws, N) * dotProduct(-ws, NN) \
            / lightDistance / pdf_light;//-ws
    }

    if(get_random_float() < RussianRoulette)
    {
        //sample a ray from p, trace a ray from p
        Vector3f wi = inter.m->sample(Vector3f(), N).normalized();
        Intersection pToq = Scene::intersect(Ray(inter.coords, wi));
        if(pToq.happened && !pToq.m->hasEmission())
        {
            float pdf = inter.m->pdf(ray.direction, wi, N);
            Vector3f f_r = inter.m->eval(ray.direction, wi, N);//inter
            L_indir = castRay(Ray(inter.coords, wi), depth + 1) * f_r \
                * dotProduct(wi, N) / pdf / RussianRoulette;
        }
    }
    return L_dir + L_indir;
}

/*
// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    if (depth > this->maxDepth) {
        return Vector3f(0.5, 0.5, 0.5);
    }

    Intersection intersection = Scene::intersect(ray);
    if(intersection.happened) {
        return shade(intersection, ray);
    }

    return Vector3f();
}*/

Vector3f Scene::shade(Intersection& p, const Ray& wo) const
{
    //sample a ray from light, get hit point on light surface.
    Vector3f L_dir;
    Intersection L_inter;
    float pdf_light = 0.f;
    sampleLight(L_inter, pdf_light);

    Vector3f x = L_inter.coords;//a ray from p to x, x is hit light point
    Vector3f ws = x - p.coords;//from p to x
    Vector3f N = p.normal.normalized();//normal of p
    Vector3f NN = L_inter.normal.normalized();//normal of L_inter
    Intersection pTox = Scene::intersect(Ray(p.coords, ws));

    if(pTox.happened && (pTox.coords - x).norm() < 1e-2) {
        L_dir = L_inter.emit * p.m->eval(wo.direction, ws, N) \
            * dotProduct(ws, N) * dotProduct(-ws, NN) \
            / dotProduct(ws, ws) / pdf_light;
    }

    Vector3f L_indir;
    if(get_random_float() < RussianRoulette) {
        //sample a ray from p
        Vector3f wi = p.m->sample(Vector3f(), N);
        //trace a ray from p
        Intersection pToq = Scene::intersect(Ray(p.coords, wi));
        if(pToq.happened && !pToq.obj->hasEmit()) {
            L_indir = shade(pToq, Ray(p.coords, wi)) * p.m->eval(wo.direction, wi, N) \
                * dotProduct(wi, N) / p.m->pdf(wo.direction, wi, N) / RussianRoulette;
        }
    }

    return L_dir + L_indir;
}
