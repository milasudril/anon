//@	{"target":{"name":"object_loader.o"}}

#include "./object_loader.hpp"

anon::parser_context::state anon::state_from_ctrl_word(std::string_view buffer)
{
	if(buffer == "obj")
	{ return parser_context::state::key; }

	if(buffer == "str")
	{ return parser_context::state::value; }
#if 0

	if(buffer == "int")
	{ return parser_context::state::integer_value; }

	if(buffer == "flt")
	{ return parser_context::state::float_value; }
#endif
	throw std::runtime_error{"Unsupported type"};
}

namespace
{
	constexpr bool is_whitespace(char val)
	{
		return val >= '\0' && val<= ' ';
	}
}

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
	putchar(val);

	switch(ctxt.current_state)
	{
		case parser_context::state::init:
			if(!is_whitespace(val))
			{
				ctxt.current_state = parser_context::state::type_tag;
				ctxt.buffer += val;
			}
			break;

		case parser_context::state::type_tag:
			switch(val)
			{
				case '{':
					++ctxt.level;
					ctxt.current_state = state_from_ctrl_word(ctxt.buffer);
					ctxt.buffer.clear();
					break;

				default:
					if(is_whitespace(val))
					{
						ctxt.current_state = parser_context::state::after_type_tag;
					}
					else
					{ ctxt.buffer += val; }
			}
			break;

		case parser_context::state::after_type_tag:
			switch(val)
			{
				case '{':
					++ctxt.level;
					ctxt.current_state = state_from_ctrl_word(ctxt.buffer);
				//	printf("%d\n", static_cast<int>(ctxt.current_state));
					ctxt.buffer.clear();
					break;
				default:
					if(!is_whitespace(val))
					{
						throw std::runtime_error{"Junk after type tag"};
					}
			}
			break;

		case parser_context::state::key:
			switch(val)
			{
				case ':':
					ctxt.current_state = parser_context::state::init;
					printf("[%s]", ctxt.buffer.c_str());
					ctxt.buffer.clear();
					break;

				case '\\':
					ctxt.prev_state = ctxt.current_state;
					ctxt.current_state = parser_context::state::ctrl_char;
					break;

				default:
					if(is_whitespace(val))
					{
						if(std::size(ctxt.buffer) != 0)
						{
							ctxt.current_state = parser_context::state::after_key;
						}
					}
					else
					{
						ctxt.buffer += val;
					}
			}
			break;

		case parser_context::state::after_key:
			switch(val)
			{
				case ':':
					ctxt.current_state = parser_context::state::init;
					printf("[%s]", ctxt.buffer.c_str());
					ctxt.buffer.clear();
					break;

				default:
					if(!is_whitespace(val))
					{
						throw std::runtime_error{"Junk after key"};
					}
			}
			break;

		case parser_context::state::value:
			switch(val)
			{
				case '\\':
					ctxt.prev_state = ctxt.current_state;
					ctxt.current_state = parser_context::state::ctrl_char;
					break;

				default:
					ctxt.buffer += val;
			}
			break;

		case parser_context::state::ctrl_char:
			switch(val)
			{
				case '}':
					if(ctxt.level == 0)
					{
						throw std::runtime_error{"No value here to end"};
					}
					--ctxt.level;
					if(ctxt.level == 0)
					{ return parse_result::done; }

					if(ctxt.prev_state == parser_context::state::value)
					{ ctxt.current_state = parser_context::state::key;}
					else
					{ ctxt.current_state = ctxt.prev_state; }

					printf("[%s]", ctxt.buffer.c_str());
					ctxt.buffer.clear();
					break;

				default:
					ctxt.buffer += val;
					ctxt.current_state = ctxt.prev_state;
			}

	}
	return parse_result::more_data_needed;
}