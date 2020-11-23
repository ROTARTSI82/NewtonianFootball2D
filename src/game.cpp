//
// Created by grant on 11/22/20.
//

#include "phys.cpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "globals.cpp"

#include <utility>
#include <string>
#include <log.hpp>


struct NF2Cam {
    b2Vec2 center{0, 0};
    b2Vec2 size{fieldWidth, fieldHeight}; // size of the camera, in half-width and half-height
};

NF2Cam &getCamera() {
    static auto cam = NF2Cam();
    return cam;
}

b2Vec2 transformCam(const b2Vec2 &in) {
    b2Vec2 topLeft = getCamera().center - getCamera().size; // get the top-left corner of the camera
    b2Vec2 relative = in - topLeft; // get the coord in a translated coord system with top left of cam as (0, 0)

    // Simply scale this coord up by mapping range [1-cam.size*2] to [1-winSize]
    b2Vec2 ret;
    ret.x = (relative.x * winWidth()) / (getCamera().size.x * 2);
    ret.y = (relative.y * winHeight()) / (getCamera().size.y * 2);

    return ret;
}


class Ball {
public:
    CircleRigidBody body;
    SDL_Texture *tex;
    SDL_Rect rect;
    SDL_Renderer *ren;

    Ball(CircleRigidBody b, SDL_Renderer *ren, const std::string &img = "./res/ball.png") : body(std::move(b)), ren(ren) {
        tex = IMG_LoadTexture(ren, img.c_str());
    }
    void draw() {
        // B---A
        // | O |
        // C---D
        // Figure

        b2Vec2 pos = body.body->GetPosition(); // get position of ball in real-space (Point O in figure above)
        b2Vec2 size = b2Vec2(body.shape.m_radius, body.shape.m_radius); // Vector from O -> D in figure above
        pos -= size; // Subtract size to find corner B in figure above, and transform that into screen-space
        size *= 2; // Transform the half-dimensions into full dimensions.
        b2Vec2 d = pos + size; // Find point D by adding pos and size vecs together.
        pos = transformCam(pos); // Transform position into screen-space
        d = transformCam(d); // Transform opposite corner into screen-space
        size = d - pos; // Find the difference between D and B!

        b2Vec2 center = transformCam(body.body->GetPosition());


        INFO("BALL DRAW: angle={}  pos=[{}, {}]", body.body->GetAngle(), pos.x, pos.y);
        rect.x = pos.x;
        rect.y = pos.y;
        rect.w = size.x;
        rect.h = size.y;

        SDL_Point point = {static_cast<int>(center.x), static_cast<int>(center.y)};
        if (SDL_RenderCopyEx(ren, tex, nullptr, &rect, body.body->GetAngle(), &point, SDL_FLIP_NONE) != 0) {
            FATAL("Failed to render ship: {}", SDL_GetError());
            throw std::runtime_error("Rendering failed");
        }
    }

    virtual ~Ball() {
        SDL_DestroyTexture(tex);
    }
};

struct Team {
public:
    uint8_t r, g, b;
};

class Ship {
public:
    Team team;
    RigidBody body;
    SDL_Texture *tex;
    SDL_Rect rect;
    SDL_Renderer *ren;

    float turnImpulse;

    Ship(RigidBody b, SDL_Renderer *ren, Team t, const std::string &img = "./res/ship.png") : body(std::move(b)), team(t), ren(ren) {
        tex = IMG_LoadTexture(ren, img.c_str());
        SDL_SetTextureColorMod(tex, team.r, team.g, team.b);
    }

    void draw() {
        // B---A
        // | O |
        // C---D
        // Figure

        b2Vec2 pos = body.body->GetPosition(); // get position of ball in real-space (Point O in figure above)
        b2Vec2 size = b2Vec2(body.w, body.h); // Vector from O -> D in figure above
        pos -= size; // Subtract size to find corner B in figure above, and transform that into screen-space
        size *= 2; // Transform the half-dimensions into full dimensions.
        b2Vec2 d = pos + size; // Find point D by adding pos and size vecs together.
        pos = transformCam(pos); // Transform position into screen-space
        d = transformCam(d); // Transform opposite corner into screen-space
        size = d - pos; // Find the difference between D and B!

        b2Vec2 center = transformCam(body.body->GetPosition());

        INFO("Ship DRAW: angle={}  pos=[{}, {}]", body.body->GetAngle(), pos.x, pos.y);
        rect.x = pos.x;
        rect.y = pos.y;
        rect.w = size.x;
        rect.h = size.y;

        SDL_Point point = {static_cast<int>(center.x), static_cast<int>(center.y)};
        if (SDL_RenderCopyEx(ren, tex, nullptr, &rect, body.body->GetAngle(), &point, SDL_FLIP_NONE) != 0) {
            FATAL("Failed to render ship: {}", SDL_GetError());
            throw std::runtime_error("Rendering failed");
        }
    }

    void turn(int accl) const {
        if (accl > 0) {
            body.body->ApplyAngularImpulse(turnImpulse, true);
        } else if (accl < 0) {
            body.body->ApplyAngularImpulse(-turnImpulse, true);
        }
    }

    void apply(float amt) const {
        float angle = body.body->GetAngle();
        b2Vec2 forward(std::sin(angle) * amt, std::cos(angle) * amt);
        body.body->ApplyForceToCenter(forward, true);
    }

    virtual ~Ship() {
        SDL_DestroyTexture(tex);
    }
};

