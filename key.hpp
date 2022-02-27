#ifndef ANON_KEY_HPP
#define ANON_KEY_HPP

#include <algorithm>
#include <string>
#include <stdexcept>

namespace anon
{
	inline bool is_valid_key(std::string_view str)
	{
		if(std::size(str) >= 32)
		{ return false; }

		if(str.starts_with("__"))
		{ return false; }

		return std::all_of(std::begin(str), std::end(str), [](auto const item) {
			return (item>= 'a' && item <='z') || item == '_';
		});
	}

	class key
	{
	public:
		key() = default;

		explicit key(std::string_view src)
		{
			if(!is_valid_key(src))
			{throw std::runtime_error{std::string{"Malformed key value '"}.append(src).append("'")};}

			m_val = src;
		}

		char const* c_str() const
		{ return m_val.c_str(); }

		auto begin() const
		{ return std::begin(m_val); }

		auto end() const
		{ return std::end(m_val); }

		auto size() const
		{ return std::size(m_val); }

		auto operator<=>(key const&) const = default;

		operator std::string_view() const
		{
			return static_cast<std::string_view>(m_val);
		}

	private:
		std::string m_val;
	};
}

#endif