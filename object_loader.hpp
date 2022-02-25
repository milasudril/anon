#ifndef ANON_OBJECTLOADER_HPP
#define ANON_OBJECTLOADER_HPP

#include "./object.hpp"

namespace anon
{
	template<class Source>
	object load(Source&& src)
	{
		enum class state:int{init, ctrl_word, value};
		auto current_state = state::init;
		auto state_prev = current_state;
		std::string buffer;
		size_t level = 0;
		while(true)
		{
			auto val = read_byte(src);
			static_assert(std::is_same_v<val, std::optional<char>>);
			if(!val.has_value())
			{
				if(level != 0)
				{
					throw std::runtime_error{"Input data contains an non-terminated value"};
				}
				return object{};
			}

			switch(state)
			{
				case state::init:
					switch(val)
					{
						case '\\':
							state_prev = state;
							state=state::ctrl_word;
							break;

						default:
							if(!(val>='\0' && val<= ' '))
							{
								throw std::runtime_error{"Invalid input data"};
							}
					}
					break;

				case state::ctrl_word:
					switch(val)
					{
						case '\\':
							if(state_prev == state::init)
							{ throw std::runtime_error{"Values may not occur here"}}

							state = state_prev;
							buffer += val;
							break;

						case '{':
							++level;
							state = state::value;
							printf("Control word: %s\n", buffer.c_str());
							buffer.clear();
							break;

						case '}':
							if(state_prev != state::value || level == 0)
							{
								throw std::runtime_error{"No value here to end"};
							}
							--level;
							printf("Value: %s\n", buffer.c_str());
							buffer.clear();
							if(level == 0)
							{
								return object{};
							}
							break;

						default:
							buffer += ch_in;
							break;
					}
					break;

				case state::value:
					switch(val)
					{
						case '\\':
							state_prev = state;
							state=state::ctrl_word;
							break;
					}
					break;
			}
		}
	}
}

#endif