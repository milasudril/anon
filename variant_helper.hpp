#ifndef ANON_VARIANTHELPER_HPP
#define ANON_VARIANTHELPER_HPP

#include <variant>
#include <array>
#include <cstddef>
#include <type_traits>

namespace anon::variant_helper
{
	template<class T>
	using empty = std::type_identity<T>;

	/**
	 * @brief Find the index of the type that satisfies pred
	 */
	template<class Variant, class Predicate, size_t N = std::variant_size_v<Variant>>
	constexpr size_t find_type(Predicate&& pred)
	{
		if constexpr(N != 0)
		{
			using current_type = std::variant_alternative_t<N - 1, Variant>;
			if(pred(empty<current_type>{}))
			{
				return N - 1;
			}
			else
			{
				return find_type<Variant, Predicate, N - 1>(std::forward<Predicate>(pred));
			}
		}
		else
		{
			return std::variant_npos;
		}
	}

	template<class Callback, class ... Args>
	using callback_wrapper = void (*)(Callback&&, Args&&...);

	template<class Variant, size_t N, class Callback, class ... Args>
	constexpr void assign_callback(
		std::array<callback_wrapper<Callback, Args...>, std::variant_size_v<Variant>>& values)
	{
		if constexpr(N != 0)
		{
			using current_type = std::variant_alternative_t<N - 1, Variant>;
			values[N - 1] = [](Callback&& cb, Args&&... args){
				std::invoke(std::forward<Callback>(cb),
					empty<current_type>{},
					std::forward<Args>(args)...);
			};
			assign_callback<Variant, N - 1>(values);
		}
	}

	template<class Variant, class Callback, class ... Args>
	constexpr auto create_vtable()
	{
		constexpr auto N = std::variant_size_v<Variant>;
		std::array<callback_wrapper<Callback, Args ...>, N> ret{};
		assign_callback<Variant, std::size(ret)>(ret);
		return ret;
	}

	/**
	 * @brief Calls cb for an "empty" of type with index
	 */
	template<class Variant, class Callback, class ... Args>
	decltype(auto) on_type_index(size_t index, Callback&& cb, Args&&... args)
	{
		static constexpr auto vtable = create_vtable<Variant, Callback, Args...>();
		vtable[index](std::forward<Callback>(cb), std::forward<Args>(args)...);
	}
}

#endif