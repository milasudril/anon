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
	/**
	 * \brief Constexpr version of strlen
	 */
	constexpr size_t strlen(char const* str)
	{
		auto start = str;
		while(*str != '\0')
		{
			++str;
		}
		return static_cast<size_t>(str - start);
	}

	/**
	 * \brief Lists all possible parser states
	 *
	 * \ingroup type_info
	 */
	enum class parser_state:int{init, type_tag, after_type_tag, key, after_key, ctrl_char, value};

	/**
	 * \brief Struct that should contain type meta-data
	 *
	 * This struct should countain type metadata so it is possible to generate code forserialization
	 * and de-serialization. Fir this to work, it must provide two methods
	 *
	 *  ```
	 *  static constexpr parser_state parser_init_state();
	 *  static constexpr char const* name();
	 *  ```
	 *
	 * For an example see specializations provided by the \ref type_info module
	 *
	 * \ingroup type_info
	 */
	template<class T>
	struct type_info;

	/**
	 * \brief specialization of type_info for int32_t
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<int32_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "i32"; }
	};

	/**
	 * \brief specialization of type_info for int64_t
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<int64_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "i64"; }
	};

	/**
	 * \brief specialization of type_info for uint32_t
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<uint32_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "u32"; }
	};

	/**
	 * \brief specialization of type_info for uint64_t
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<uint64_t>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "u64"; }
	};

	/**
	 * \brief specialization of type_info for float
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<float>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "f32"; }
	};

	/**
	 * \brief specialization of type_info for double
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<double>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "f64"; }
	};

	/**
	 * \brief specialization of type_info for std::string
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<std::string>
	{
		static constexpr auto parser_init_state(){ return parser_state::value; }
		static constexpr char const* name(){ return "str"; }
	};

	/**
	 * \brief specialization of type_info for object
	 *
	 * \ingroup type_info
	 */
	template<>
	struct type_info<object>
	{
		static constexpr auto parser_init_state(){ return parser_state::key; }
		static constexpr char const* name(){ return "obj"; }
	};

	/**
	 * \brief specialization of type_info for std::vector<T>
	 *
	 * \ingroup type_info
	 */
	template<class T>
	struct type_info<std::vector<T>> : type_info<T>
	{
		/**
		 * \brief Returns the name of the type std::vector<T>
		 *
		 * The name of a std::vector<T> is derived from the name of T. It is constructed by
		 * appending an `*` after the name. For example, the name of `std::vector<int32_t>` is
		 * `i32*`. The syntax is designed to mimic regex zero-or-more syntax.
		 *
		 */
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