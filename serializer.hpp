#ifndef ANON_SERIALIZER_HPP
#define ANON_SERIALIZER_HPP

/**
 * \file serializer.hpp
 *
 * \brief Contains declarations and definitions regarding serialization
 */

#include "./type_info.hpp"

#include <filesystem>
#include <cstdio>
#include <memory>
#include <array>
#include <charconv>
#include <cstring>

/**
 * \defgroup serialization Serialization
 */

namespace anon
{
	/**
	 * \brief Defines the requirements of a "sink"
	 *
	 * A sink is something that can be written to, for example, an output buffer or a file. It shall
	 * support writing single characters as well as C-style strings.
	 *
	 * \ingroup serialization
	 *
	 */
	template<class T>
	concept sink = requires(T a)
	{
		{ write(std::declval<char>(), a) } -> std::same_as<void>;
		{ write(std::declval<char const*>(), a) } -> std::same_as<void>;
	};

	/**
	 * \brief Writes item to sink, within type_info<Entity>::name()`{` and `\}`
	 *
	 * \ingroup serialization
	 */
	template<class Entity, sink Sink>
	void store(Entity const& item, Sink&& sink);

	/**
	 * \brief Writes obj to sink
	 *
	 * \ingroup serialization
	 */
	template<sink Sink>
	void store_body(object const& obj, Sink&& sink);

	/**
	 * \brief Writes `array` to sink
	 *
	 * This function writes `array` to sink. To separate the elements, the sequence `\;` is written
	 * directly after the element.
	 *
	 * \note Array elements are written using store_body, rather than store
	 *
	 * \ingroup serialization
	 */
	template<class T, sink Sink>
	void store_body(std::vector<T> const& array, Sink&& sink);

	/**
	 * \brief Writes value to sink
	 *
	 * \ingroup serialization
	 */
	template<std::integral T, sink Sink>
	void store_body(T value, Sink&& sink);

	/**
	 * \brief Writes value to sink
	 *
	 * \ingroup serialization
	 */
	template<std::floating_point T, sink Sink>
	void store_body(T value, Sink&& sink);

	/**
	 * \brief Writes value to sink
	 *
	 * This function writes a string `value` to sink. Any `\` is escaped with `\\`. If there is a
	 * null character in `value`, an exception is thrown.
	 *
	 * \ingroup serialization
	 */
	template<sink Sink>
	void store_body(std::string_view value, Sink&& sink);

	/**
	 * \brief Writes value to sink.
	 *
	 * \ingroup serialization
	 */
	template<sink Sink>
	void store_body(property_name const& value, Sink&& sink)
	{ write(value.c_str(), sink); }


	template<sink Sink>
	void store_body(object const& obj, Sink&& sink)
	{
		std::ranges::for_each(obj, [&sink](auto const& item){
			store_body(item.first, sink);
			write(':', sink);
			std::visit([&sink](auto const& item) {
				store(item, sink);
			}, item.second);
		});
	}

	template<class T, sink Sink>
	void store_body(std::vector<T> const& array, Sink&& sink)
	{
		std::ranges::for_each(array, [&sink](auto const& item){
			store_body(item, sink);
			write("\\;", sink);
		});
	}

	template<std::integral T, sink Sink>
	void store_body(T value, Sink&& sink)
	{
		std::array<char, std::numeric_limits<T>::digits10 + 2> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value);
		write(std::data(buffer), sink);
	}

	template<std::floating_point T, sink Sink>
	void store_body(T value, Sink&& sink)
	{
		std::array<char, std::numeric_limits<T>::digits10 + 7> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value, std::chars_format::scientific);
		write(std::data(buffer), sink);
	}

	template<sink Sink>
	void store_body(std::string_view value, Sink&& sink)
	{
		std::ranges::for_each(value, [&sink](auto item) {
			if(item == '\0')
			{ throw std::runtime_error{"Cannot serialize null characters"}; }

			if(item == '\\')
			{ write('\\', sink); }
			write(item, sink);
		});
	}

	template<class Entity, sink Sink>
	void store(Entity const& item, Sink&& sink)
	{
		write(type_info<Entity>::name(), sink);
		write('{', sink);
		store_body(item, sink);
		write("\\}", sink);
	}

	/**
	 * \brief An adapter to make it possible to use the C file API when storing objects
	 *
	 * \ingroup serialization
	 */
	struct cfile_writer
	{
		FILE* sink;
	};

	/**
	 * \brief Writes ch to the stream referred to by writer
	 *
	 * \ingroup serialization
	 */
	inline void write(char ch, cfile_writer writer)
	{
		putc(ch, writer.sink);
	}

	/**
	 * \brief Writes buffer to the stream referred to by writer
	 *
	 * \ingroup serialization
	 */
	inline void write(char const* buffer, cfile_writer writer)
	{
		fputs(buffer, writer.sink);
	}

	/**
	 * \brief Stores obj to dest
	 *
	 * \ingroup serialization
	 */
	inline void store(object const& obj, FILE* dest)
	{
		store(obj, cfile_writer{dest});
	}

	/**
	 * \brief Stores obj to path
	 *
	 * \note If the file already exists, it is overwritten
	 *
	 * \ingroup serialization
	 */
	inline void store(object const& obj, std::filesystem::path const& path)
	{
		auto file_deleter = [](FILE* f){ return fclose(f); };
		std::unique_ptr<FILE, decltype(file_deleter)> sink{fopen(path.c_str(), "wb")};
		if(sink == nullptr)
		{
			throw std::runtime_error{std::string{"Failed to open file "}.append(path)};
		}
		store(obj, sink.get());
	}

	/**
	 * \brief An adapter to make it possible to write objects to an std::string
	 *
	 * \ingroup serialization
	 */
	struct string_writer
	{
		std::reference_wrapper<std::string> buffer;
	};

	/**
	 * \brief Writes ch to the string referred to by writer
	 *
	 * \ingroup serialization
	 */
	void write(char ch, string_writer writer)
	{
		writer.buffer.get() += ch;
	}

	/**
	 * \brief Writes data to the string referred to by writer
	 *
	 * \ingroup serialization
	 */
	void write(char const* data, string_writer writer)
	{
		writer.buffer.get() += data;
	}

	/**
	 * \brief Generates a string representation of obj
	 *
	 * \ingroup serialization
	 */
	inline auto to_string(object const& obj)
	{
		std::string ret;
		store(obj, string_writer{ret});
		return ret;
	}
}
#endif