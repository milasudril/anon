//@	{"target":{"name":"object_loader.o"}}

#include "./object_loader.hpp"

std::pair<anon::parser_context::state, anon::object::mapped_type> anon::state_from_ctrl_word(std::string_view buffer)
{
	if(buffer == "obj")
	{ return std::pair{parser_context::state::key, object{}}; }

	if(buffer == "str")
	{ return std::pair{parser_context::state::value, std::string{}}; }

	if(buffer == "str...")
	{ return std::pair{parser_context::state::value, std::vector<std::string>{}}; }

#if 0
	if(buffer == "int")
	{ return parser_context::state::integer_value; }

	if(buffer == "flt")
	{ return parser_context::state::float_value; }
#endif
	throw std::runtime_error{"Unsupported type"};
}

namespace anon::object_loader_detail
{
	constexpr bool is_whitespace(char val)
	{
		return val >= '\0' && val<= ' ';
	}

	template<class Dest>
	void finalize(Dest& dest, std::string&& src)
	{
		dest = std::move(src);
	}

	template<class Dest>
	void finalize(std::vector<Dest>& dest, std::string&& src)
	{
		dest.push_back(std::move(src));
	}

	void finalize(object&, std::string&&)
	{
	}

	template<class Dest>
	void append(Dest&, std::string&&)
	{
		throw std::runtime_error{"Multiple values require an array"};
	}

	template<class Dest>
	void append(std::vector<Dest>& dest, std::string&& src)
	{
		dest.push_back(std::move(src));
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
			if(!object_loader_detail::is_whitespace(val))
			{
				ctxt.current_state = parser_context::state::type_tag;
				ctxt.buffer += val;
			}
			break;

		case parser_context::state::type_tag:
			switch(val)
			{
				case '{':
				{
					++ctxt.level;
					auto [state, value] = state_from_ctrl_word(ctxt.buffer);
					ctxt.parent_nodes.push(std::move(ctxt.current_node));
					ctxt.current_node.first = std::move(ctxt.current_key);
					ctxt.current_node.second = std::move(value);
					ctxt.current_state = state;
					ctxt.buffer.clear();
					break;
				}

				default:
					if(object_loader_detail::is_whitespace(val))
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
				{
					++ctxt.level;
					auto [state, value] = state_from_ctrl_word(ctxt.buffer);
					ctxt.parent_nodes.push(std::move(ctxt.current_node));
					ctxt.current_node.first = std::move(ctxt.current_key);
					ctxt.current_node.second = std::move(value);
					ctxt.current_state = state;
					ctxt.buffer.clear();
					break;
				}
				default:
					if(!object_loader_detail::is_whitespace(val))
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
					ctxt.current_key = std::move(ctxt.buffer);
					break;

				case '\\':
					ctxt.prev_state = ctxt.current_state;
					ctxt.current_state = parser_context::state::ctrl_char;
					break;

				default:
					if(object_loader_detail::is_whitespace(val))
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
					ctxt.current_key = std::move(ctxt.buffer);
					break;

				default:
					if(!object_loader_detail::is_whitespace(val))
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
				{
					if(ctxt.level == 0)
					{
						throw std::runtime_error{"No value here to end"};
					}
					--ctxt.level;

					if(ctxt.level == 0)
					{ return parse_result::done; }

					std::visit([buffer = std::move(ctxt.buffer)](auto& val) mutable {
						object_loader_detail::finalize(val, std::move(buffer));
					}, ctxt.current_node.second);

					auto top_of_stack = std::move(ctxt.parent_nodes.top());
					ctxt.parent_nodes.pop();
					std::get<object>(top_of_stack.second).insert(std::move(ctxt.current_node.first), std::move(ctxt.current_node.second));
					ctxt.current_node = std::move(top_of_stack);

					if(ctxt.prev_state == parser_context::state::value)
					{ ctxt.current_state = parser_context::state::key;}
					else
					{ ctxt.current_state = ctxt.prev_state; }

					ctxt.buffer.clear();
					break;
				}

				case ';':
					std::visit([buffer = std::move(ctxt.buffer)](auto& val) mutable {
						object_loader_detail::append(val, std::move(buffer));
					}, ctxt.current_node.second);

					ctxt.current_state = ctxt.prev_state;
					break;

				default:
					ctxt.buffer += val;
					ctxt.current_state = ctxt.prev_state;
			}


	}
	return parse_result::more_data_needed;
}