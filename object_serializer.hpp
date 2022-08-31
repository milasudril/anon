#ifndef ANON_OBJECTSERIALIZER_HPP
#define ANON_OBJECTSERIALIZER_HPP

#include "./object.hpp"

#include <filesystem>
#include <cstdio>
#include <memory>
#include <array>
#include <charconv>
#include <cstring>

namespace anon
{
	template<class T>
	struct type_info;

	template<>
	struct type_info<object>
	{
		static constexpr char const* name(){ return "obj"; }
	};

	template<>
	struct type_info<int32_t>
	{
		static constexpr char const* name(){ return "i32"; }
	};

	template<>
	struct type_info<int64_t>
	{
		static constexpr char const* name(){ return "i64"; }
	};

	template<>
	struct type_info<uint32_t>
	{
		static constexpr char const* name(){ return "u32"; }
	};

	template<>
	struct type_info<uint64_t>
	{
		static constexpr char const* name(){ return "u64"; }
	};

	template<>
	struct type_info<float>
	{
		static constexpr char const* name(){ return "f32"; }
	};

	template<>
	struct type_info<double>
	{
		static constexpr char const* name(){ return "f64"; }
	};

	template<>
	struct type_info<std::string>
	{
		static constexpr char const* name(){ return "str"; }
	};

	template<class T>
	struct type_info<std::vector<T>>
	{
		static constexpr char const* name() { return std::data(namebuff); }

		static constexpr auto make_namebuff()
		{
			constexpr auto N = strlen(type_info<T>::name());
			std::array<char, N + 4> ret{};
			std::copy_n(type_info<T>::name(), N, std::data(ret));
			std::copy_n("...", 4, std::data(ret) + N);
			return ret;
		}

		static constexpr auto namebuff = make_namebuff();
	};

	template<class Entity, class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store(Entity const& item, Sink&& sink);

	template<class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store_body(object const& obj, Sink&& sink);

	template<class T, class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store_body(std::vector<T> const& array, Sink&& sink);

	template<class T, class Sink>
	requires(std::is_integral_v<T> && !std::is_same_v<Sink, std::filesystem::path>)
	void store_body(T value, Sink&& sink);

	template<class T, class Sink>
	requires(std::is_floating_point_v<T> && !std::is_same_v<Sink, std::filesystem::path>)
	void store_body(T value, Sink&& sink);

	template<class Entity, class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store(Entity const& item, Sink&& sink);

	template<class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store_body(std::string_view value, Sink&& sink);



	template<class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store_body(object const& obj, Sink&& sink)
	{
		std::ranges::for_each(obj, [&sink](auto const& item){
			write(item.first.c_str(), sink);
			write(':', sink);
			std::visit([&sink](auto const& item) {
				store(item, sink);
			}, item.second);
		});
	}

	template<class T, class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store_body(std::vector<T> const& array, Sink&& sink)
	{
		std::ranges::for_each(array, [&sink](auto const& item){
			store_body(item, sink);
			write("\\;", sink);
		});
	}

	template<class T, class Sink>
	requires(std::is_integral_v<T> && !std::is_same_v<Sink, std::filesystem::path>)
	void store_body(T value, Sink&& sink)
	{
		std::array<char, std::numeric_limits<T>::digits10 + 2> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value);
		write(std::data(buffer), sink);
	}

	template<class T, class Sink>
	requires(std::is_floating_point_v<T> && !std::is_same_v<Sink, std::filesystem::path>)
	void store_body(T value, Sink&& sink)
	{
		std::array<char, std::numeric_limits<T>::digits10 + 2> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value, std::chars_format::scientific);
		write(std::data(buffer), sink);
	}

	template<class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
	void store_body(std::string_view value, Sink&& sink)
	{
		std::ranges::for_each(value, [&sink](auto item) {
			if(item == '\\')
			{ write('\\', sink); }
			write(item, sink);
		});
	}

	template<class Entity, class Sink>
	requires(!std::is_same_v<Sink, std::filesystem::path>)
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