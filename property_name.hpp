#ifndef ANON_PROPERTYNAME_HPP
#define ANON_PROPERTYNAME_HPP

/**
 * \file property_name.hpp
 *
 * \brief Contains the definition of the class property_name, and its associated functions
 */

#include <algorithm>
#include <string>
#include <stdexcept>

/**
 * \defgroup property_names Property names
 *
 * Property names are used to identify object properties, and to retrieve their value. For a
 * property name to be valid, it must
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

namespace anon
{
	/**
	 * \brief Checks whether or not str can be used as a property_name
	 *
	 * \returns true, if and only if str can be used as a property_name, otherwise false
	 *
	 * \ingroup property_names
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

	/**
	 * \brief Storage class for a Property name
	 *
	 * \ingroup property_names
	 *
	 */
	class property_name
	{
	public:
		property_name() = default;

		/**
		 * \brief Sets a property name from src
		 *
		 * This constructor initiates the property name with the value from src. If the content of
		 * src cannot be used as a property name, it throws an exception
		 *
		 */
		explicit property_name(std::string_view src)
		{
			if(!is_valid_property_name(src))
			{throw std::runtime_error{std::string{"Malformed property name '"}.append(src).append("'")};}

			m_val = src;
		}

		/**
		 * \brief Returns a pointer to a c string, containing the property name
		 */
		char const* c_str() const
		{ return m_val.c_str(); }

		/**
		 * \brief Returns an iterator to the first element of the property name
		 */
		auto begin() const
		{ return std::begin(m_val); }

		/**
		 * \brief Returns an iterator to the last element after the last of the property name
		 */
		auto end() const
		{ return std::end(m_val); }

		/**
		 * \brief Returns the number of characters in the property name
		 */
		auto size() const
		{ return std::size(m_val); }

		/**
		 * \brief Default comparison operator
		 */
		auto operator<=>(property_name const&) const = default;

		/**
		 * \brief Converts the property name into a std::string_view
		 */
		operator std::string_view() const
		{
			return static_cast<std::string_view>(m_val);
		}

	private:
		std::string m_val;
	};
}

#endif