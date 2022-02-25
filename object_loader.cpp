//@	{"target":{"name":"object_loader.o"}}

#include "./object_loader.hpp"

anon::parse_result anon::update(std::optional<char> input, parser_context& ctxt)
{
	if(!input.has_value())
	{
		if(ctxt.level != 0)
		{
			throw std::runtime_error{"Input data contains an non-terminated value"};
		}
		return anon::parse_result::done;
	}

	auto const val = *input;

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
						return anon::parse_result::done;
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
	return anon::parse_result::more_data_needed;
}