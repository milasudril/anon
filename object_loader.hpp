//@	{"dependencies_extra":[{"ref":"./object_loader.o","rel":"implementation"}]}

#ifndef ANON_OBJECTLOADER_HPP
#define ANON_OBJECTLOADER_HPP

#include "./object.hpp"

namespace anon
{
	struct parser_context
	{
		enum class state:int{init, ctrl_word, value};

		state current_state{state::init};
		state prev_state{state::init};
		std::string buffer;
		size_t level{0};
		object retval;
	};

	enum class parse_result{done, more_data_needed};

	parse_result update(std::optional<char> input, parser_context& ctxt);

	template<class Source>
	object load(Source&& src)
	{
		parser_context ctxt;

		while(true)
		{
			if(update(read_byte(src), ctxt) == parse_result::done)
			{
				return ctxt.retval;
			}
		}
	}
}

#endif