//@	{"target":{"name":"object_deserializer.o"}}

#include "./object_deserializer.hpp"
#include "./variant_helper.hpp"
#include "./type_info.hpp"

#include <charconv>

std::pair<anon::parser_context::state, anon::object::mapped_type> anon::state_from_ctrl_word(std::string_view buffer)
{
	using variant_type = object::mapped_type;
	auto const index
		= variant_helper::find_type<variant_type>([buffer]<class T>(variant_helper::empty<T>){
		return buffer == type_info<T>::name();
	});

	if(index == std::variant_npos)
	{
		throw std::runtime_error{std::string{"Unsupported type '"}.append(buffer).append("'")};
	}

	std::pair<anon::parser_context::state, variant_type> ret{};
	variant_helper::on_type_index<variant_type>(index, [&ret]<class T>(variant_helper::empty<T>){
		ret = std::pair{type_info<T>::parser_init_state(), T{}};
	});

	return ret;
}

namespace anon::object_loader_detail
{
	template<class T>
	requires(std::is_floating_point_v<T> || std::is_integral_v<T>)
	void finalize(T& value, std::string const& src)
	{
		auto const begin = std::data(src);
		auto const end = begin + std::size(src);
		auto res = std::from_chars(begin, end, value);
		if(res.ec == std::errc{})
		{
		if(res.ptr != end)
			{
				throw std::runtime_error{"Junk after number"};
			}
			return;
		}

		switch(res.ec)
		{
			case std::errc::invalid_argument:
				throw std::runtime_error{std::string{src}.append(" is not convertable to a number")};
			case std::errc::result_out_of_range:
				throw std::runtime_error{std::string{src}
					.append(" does not fit in a ").append(type_info<T>::name())};
			default:
				__builtin_unreachable();
		}
	}

	constexpr bool is_whitespace(char val)
	{
		return val >= '\0' && val<= ' ';
	}

	void finalize(std::string& dest, std::string&& src)
	{
		dest = std::move(src);
	}

	template<class Dest>
	void finalize(std::vector<Dest>&, std::string&& src)
	{
		if(std::size(src) != 0)
		{ throw std::runtime_error{std::string{"Non-terminated array element "}.append(std::move(src))}; }
	}

	void finalize(object&, std::string&&)
	{
	}

	template<class Dest>
	requires(!std::ranges::range<Dest>
		|| std::is_same_v<std::decay_t<Dest>, std::string>
		|| std::is_same_v<std::decay_t<Dest>, object>)
	void append(Dest&, std::string&&)
	{
		throw std::runtime_error{"Multiple values require an array"};
	}

	template<class Dest>
	void append(std::vector<Dest>& dest, std::string&& src)
	{
		if constexpr(std::is_same_v<std::string, Dest>)
		{dest.push_back(std::move(src));}
		else
		{
			Dest tmp;
			finalize(tmp, src);
			dest.push_back(std::move(tmp));
		}
	}

	void append(std::vector<object>, std::string&&)
	{}
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
					ctxt.current_node.first = key{ctxt.current_key};
					ctxt.current_node.second = std::move(value);
					ctxt.current_state = state;
					ctxt.buffer.clear();
					if(std::holds_alternative<std::vector<object>>(ctxt.current_node.second))
					{
						ctxt.parent_nodes.push(std::move(ctxt.current_node));
						ctxt.current_node.second = object{};
					}
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
					ctxt.current_node.first = key{ctxt.current_key};
					ctxt.current_node.second = std::move(value);
					ctxt.current_state = state;
					ctxt.buffer.clear();
					if(std::holds_alternative<std::vector<object>>(ctxt.current_node.second))
					{
						ctxt.parent_nodes.push(std::move(ctxt.current_node));
						ctxt.current_node.second = object{};
					}
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
					if(val == '\0')
					{ throw std::runtime_error{"Null character detected in input stream"}; }
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

					if(auto item = std::get_if<std::vector<object>>(&ctxt.parent_nodes.top().second); item != nullptr)
					{
						if(std::size(std::get<object>(ctxt.current_node.second)) != 0)
						{ throw std::runtime_error{"Non-terminated array element"}; }

						ctxt.current_node = std::move(ctxt.parent_nodes.top());
						ctxt.parent_nodes.pop();
					}

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
					if(auto item = std::get_if<std::vector<object>>(&ctxt.parent_nodes.top().second); item != nullptr)
					{
						item->push_back(std::move(std::get<object>(ctxt.current_node.second)));
					}
					else
					{
						std::visit([buffer = std::move(ctxt.buffer)](auto& val) mutable {
							object_loader_detail::append(val, std::move(buffer));
						}, ctxt.current_node.second);
					}

					ctxt.current_state = ctxt.prev_state;
					break;

				default:
					if(val == '\0')
					{ throw std::runtime_error{"Null character detected in input stream"}; }
					ctxt.buffer += val;
					ctxt.current_state = ctxt.prev_state;
			}


	}
	return parse_result::more_data_needed;
}