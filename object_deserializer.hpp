//@	{"dependencies_extra":[{"ref":"./object_deserializer.o","rel":"implementation"}]}

#ifndef ANON_OBJECTDESERIALIZER_HPP
#define ANON_OBJECTDESERIALIZER_HPP

#include "./object.hpp"

#include <stack>
#include <filesystem>
#include <optional>

namespace anon
{
	struct parser_context
	{
		enum class state:int{init, type_tag, after_type_tag, key, after_key, ctrl_char, value};

		state current_state{state::init};
		state prev_state{state::init};
		std::string buffer;
		using node_type = std::pair<object::key_type, object::mapped_type>;
		std::string current_key;
		node_type current_node;
		std::stack<node_type> parent_nodes;
		size_t level{0};
	};

	enum class parse_result{done, more_data_needed};

	parse_result update(std::optional<char> input, parser_context& ctxt);

	std::pair<parser_context::state, object::mapped_type> state_from_ctrl_word(std::string_view);

	template<class Source>
	requires(!std::is_same_v<Source, std::filesystem::path>)
	object load(Source&& src)
	{
		parser_context ctxt;

		while(true)
		{
			if(update(read_byte(src), ctxt) == parse_result::done)
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