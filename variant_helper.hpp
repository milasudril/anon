#ifndef ANON_VARIANTHELPER_HPP
#define ANON_VARIANTHELPER_HPP

#include <variant>
#include <array>
#include <cstddef>

namespace anon
{
	template<class Variant, class Predicate, size_t N = std::variant_size_v<Variant>>
	constexpr size_t find_type(Predicate&& cb)
	{
		if constexpr(N != 0)
		{
			using current_type = std::variant_alternative_t<N - 1, Variant>;
			if(cb(std::type_identity<current_type>{}))
			{
				return N - 1;
			}
			else
			{
				return find_type<N - 1>(std::forward<Predicate>());
			}
		}
		else
		{
			return std::variant_npos;
		}
	}

	template<class Variant, class ... CtorArgs>
	using variant_factory = Variant (*)(CtorArgs&&...);

	template<size_t N, class Variant, class ... CtorArgs>
	constexpr void set_item(std::array<variant_factory<Variant, CtorArgs...>, std::variant_size_v<Variant>>& values)
	{
		if constexpr(N != 0)
		{
			using current_type = std::variant_alternative_t<N - 1, Variant>;
			values[N - 1] = [](CtorArgs&&... args){
				return Variant{std::in_place_type_t<current_type>{}, std::move(args)...};
			};
			set_item<N - 1>(values);
		}
	}

	template<class Variant, class ... CtorArgs>
	constexpr auto create_factory_table()
	{
		std::array<variant_factory<Variant, CtorArgs ...>, std::variant_size_v<Variant>> ret{};
		set_item<std::size(ret)>(ret);
		return ret;
	}

	template<class Variant, class ... CtorArgs>
	inline constexpr auto variant_factories = create_factory_table<Variant, CtorArgs...>();

	template<class Variant, class ... CtorArgs>
	constexpr auto create(size_t type_id, CtorArgs&&...args)
	{
		return variant_factories<Variant, CtorArgs...>[type_id](std::forward<CtorArgs>(args)...);
	}
}

#endif