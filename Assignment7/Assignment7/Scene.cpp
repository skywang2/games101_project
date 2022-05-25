//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


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

/*
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    Intersection inter = Scene::intersect(ray);

    if(inter.happened)
    {
        // std::cout << "happened" << std::endl;
        if(inter.m->hasEmission())//ray from light
        {
            // static int count = 0;
            // std::cout << "happened && emission, " << count++ << ", depth" << depth << std::endl;
            if(depth == 0) 
            {
                // auto g = inter.m->getEmission();
                // std::cout << g.x << ", " << g.y << ", " << g.z << std::endl;
                return inter.m->getEmission();
            }
            else
                return Vector3f();
        }
        std::cout << inter.m->getEmission().x << std::endl;
        return inter.m->getEmission();

        Vector3f L_dir, L_indir;

        //sample a ray from light, get hit point on light surface.
        Intersection L_inter;
        float pdf_light = 0.f;
        sampleLight(L_inter, pdf_light);
        Vector3f x = L_inter.coords;//a ray from p to x, x is hit light point
        Vector3f ws = x - inter.coords;//from p to x
        Vector3f N = inter.normal.normalized();//normal of p
        Vector3f NN = L_inter.normal.normalized();//normal of L_inter
        float lightDistance = dotProduct(ws, ws);
        ws = ws.normalized();

        Intersection pTox = Scene::intersect(Ray(inter.coords, ws));
        if(pTox.obj == L_inter.obj)
        {
            L_dir = L_inter.emit * inter.m->eval(ws, ray.direction, N) \
                * dotProduct(ws, N) * dotProduct(ws, NN) \
                / lightDistance / pdf_light;
        }

        if(get_random_float() < RussianRoulette)
        {
            //sample a ray from p, trace a ray from p
            Vector3f wi = inter.m->sample(Vector3f(), N).normalized();
            Intersection pToq = Scene::intersect(Ray(inter.coords, wi));
            if(pToq.happened && !pToq.obj->hasEmit())
            {
                L_indir = castRay(Ray(inter.coords, wi), depth + 1) * pToq.m->eval(ray.direction, wi, N) \
                    * dotProduct(wi, N) / inter.m->pdf(ray.direction, wi, N) / RussianRoulette;
            }
        }
        return L_dir + L_indir;
    }

    return Vector3f();
}*/

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    if (depth > this->maxDepth) {
        return Vector3f(0.0,0.0,0.0);
    }

    Intersection intersection = Scene::intersect(ray);
    if(intersection.happened) {
        if(intersection.m->hasEmission())//ray from light
        {
            if(depth == 0) 
                return intersection.m->getEmission();
            else
                return Vector3f();
        }

        return shade(intersection, ray);
    }

    return Vector3f();
}

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

    if(pTox.obj == L_inter.obj) {
        L_dir = L_inter.emit * p.m->eval(ws, wo.direction, N)/** p.m->eval(wo.direction, ws, N)*/ \
            * dotProduct(ws, N) * dotProduct(ws, NN) \
            / dotProduct(ws, ws) / pdf_light;
    }

    Vector3f L_indir;
    if(get_random_float() < RussianRoulette) {
        Vector3f wi;
        Object* obj;
        uint32_t hitIndex;

        //sample a ray from p
        wi = p.m->sample(Vector3f(), N);
        //trace a ray from p
        Intersection pToq = Scene::intersect(Ray(p.coords, wi));
        if(pToq.happened && !pToq.obj->hasEmit()) {
            L_indir = shade(pToq, Ray(p.coords, wi)) * pToq.m->eval(wi, wo.direction, N)/** pToq.m->eval(wo.direction, wi, N)*/ \
                * dotProduct(wi, N) / pToq.m->pdf(wi, wo.direction, N) / RussianRoulette;
        }

    }

    return L_dir + L_indir;
}
