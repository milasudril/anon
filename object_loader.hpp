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
	};

	template<class Source>
	object load(Source&& src)
	{
		parser_context ctxt;

		while(true)
		{
			auto const result = read_byte(src);
			static_assert(std::is_same_v<std::decay_t<decltype(result)>, std::optional<char>>);
			if(!result.has_value())
			{
				if(ctxt.level != 0)
				{
					throw std::runtime_error{"Input data contains an non-terminated value"};
				}
				return object{};
			}

			auto const val = *result;

			switch(ctxt.current_state)
			{
				case parser_context::state::init:
					switch(val)
					{
						case '\\':
							ctxt.prev_state = ctxt.current_state;
							ctxt.current_state=parser_context::state::ctrl_word;
							break;

						default:
							if(!(val>='\0' && val<= ' '))
							{
								throw std::runtime_error{"Invalid input data"};
							}
					}
					break;

				case parser_context::state::ctrl_word:
					switch(val)
					{
						case '\\':
							if(ctxt.prev_state == parser_context::state::init)
							{ throw std::runtime_error{"Values may not occur here"};}

							ctxt.current_state = ctxt.prev_state;
							ctxt.buffer += val;
							break;

						case '{':
							++ctxt.level;
							ctxt.current_state = parser_context::state::value;
							printf("Control word: %s\n", ctxt.buffer.c_str());
							ctxt.buffer.clear();
							break;

						case '}':
							if(ctxt.prev_state != parser_context::state::value || ctxt.level == 0)
							{
								throw std::runtime_error{"No value here to end"};
							}
							--ctxt.level;
							printf("Value: %s\n", ctxt.buffer.c_str());
							ctxt.buffer.clear();
							if(ctxt.level == 0)
							{
								return object{};
							}
							break;

						default:
							ctxt.buffer += val;
							break;
					}
					break;

				case parser_context::state::value:
					switch(val)
					{
						case '\\':
							ctxt.prev_state = ctxt.current_state;
							ctxt.current_state=parser_context::state::ctrl_word;
							break;
						default:
							ctxt.buffer += val;
					}
					break;
			}
		}
	}
}

#endif