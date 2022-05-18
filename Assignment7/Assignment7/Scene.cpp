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

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    if (depth > this->maxDepth) {
        return Vector3f(0.0,0.0,0.0);
    }

    Intersection intersection = Scene::intersect(ray);
    if(intersection.happened) {
        return shade(intersection, ray);
    }

    return Vector3f();
}

Vector3f Scene::shade(Intersection& p, const Ray& wo) const
{
    //1.a ray hit the sence
    Vector3f L_dir;
    Intersection inter;
    float pdf_light = 0.f;
    sampleLight(inter, pdf_light);

    Vector3f x = inter.coords;//a ray from p to x, x is hit light point
    Vector3f ws = p.coords - x;//from x to p
    Vector3f N = p.normal.normalized();
    Vector3f NN = inter.normal.normalized();
    Intersection pTox = Scene::intersect(Ray(p.coords, -ws));

    if(pTox.obj == inter.obj) {
        L_dir = inter.emit * p.m->eval(wo.direction, ws, N) \
            * dotProduct(ws, N) * dotProduct(ws, NN) \
            / ws.norm() / pdf_light;
    }

    Vector3f L_indir;
    if(get_random_float() < RussianRoulette) {
        Vector3f wi;
        Object* obj;
        float tNear = 0.f;
        uint32_t hitIndex;

        //sample a light from p
        wi = p.m->sample(wi, N);
        //trace a ray from p
        //bool isHit = trace(Ray(p.coords, wi), objects, tNear, hitIndex, &obj);
        Intersection q = Scene::intersect(Ray(p.coords, wi));
        if(q.happened && !q.obj->hasEmit()) {
            L_indir = shade(q, Ray(p.coords, wi)) * q.m->eval(wo.direction, wi, N) \
                * dotProduct(wi, N) / 1/(2*M_PI) / RussianRoulette;
        }

    }

    return L_dir + L_indir;
}
