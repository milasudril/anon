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
		 * \brief Destroys ctxt
		 *
		 * \ingroup de-serialization
		 */
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
	 * \brief Defines the current status of a stream
	 *
	 * \ingroup de-serialization
	 *
	 */
	enum class stream_status:char{ready, eof, blocking};

	/**
	 * \brief Holder for the result of a read operation
	 *
	 * \ingroup de-serialization
	 */
	struct read_result
	{
		/**
		 * \brief Holds the last value read from a input stream, only useful of if status is ready.
		 */
		char value;

		/**
		 * \brief Determines the status of the input stream
		 */
		stream_status status;
	};

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
		 * \brief Shall try to consume next byte and return a read_result with appropriate values
		 */
		{ read_byte(a) } -> std::same_as<read_result>;
	};

	using parser_context_handle =
		std::unique_ptr<deserializer_detail::parser_context, deserializer_detail::parser_context_deleter>;

	/**
	 * \brief Creates a new parser context
	 *
	 *  \ingroup de-serialization
	 */
	parser_context_handle create_parser_context();

	/**
	 * \brief Extracts the latest result from ctxt
	 *
	 * \ingroup de-serialization
	 */
	object::mapped_type&& take_result(deserializer_detail::parser_context& ctxt);

	/**
	 * \brief Extracts the latest result from ctxt, and resets ctxt to its initial state
	 *
	 * \ingroup de-serialization
	 */
	object::mapped_type&& take_result_and_reset(deserializer_detail::parser_context& ctxt);

	/**
	* \brief Holds the result after processing one byte
	*
	* \ingroup de-serialization
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
	parse_result update(char input, deserializer_detail::parser_context& ctxt);

	template<source Source>
	class async_loader
	{
	public:
		explicit async_loader(Source&& src):
			m_source{std::move(src)},
			m_parser_ctxt{create_parser_context()}
		{}

		template<class T>
		std::optional<T> try_read_next()
		{
			while(true)
			{
				auto const read_res = read_byte(m_source);
				switch(read_res.status)
				{
					case stream_status::ready:
						if(update(read_res.value, *m_parser_ctxt) == parse_result::done)
						{
							return std::get<T>(take_result_and_reset(*m_parser_ctxt));
						}
						break;

					case stream_status::eof:
						throw std::runtime_error{"Input data contains an non-terminated value"};

					case stream_status::blocking:
						return std::nullopt;
				}
			}
		}

	private:
		Source m_source;
		parser_context_handle m_parser_ctxt;
	};



	/**
	 * \brief Loads an object from src, and returns it. stream_status::blocking is treated
	 * as stream_status::eof.
	 *
	 * \ingroup de-serialization
	 */
	object load(source auto&& src)
	{
		auto ctxt = create_parser_context();

		while(true)
		{
			auto const read_res = read_byte(src);
			switch(read_res.status)
			{
				case stream_status::ready:
					if(update(read_res.value, *ctxt) == parse_result::done)
					{
						return std::get<object>(take_result(*ctxt));
					}
					break;

				case stream_status::eof:
				case stream_status::blocking:
					throw std::runtime_error{"Input data contains an non-terminated value"};
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
	 * \brief Reads one byte from src and returns it in a read_result
	 *
	 * \ingroup de-serialization
	 */
	inline read_result read_byte(cfile_reader src)
	{
		auto ch_in = getc(src.src);

		return read_result{
			static_cast<char>(ch_in),
			ch_in == EOF? stream_status::eof:stream_status::ready
		};
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