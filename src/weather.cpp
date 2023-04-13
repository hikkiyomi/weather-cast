#include "../include/weather.h"

#include <conio.h>
#include <windows.h>

#include <iostream>

constexpr char kEscapeKey = 27;

Forecaster::Forecaster()
    : handler(new Handler())
{}

Forecaster::~Forecaster() {
    delete handler;
}

void Forecaster::ChangeConfig(const std::filesystem::path& config_path) {
    delete handler;
    handler = new Handler(config_path);
}

void TestEsc() {
    while (true) {
        char c = getch();

        if (c == kEscapeKey) {
            system("cls");
            exit(0);
        } else {
            std::cout << c << " is pressed" << std::endl;
        }
    }
}

void Heh() {
    while (true) {
        Sleep(1000);
        std::cout << "hey" << std::endl;
    }
}

void Forecaster::Run() {
    BlockInput(true);

    std::vector<std::thread> threads;

    threads.push_back(std::thread(Heh));
    threads.push_back(std::thread(TestEsc));

    for (auto& thread : threads) {
        thread.join();
    }
}
