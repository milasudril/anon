#ifndef ANON_TYPENAME_HPP
#define ANON_TYPENAME_HPP

/**
 * \file type_info.hpp
 *
 * \brief Contains mappings between types and initial parser states + type names
 */

#include "./object.hpp"

/**
 * \defgroup type_info Type information
 *
 * \brief This module contains type information used during \ref serialization and
 * \ref de-serialization
 *
 */

namespace anon
{
	enum class parser_state:int{init, type_tag, after_type_tag, key, after_key, ctrl_char, value};

	template<class T>
	struct type_info;

	constexpr size_t strlen(char const* str)
	{
		auto start = str;
		while(*str != '\0')
		{
			++str;
		}
		return static_cast<size_t>(str - start);
	}


	template<>
	struct type_info<int32_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "i32"; }
	};

	template<>
	struct type_info<int64_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "i64"; }
	};

	template<>
	struct type_info<uint32_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "u32"; }
	};

	template<>
	struct type_info<uint64_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "u64"; }
	};

	template<>
	struct type_info<float>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "f32"; }
	};

	template<>
	struct type_info<double>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "f64"; }
	};

	template<>
	struct type_info<std::string>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "str"; }
	};

	template<>
	struct type_info<object>
	{
		static constexpr auto parser_init_state(){ return parser_state::key; }
		static constexpr char const* name(){ return "obj"; }
	};

	template<class T>
	struct type_info<std::vector<T>> : type_info<T>
	{
		static constexpr char const* name() { return std::data(namebuff); }

		static constexpr auto make_namebuff()
		{
			constexpr auto N = strlen(type_info<T>::name());
			std::array<char, N + 2> ret{};
			std::copy_n(type_info<T>::name(), N, std::data(ret));
			std::copy_n("*", 2, std::data(ret) + N);
			return ret;
		}

		static constexpr auto namebuff = make_namebuff();
	};
}
#endif