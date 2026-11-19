#include "math_stuff.hpp"

// Serialization function for BoundingBox
void to_json(nlohmann::json& j, const BoundingBox& bbox)
{
    j = nlohmann::json{
        {"min", nlohmann::json::array({ bbox.min.x, bbox.min.y, bbox.min.z })},
        {"max", nlohmann::json::array({ bbox.max.x, bbox.max.y, bbox.max.z })}
    };
}

// Deserialization function for BoundingBox
void from_json(const nlohmann::json& j, BoundingBox& bbox)
{
    const auto min = j["min"];
    bbox.min = Vector3{ min[0].get<float>(), min[1].get<float>(), min[2].get<float>() };
    const auto max = j["max"];
    bbox.max = Vector3{ max[0].get<float>(), max[1].get<float>(), max[2].get<float>() };
}

// Serialization function for Rectangle
void to_json(nlohmann::json& j, const Rectangle& rect)
{
    j = nlohmann::json::array({
        rect.x, rect.y, rect.width, rect.height,
    });
}

// Deserialization function for Rectangle
void from_json(const nlohmann::json& j, Rectangle& rect)
{
    rect = Rectangle{ 
        .x = j[0].get<float>(),
        .y = j[1].get<float>(),
        .width = j[2].get<float>(),
        .height = j[3].get<float>(),
    };
}
