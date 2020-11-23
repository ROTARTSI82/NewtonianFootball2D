//
// Created by grant on 11/22/20.
//

#pragma once

#ifndef PHYS_CPP_INCLUDED
#define PHYS_CPP_INCLUDED

#include <box2d/box2d.h>
#include <vector>
#include "config.hpp"

struct RigidBody {
    float w{}, h{};

    b2BodyDef def;
    b2Body *body{};
    b2PolygonShape shape;
    b2FixtureDef fixture;
};

struct CircleRigidBody {
    b2BodyDef def;
    b2Body *body{};
    b2CircleShape shape;
    b2FixtureDef fixture;
};

class PhysicsEngine {
public:
    b2Vec2 gravity{0, 0};
    b2World world{gravity};

    std::vector<RigidBody> bodies;


    inline void addWall(float x, float y, float w, float h) {
        RigidBody body;
        body.def.position.Set(x, y);
        body.body = world.CreateBody(&body.def);
        body.shape.SetAsBox(w, h);
        body.body->CreateFixture(&body.shape, 0.0f);

        bodies.emplace_back(body);
    }

    PhysicsEngine() {
        // field is 120 x 75 (or x4 480 x 300)
        addWall(-fieldWidth, 0, wallWidth + fbuf, fieldHeight + fbuf);
        addWall(fieldWidth, 0, wallWidth+ fbuf, fieldHeight + fbuf);
        addWall(0, fieldHeight, fieldWidth + fbuf, wallWidth + fbuf);
        addWall(0, -fieldHeight, fieldWidth + fbuf, wallWidth + fbuf);
    }

    inline void step(float time = 1.0f / 60.f, int32 velIter = 6, int32 posIter = 2) {
        world.Step(time, velIter, posIter);
    }

    RigidBody makeDynamicBox(float x, float y, float w, float h, float density = 1.0f, float friction = 0.3f) {
        RigidBody ret;
        ret.def.type = b2_dynamicBody;
        ret.def.position.Set(x, y);
        ret.body = world.CreateBody(&ret.def);
        ret.shape.SetAsBox(w, h);
        ret.fixture.shape = &ret.shape;
        ret.fixture.density = density;
        ret.fixture.friction = friction;
        ret.body->CreateFixture(&ret.fixture);

        ret.w = w;
        ret.h = h;

        return ret;
    }

    CircleRigidBody makeDynamicCircle(float x, float y, float r, float density = 1.0f, float friction = 0.3f) {
        CircleRigidBody ret;
        ret.def.type = b2_dynamicBody;
        ret.def.position.Set(x, y);
        ret.body = world.CreateBody(&ret.def);
        ret.shape.m_radius = r;
        ret.fixture.shape = &ret.shape;
        ret.fixture.density = density;
        ret.fixture.friction = friction;
        ret.body->CreateFixture(&ret.fixture);
        return ret;
    }


};

#endif