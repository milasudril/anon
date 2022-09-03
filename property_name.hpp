#ifndef ANON_PROPERTYNAME_HPP
#define ANON_PROPERTYNAME_HPP

#include <algorithm>
#include <string>
#include <stdexcept>

namespace anon
{
	/**
	 * \brief Checks whether or not str can be used as a property_name
	 *
	 * This function checks whether or not str can be used as a property_name. For this to be true,
	 * it must
	 *
	 * * Be shorter than 32 characters
	 *
	 * * Must not be empty
	 *
	 * * Not start with two `_`
	 *
	 * * Must not start with a digit
	 *
	 * * Must only contain lowercase a to z, `_`, and digits
	 *
	 */
	constexpr bool is_valid_property_name(std::string_view str)
	{
		if(std::size(str) >= 32)
		{ return false; }

		if(str.starts_with("__"))
		{ return false; }

		if(std::size(str)!=0 && (*std::begin(str)>='0' && *std::begin(str) <= '9'))
		{ return false; }

		return std::all_of(std::begin(str), std::end(str), [](auto const item) {
			return (item>= 'a' && item <='z') || item == '_' || (item>='0' && item <= '9');
		});
	}

	class property_name
	{
	public:
		property_name() = default;

		explicit property_name(std::string_view src)
		{
			if(!is_valid_property_name(src))
			{throw std::runtime_error{std::string{"Malformed property name '"}.append(src).append("'")};}

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

		auto operator<=>(property_name const&) const = default;

		operator std::string_view() const
		{
			return static_cast<std::string_view>(m_val);
		}

	private:
		std::string m_val;
	};
}

#endif