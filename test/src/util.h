#pragma once

#include <execution>
#include <ranges>
#include <vector>
#include <span>


namespace test_util
{

	namespace detail
	{
		template <typename Container1, typename Container2>
		concept ContainerPair = requires(Container1 x, Container2 y) {
			{ x.size() } -> std::convertible_to<std::size_t>;
			{ y.size() } -> std::convertible_to<std::size_t>;
			{ x[0] } -> std::same_as<decltype(y[0])>;
		};

		template <typename Container, typename T>
		concept ContainerAndValue = requires(Container x, T val) {
			{ x.size() } -> std::convertible_to<std::size_t>;
			{ x[0] != val } -> std::convertible_to<bool>;
		};

	} // detail


	template <typename Container1, typename Container2>
		requires detail::ContainerPair<Container1, Container2>
	void check_vector_verbose(const Container1& x, const Container2& y)
	{
		REQUIRE(x.size() == y.size());
		for (size_t i = 0; i < x.size(); ++i) {
			if (x[i] != y[i]) {
				REQUIRE_MESSAGE(x[i] == y[i], "Failed vector index: " << i);
			}
		}
	}

	template <typename Container, typename T>
		requires detail::ContainerAndValue<Container, T>
	void check_vector_verbose(const Container& x, T y)
	{
		for (size_t i = 0; i < x.size(); ++i) {
			if (x[i] != y) {
				REQUIRE_MESSAGE(x[i] == y, "Failed vector index: " << i);
			}
		}
	}

}