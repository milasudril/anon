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
 * \defgroup serialization
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

	template<class Entity, sink Sink>
	void store(Entity const& item, Sink&& sink);

	template<sink Sink>
	void store_body(object const& obj, Sink&& sink);

	template<class T, sink Sink>
	void store_body(std::vector<T> const& array, Sink&& sink);

	template<std::integral T, sink Sink>
	void store_body(T value, Sink&& sink);

	template<std::floating_point T, sink Sink>
	void store_body(T value, Sink&& sink);

	template<class Entity, sink Sink>
	void store(Entity const& item, Sink&& sink);

	template<sink Sink>
	void store_body(std::string_view value, Sink&& sink);

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
		std::array<char, std::numeric_limits<T>::digits10 + 2> buffer{};
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

	struct cfile_writer
	{
		FILE* sink;
	};

	inline void write(char ch, cfile_writer writer)
	{
		putc(ch, writer.sink);
	}

	inline void write(char const* buffer, cfile_writer writer)
	{
		fputs(buffer, writer.sink);
	}


	inline void store(object const& obj, std::filesystem::path const& path)
	{
		auto file_deleter = [](FILE* f){ return fclose(f); };
		std::unique_ptr<FILE, decltype(file_deleter)> sink{fopen(path.c_str(), "wb")};
		if(sink == nullptr)
		{
			throw std::runtime_error{std::string{"Failed to open file "}.append(path)};
		}
		store(obj, cfile_writer{sink.get()});
	}

	struct string_writer
	{
		std::string buffer;
	};

	void write(char ch, string_writer& buff)
	{
		buff.buffer += ch;
	}

	void write(char const* data, string_writer& buff)
	{
		buff.buffer += data;
	}

	inline auto to_string(object const& obj)
	{
		string_writer writer;
		store(obj, writer);
		return writer.buffer;
	}
}
#endif