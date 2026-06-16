#include "Primitive.h"

using namespace ONEngine;

void ONEngine::from_json(const nlohmann::json& j, Sphere& s) {
	j.at("center").get_to(s.center);
	j.at("radius").get_to(s.radius);
}

void ONEngine::to_json(nlohmann::json& j, const Sphere& s) {
	j = nlohmann::json{
		{ "center", s.center },
		{ "radius", s.radius }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Cube& c) {
	j.at("center").get_to(c.center);
	j.at("size").get_to(c.size);
}

void ONEngine::to_json(nlohmann::json& j, const Cube& c) {
	j = nlohmann::json{
		{ "center", c.center },
		{ "size", c.size }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Line& l) {
	j.at("start").get_to(l.start);
	j.at("end").get_to(l.end);
}

void ONEngine::to_json(nlohmann::json& j, const Line& l) {
	j = nlohmann::json{
		{ "start", l.start },
		{ "end", l.end }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Ray& r) {
	j.at("origin").get_to(r.origin);
	j.at("direction").get_to(r.direction);
}

void ONEngine::to_json(nlohmann::json& j, const Ray& r) {
	j = nlohmann::json{
		{ "origin", r.origin },
		{ "direction", r.direction }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Plane& p) {
	j.at("normal").get_to(p.normal);
	j.at("d").get_to(p.d);
}

void ONEngine::to_json(nlohmann::json& j, const Plane& p) {
	j = nlohmann::json{
		{ "normal", p.normal },
		{ "d", p.d }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Frustum& f) {
	j.at("planes").get_to(f.planes);
}

void ONEngine::to_json(nlohmann::json& j, const Frustum& f) {
	j = nlohmann::json{
		{ "planes", f.planes }
	};
}

void ONEngine::from_json(const nlohmann::json& j, Cone& c) {
	j.at("center").get_to(c.center);
	j.at("angle").get_to(c.angle);
	j.at("radius").get_to(c.radius);
	j.at("height").get_to(c.height);
}

void ONEngine::to_json(nlohmann::json& j, const Cone& c) {
	j = nlohmann::json{
		{ "center", c.center },
		{ "angle", c.angle },
		{ "radius", c.radius },
		{ "height", c.height }
	};
}
