#ifndef ANON_OBJECT_HPP
#define ANON_OBJECT_HPP

#include <variant>
#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace anon
{
	class object
	{
	public:
		template<class T>
		object& insert_or_assign(std::string&& key, T&& val) &
		{
			m_content.insert_or_assign(std::move(key), std::forward<T>(val));
			return *this;
		}

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
		object& insert(std::string&& key, T&& val) &
		{
			if(auto ip = m_content.insert(std::move(key), std::forward<T>(val)); ip.second)
			{
				return *this;
			}
			throw std::runtime_error{"Key already exists"};
		}

		template<class T>
		object&& insert_or_assign(std::string&& key, T&& val) &&
		{
			m_content.insert_or_assign(std::move(key), std::forward<T>(val));
			return std::move(*this);
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

		template<class T>
		object&& insert(std::string&& key, T&& val) &&
		{
			if(auto ip = m_content.insert(std::move(key), std::forward<T>(val)); ip.second)
			{
				return std::move(*this);
			}
			throw std::runtime_error{"Key already exists"};
		}

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

		bool contains(std::string_view key) const
		{
			return m_content.contains(key);
		}

		decltype(auto) find(std::string_view key) const
		{
			return m_content.find(key);
		}

		decltype(auto) find(std::string_view key)
		{
			return m_content.find(key);
		}

		decltype(auto) size() const
		{
			return std::size(m_content);
		}

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

		auto operator<=>(object const&) const = default;

	private:
		std::map<std::string,
		std::variant<object, std::string, int64_t, double,
			std::vector<object>, std::vector<std::string>, std::vector<int64_t>, std::vector<double>>,
			std::less<>> m_content;
	};
}

#endif