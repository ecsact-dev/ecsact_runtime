#pragma once

namespace ecsact {

/**
 * Helper type to contain an arbitrary amounts of types. Meant for
 * metaprogramming use.
 * SEE: boost.mp11
 */
template<typename... T>
struct mp_list {};

template<typename T>
struct entity_component_required {
	using type = T;
};

template<typename T>
struct entity_component_optional {
	using type = T;
};

} // namespace ecsact
