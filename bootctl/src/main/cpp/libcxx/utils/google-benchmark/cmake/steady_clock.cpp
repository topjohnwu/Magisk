#include <chrono>

int main() {
    typedef std::chrono::steady_clock Clock;
    Clock::time_point tp = Clock::now();
    ((void)tp);
}
