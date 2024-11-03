#include <nlohmann/json.hpp>
#include "PipelineHandler.h"

PipelineHandler::PipelineHandler(const std::string& file_name) : file_(std::make_unique<File>(file_name)) {
    LOG_TRACE("PipelineHandler constructor");
}

PipelineHandler::~PipelineHandler() {
    LOG_TRACE("PipelineHandler destructor");
}

std::vector<PipelineElement> PipelineHandler::get_all_elements() {
    nlohmann::json json_data;
    file_->get_content() >> json_data;

    std::vector<PipelineElement> all_elements;
    for (const auto& element : json_data["pipeline"]["elements"]) {
        static unsigned int id = 0;
        all_elements.emplace_back(PipelineElement{
            id++,
            element["name"].get<std::string>(),
            element["type"].get<std::string>(),
            element["caps"].get<std::string>(),
            element["optional"].get<bool>(),
            }
            );
    }

    return all_elements;
}
