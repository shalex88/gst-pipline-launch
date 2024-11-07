#include <memory>
#include <iostream>
#include <filesystem>
#include <thread>
#include <sys/select.h>
#include <unistd.h>
#include "Logger/Logger.h"
#include "cxxopts.hpp"
#include "App/App.h"

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
        LOG_INFO("Verbose logging enabled");
    } else {
        Logger::SET_LOG_LEVEL(LoggerInterface::LogLevel::Info);
    }
}

int main(const int argc, const char* argv[]) {
    const AppConfig config = parse_command_line_arguments(argc, argv);
    configure_logger(config.verbose);

    LOG_TRACE("{} {}.{}.{}", APP_NAME, APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);

    try {
        App app;
        app.run(config);
    } catch (const std::exception& e) {
        LOG_ERROR("Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}