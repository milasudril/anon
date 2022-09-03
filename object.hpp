#ifndef ANON_OBJECT_HPP
#define ANON_OBJECT_HPP

/**
 * \file object.hpp
 *
 * \brief Contains the definition of the class object, and its associated functions
 */

#include "./property_name.hpp"

#include <variant>
#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstdint>

/**
 * \defgroup objects Objects
 *
 * Objects maps \ref property_names to values. The mapping is done such that each property name is
 * mapped to a uniquely defined property value. The values are sorted by the corresponding property
 * name, as if `LC_COLLATE=C`. The storage type of property values is an instantiation of
 * var_with_arrays, and thus, multiple types are supported. The supported types are defined by
 * \ref object::mapped_type.
 *
 */
namespace anon
{
	/**
	 * \brief Wrapper that generates a variant of single-value types, together with a matching array
	 * type
	 *
	 * This alias generates a variant, such that each argument is passed to std::variant both as is,
	 * and inside an std::vector. For example, with `var_with_arrays<int, double>` becomes
	 *
	 * ```
	 * std::variant<int, double, std::vector<int>, std::vector<double>>
	 * ```
	 *
	 * \ingroup objects
	 *
	 */
	template<class ... Args>
	using var_with_arrays = std::variant<Args..., std::vector<Args>...>;

	/**
	 * \brief Representation of \ref objects
	 *
	 * This class represents an object. It is designed on top of a suitable instantiation of
	 * `std::map`, and provides a similar interface. The main difference is that `operator[]` of an
	 * object is "safe", and does not allow insertion of new properties. Instead, it behaves like
	 * `std::map::at`, with the difference that a different exception is thrown when the key is not
	 * found.
	 *
	 * \ingroup objects
	 *
	 */
	class object
	{
	public:
		/**
		 * \brief Type used to represent a property value
		 *
		 * This type is used to represent property values. A property can be of any of the types
		 * listed in this alias, and also the corresponding array type, as generated by
		 * \ref var_with_arrays.
		 *
		 */
		using mapped_type = var_with_arrays<int32_t, int64_t, uint32_t, uint64_t, float, double,
			std::string, object>;

		/**
		 * \brief The key type used for element lookup
		 */
		using key_type = property_name;

		/**
		 * \brief The container element type
		 */
		using value_type = std::pair<key_type const, mapped_type>;

		/**
		 * \name insert_or_assign
		 *
		 * \brief Inserts or updates a property with name key, and sets it value to val
		 *
		 * \return *this, to allow method chaining
		 *
		 */
		///@{
		template<class T>
		object& insert_or_assign(key_type&& key, T&& val) &
		{
			m_content.insert_or_assign(std::move(key), std::forward<T>(val));
			return *this;
		}

		template<class T>
		object&& insert_or_assign(key_type&& key, T&& val) &&
		{
			m_content.insert_or_assign(std::move(key), std::forward<T>(val));
			return std::move(*this);
		}
		///@}

		/**
		 * \name assign
		 *
		 * \brief Updates an existing property with name key, so it holds val
		 *
		 * \note If the property does not exist, an exception is thrown
		 *
		 * \return *this, to allow method chaining
		 *
		 */
		///@{
		template<class T>
		object& assign(std::string_view key, T&& val) &
		{
			if(auto i = m_content.find(key); i!= std::end(m_content))
			{
				i->second = std::forward<T>(val);
				return *this;
			}
			throw std::runtime_error{"Key not found"};
		}

		template<class T>
		object&& assign(std::string_view key, T&& val) &&
		{
			if(auto i = m_content.find(key); i!= std::end(m_content))
			{
				i->second = std::forward<T>(val);
				return std::move(*this);
			}
			throw std::runtime_error{"Key not found"};
		}
		///@}

		/**
		 * \name insert
		 *
		 * \brief Inserts a property with name key, and sets its value to val
		 *
		 * \note If the property already exists, an exception is thrown
		 *
		 * \return *this, to allow method chaining
		 *
		 */
		///@{
		template<class T>
		object& insert(key_type&& key, T&& val) &
		{
			if(auto ip = m_content.insert(std::pair{std::move(key), std::forward<T>(val)}); ip.second)
			{
				return *this;
			}
			throw std::runtime_error{"Key already exists"};
		}

		template<class T>
		object&& insert(key_type&& key, T&& val) &&
		{
			if(auto ip = m_content.insert(std::move(key), std::forward<T>(val)); ip.second)
			{
				return std::move(*this);
			}
			throw std::runtime_error{"Key already exists"};
		}
		///@}

		/**
		 * \name operator[]
		 *
		 * \brief Retrieves the value of an existing property
		 *
		 * \note If the property does not exist, an exception is thrown
		 *
		 * \return A reference to the property value
		 *
		 */
		///@{
		auto const& operator[](std::string_view key) const
		{
			if(auto i = m_content.find(key); i != std::end(m_content))
			{
				return i->second;
			}
			throw std::runtime_error{"Key not found"};
		}

		auto& operator[](std::string_view key)
		{
			if(auto i = m_content.find(key); i != std::end(m_content))
			{
				return i->second;
			}
			throw std::runtime_error{"Key not found"};
		}
		///@}

		/**
		 * \brief Checks whether or not the object has an property with name key
		 *
		 * \return true if and only if the property exists, otherwise false
		 *
		 */
		bool contains(std::string_view key) const
		{
			return m_content.contains(key);
		}

		/**
		 * \name find
		 *
		 * \brief Looks up a property with name key as if calling `std::map::find`
		 *
		 */
		///@{
		decltype(auto) find(std::string_view key) const
		{
			return m_content.find(key);
		}

		decltype(auto) find(std::string_view key)
		{
			return m_content.find(key);
		}
		///@}

		/**
		 * \brief Returns the number of properties this object has
		 */
		decltype(auto) size() const
		{
			return std::size(m_content);
		}

		/**
		 * \name Iterator access
		 *
		 */
		///@{
		decltype(auto) begin() const
		{
			return std::begin(m_content);
		}

		decltype(auto) begin()
		{
			return std::begin(m_content);
		}

		decltype(auto) end() const
		{
			return std::end(m_content);
		}

		decltype(auto) end()
		{
			return std::end(m_content);
		}
		///@}

		auto operator<=>(object const&) const = default;

	private:
		std::map<key_type, mapped_type, std::less<>> m_content;
	};
}

#endif