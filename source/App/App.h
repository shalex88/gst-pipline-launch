#ifndef PERIPHERY_MANAGER_APP_H
#define PERIPHERY_MANAGER_APP_H

#include <atomic>
#include <filesystem>

struct AppConfig {
    std::filesystem::path input_file;
    bool verbose;
};

class App {
public:
    App() = default;
    ~App() = default;
    void run(const AppConfig& config);
    static void shutdown();

private:
    static std::atomic<bool> keep_running_;
};

#endif //PERIPHERY_MANAGER_APP_H