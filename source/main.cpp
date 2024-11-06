#include <memory>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sys/select.h>
#include <unistd.h>
#include "Logger/Logger.h"
#include "Gstreamer/Gstreamer.h"
#include "cxxopts.hpp"

struct AppConfig {
    std::filesystem::path input_file;
    bool verbose;
};

AppConfig parse_command_line_arguments(const int argc, const char* argv[]) {
    cxxopts::Options options(argv[0], "Gstreamer runner");
    options.add_options()
        ("i,input", "Input pipeline file", cxxopts::value<std::filesystem::path>()->default_value("../resources/pipeline.json"))
        ("v,verbose", "Enable verbose logging", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(EXIT_SUCCESS);
    }

    AppConfig config {
        .input_file = result["input"].as<std::filesystem::path>(),
        .verbose = result["verbose"].as<bool>()
    };

    return config;
}

void configure_logger(const bool verbose) {
    if (verbose) {
        Logger::SET_LOG_LEVEL(LoggerInterface::LogLevel::Trace);
        LOG_INFO("Verbose logging enabled.");
    } else {
        Logger::SET_LOG_LEVEL(LoggerInterface::LogLevel::Info);
    }
}

std::filesystem::path get_pipeline_file_path(const std::filesystem::path& file_path) {
    std::filesystem::path pipeline_file = std::filesystem::current_path() / file_path;
    if (!exists(pipeline_file)) {
        LOG_ERROR("Pipeline file not found at: {}", pipeline_file.string());
        throw std::runtime_error("Pipeline file not found");
    } else {
        LOG_INFO("Provided pipeline file: {}", pipeline_file.string());
    }

    return pipeline_file;
}

void user_input_thread(const std::shared_ptr<Gstreamer>& gstreamer, std::atomic<bool>& keep_running) {
    LOG_INFO("Enter 'e' to enable optional element, 'd' to disable optional element, and 'q' to quit:");

    while (keep_running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100 milliseconds

        int result = select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout);

        if (result > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) {
            char command = 0;
            std::cin >> command;

            switch (command) {
                case 'e':
                    gstreamer->enable_optional_pipeline_elements();
                    LOG_INFO("Enabled optional pipeline elements.");
                    break;
                case 'd':
                    gstreamer->disable_optional_pipeline_elements();
                    LOG_INFO("Disabled optional pipeline elements.");
                    break;
                case 'q':
                    gstreamer->stop();
                    keep_running = false;  // Signal to stop the thread
                    LOG_INFO("Stopped pipeline and exiting.");
                    return;
                default:
                    LOG_WARN("Invalid command. Use 'e', 'd', or 'q'.");
                    break;
            }
        }
    }
}

int main(const int argc, const char* argv[]) {
    const AppConfig config = parse_command_line_arguments(argc, argv);
    configure_logger(config.verbose);

    LOG_TRACE("{} {}.{}.{}", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);

    try {
        auto pipeline_file = get_pipeline_file_path(config.input_file);
        auto gstreamer = std::make_shared<Gstreamer>(pipeline_file);
        std::atomic<bool> keep_running{true};

        std::thread input_thread(user_input_thread, gstreamer, std::ref(keep_running));

        gstreamer->play(); // Blocking call

        keep_running = false; // Signal the input thread to stop once play() returns
        if (input_thread.joinable()) {
            input_thread.join();
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}