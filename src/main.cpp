#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <iostream>

#include "game.cpp"

#include "log.cpp"
#include "c_smart_ptr.cpp"

#include "phys.cpp"

#include "timers.cpp"


#define SDL_ASSERT_EQ(x, val) if ((x) != val) { \
FATAL("`{}` failed: {}", #x, SDL_GetError()); \
return EXIT_FAILURE; \
}


#define SDL_ASSERT_NE(x, val) if ((x) == val) { \
FATAL("`{}` failed: {}", #x, SDL_GetError()); \
return EXIT_FAILURE; \
}

int main() {
    auto pool = stms::ThreadPool();
    pool.start();
    stms::getLogPool() = &pool;
    stms::initLogging();

    PhysicsEngine phys{};

    SDL_ASSERT_EQ(SDL_Init(SDL_INIT_EVERYTHING), 0);
    SDL_ASSERT_NE(IMG_Init(IMG_INIT_PNG), 0);

    CSmartPtr<SDL_Window> win(SDL_CreateWindow("Hello World!", 100, 100, winWidth(), winHeight(), SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE), SDL_DestroyWindow);
    SDL_ASSERT_NE(win.val, nullptr);

    Quitter quitter{};

    CSmartPtr<SDL_Renderer> ren(SDL_CreateRenderer(win.val, -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);
    SDL_ASSERT_NE(ren.val, nullptr);


    Ball ball = Ball(phys.makeDynamicCircle(0, 0, fieldWidth / 8.), ren.val);
    Ship ship = Ship(phys.makeDynamicBox(5, -(fieldHeight / 2.), fieldWidth / 8., fieldHeight / 8.), ren.val, Team{255, 0, 0});

    stms::TPSTimer timer{};
    while (true) {
        timer.tick();

        INFO("FPS = {}, MSPT = {}", timer.getLatestTps(), timer.getLatestMspt());

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            INFO("Window resized to {}x{}", event.window.data1, event.window.data2);
                            winWidth() = event.window.data1;
                            winHeight() = event.window.data2;
                            break;
                        case SDL_WINDOWEVENT_CLOSE:
                            INFO("Window closed! quitting!");
                            goto done;
                    }
                    break;
                case SDL_QUIT:
                    INFO("SDL quit!");
                    goto done;
            }
        }

        SDL_SetRenderDrawColor(ren.val, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(ren.val);
        ship.draw();
        ball.draw();

        SDL_RenderPresent(ren.val);


        if (targetFps > 0) {
            timer.wait(targetFps);
        } else if (targetFps == 0) {
            SDL_DisplayMode mode;
            SDL_ASSERT_EQ(SDL_GetWindowDisplayMode(win.val, &mode), 0);
            timer.wait(mode.refresh_rate);
        }
    }

    done:
    
    return EXIT_SUCCESS;
}