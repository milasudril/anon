//@	{"dependencies_extra":[{"ref":"./deserializer.o","rel":"implementation"}]}

#ifndef ANON_DESERIALIZER_HPP
#define ANON_DESERIALIZER_HPP

/**
 * \file deserializer.hpp
 *
 * \brief Contains declarations and definitions regarding de-serialization
 */

#include "./object.hpp"
#include "./type_info.hpp"

#include <stack>
#include <filesystem>
#include <optional>

/**
 * \defgroup de-serialization De-serialization
 */

namespace anon
{
	namespace deserializer_detail
	{
		/**
		* \brief Holds the current parsing context
		*
		* \ingroup de-serialization
		*/
		struct parser_context;

		/**
		* \brief Holds the result after processing one byte
		*
		* \ingroup de-serialization
		*
		*/
		enum class parse_result{done, more_data_needed};

		/**
		* \brief Processes input, and updates ctxt accordingly
		*
		* \note If an error occurs during processing of input, an exception is thrown
		*
		* \return parse_result::done if the outermost object was closed, otherwise
		*         parse_result::more_data_needed
		*
		* \ingroup de-serialization
		*/
		parse_result update(std::optional<char> input, parser_context& ctxt);

		void destroy_parser_context(parser_context* ctxt);

		struct parser_context_deleter
		{
			void operator()(parser_context* ctxt)
			{
				destroy_parser_context(ctxt);
			}
		};
	}

	/**
	 * \brief Defines the requirements of a "source"
	 *
	 * A source is something that can be read from, for example, an input buffer or a file.
	 *
	 * \ingroup de-serialization
	 *
	 */
	template<class T>
	concept source = requires(T a)
	{
		/**
		 * \brief Shall try to consume next byte and return it. If it possible to consume another
		 *  byte, it shall return std::nullopt, which implies an End-Of-File condition.
		 */
		{ read_byte(a) } -> std::same_as<std::optional<char>>;
	};

	using parser_context_handle =
		std::unique_ptr<deserializer_detail::parser_context, deserializer_detail::parser_context_deleter>;

	parser_context_handle create_parser_context();

	object::mapped_type&& take_result(deserializer_detail::parser_context& ctxt);

	/**
	 * \brief Loads an object from src, and returns it
	 *
	 * \ingroup de-serialization
	 */
	object load(source auto&& src)
	{
		auto ctxt = create_parser_context();

		while(true)
		{
			if(update(read_byte(src), *ctxt) == deserializer_detail::parse_result::done)
			{
				return std::get<object>(take_result(*ctxt));
			}
		}
	}

	/**
	 * \brief An adapter to make it possible to use the C file API when loading objects
	 *
	 * \ingroup de-serialization De-serialization
	 */
	struct cfile_reader
	{
		FILE* src;
	};

	/**
	 * \brief Reads one byte from src and returns it if not at End-Of-File, otherwise it returns
	 *  std::nullopt
	 *
	 * \ingroup de-serialization
	 */
	inline std::optional<char> read_byte(cfile_reader src)
	{
		auto ch_in = getc(src.src);
		return ch_in != EOF? static_cast<char>(ch_in): std::optional<char>{};
	}

	/**
	 * \brief Loads an object from the current position of src
	 *
	 * \ingroup de-serialization
	 */
	inline object load(FILE* src)
	{
		return load(cfile_reader{src});
	}

	/**
	 * \brief Loads an object path
	 *
	 * \ingroup de-serialization
	 */
	inline object load(std::filesystem::path const& path)
	{
		auto file_deleter = [](FILE* f){ return fclose(f); };
		std::unique_ptr<FILE, decltype(file_deleter)> src{fopen(path.c_str(), "rb")};
		if(src == nullptr)
		{
			throw std::runtime_error{std::string{"Failed to open file "}.append(path)};
		}
		return load(src.get());
	}
}

#endif