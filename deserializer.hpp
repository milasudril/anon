//@	{"dependencies_extra":[{"ref":"./deserializer.o","rel":"implementation"}]}

#ifndef ANON_DESERIALIZER_HPP
#define ANON_DESERIALIZER_HPP

/**
 * \file deserializer.hpp
 *
 * \brief Contains declarations and definitions regarding de-serialization
 */

#include "./object.hpp"
#include "./type_info.hpp"

#include <stack>
#include <filesystem>
#include <optional>

/**
 * \defgroup de-serialization
 */

namespace anon
{
	namespace deserializer_detail
	{
		/**
		* \brief Holds the current parsing context
		*
		* \ingroup de-serialization
		*/
		struct parser_context
		{
			using state = parser_state;

			state current_state{state::init};
			state prev_state{state::init};
			std::string buffer;
			using node_type = std::pair<object::key_type, object::mapped_type>;
			std::string current_key;
			node_type current_node;
			std::stack<node_type> parent_nodes;
			size_t level{0};
		};

		/**
		* \brief Holds the result after processing one byte
		*
		* \ingroup de-serialization
		*
		*/
		enum class parse_result{done, more_data_needed};

		/**
		* \brief Processes input, and updates ctxt accordingly
		*
		* \return parse_result::done if the outermost object was closed, otherwise
		*         parse_result::more_data_needed
		*
		* \ingroup de-serialization
		*/
		parse_result update(std::optional<char> input, parser_context& ctxt);
	}

	template<class Source>
	requires(!std::is_same_v<Source, std::filesystem::path>)
	object load(Source&& src)
	{
		deserializer_detail::parser_context ctxt;

		while(true)
		{
			if(update(read_byte(src), ctxt) == deserializer_detail::parse_result::done)
			{
				return std::get<object>(ctxt.current_node.second);
			}
		}
	}

	struct cfile_reader
	{
		FILE* src;
	};

	inline std::optional<char> read_byte(cfile_reader src)
	{
		auto ch_in = getc(src.src);
		return ch_in != EOF? static_cast<char>(ch_in): std::optional<char>{};
	}

	inline object load(FILE* src)
	{
		return load(cfile_reader{src});
	}

	inline object load(std::filesystem::path const& path)
	{
		auto file_deleter = [](FILE* f){ return fclose(f); };
		std::unique_ptr<FILE, decltype(file_deleter)> src{fopen(path.c_str(), "rb")};
		if(src == nullptr)
		{
			throw std::runtime_error{std::string{"Failed to open file "}.append(path)};
		}
		return load(src.get());
	}
}

#endif