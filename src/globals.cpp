//
// Created by grant on 11/22/20.
//


int &winWidth() {
    static int val = 640;
    return val;
}

int &winHeight() {
    static int val = 480;
    return val;
}
