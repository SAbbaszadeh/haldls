#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "haldls/vx/common.h"
#include "haldls/vx/container.h"
#include "hate/visibility.h"
#include "stadls/vx/genpybind.h"
#include "stadls/vx/playback_program.h"

namespace fisch::vx {
class PlaybackProgramBuilder;
} // namespace fisch::vx

namespace stadls {
namespace vx GENPYBIND_TAG_STADLS_VX {

class PlaybackProgram;

/**
 * Sequential PlaybackProgram builder.
 */
class GENPYBIND(visible) PlaybackProgramBuilder
{
public:
	/** Default constructor. */
	PlaybackProgramBuilder() SYMBOL_VISIBLE;

	/**
	 * Add instruction to block execution until specified timer has reached specified value.
	 * @param coord Timer coordinate for which to wait
	 * @param time Timer value until which to block execution
	 */
	void wait_until(
	    typename haldls::vx::Timer::coordinate_type const& coord,
	    haldls::vx::Timer::Value time) SYMBOL_VISIBLE;

#define PLAYBACK_CONTAINER(Name, Type)                                                             \
	/**                                                                                            \
	 * Add instructions to write given container to given location.                                \
	 * @param coord Coordinate value selecting location                                            \
	 * @param config Container configuration data                                                  \
	 * @param backend Backend selection                                                            \
	 */                                                                                            \
	void write(                                                                                    \
	    typename Type::coordinate_type const& coord, Type const& config,                           \
	    haldls::vx::Backend backend) SYMBOL_VISIBLE;                                               \
                                                                                                   \
	/**                                                                                            \
	 * Add instructions to write given container to given location.                                \
	 * The container's default backend is used.                                                    \
	 * @param coord Coordinate value selecting location                                            \
	 * @param config Container configuration data                                                  \
	 * @note This function without backend parameter is needed due to python wrapping not being    \
	 * able to handle templated default arguments.                                                 \
	 */                                                                                            \
	void write(typename Type::coordinate_type const& coord, Type const& config) SYMBOL_VISIBLE;    \
                                                                                                   \
	/**                                                                                            \
	 * Add instructions to read container data from given location.                                \
	 * @param coord Coordinate value selecting location                                            \
	 * @param backend Backend selection                                                            \
	 */                                                                                            \
	PlaybackProgram::ContainerTicket<Type> read(                                                   \
	    typename Type::coordinate_type const& coord, haldls::vx::Backend backend) SYMBOL_VISIBLE;  \
                                                                                                   \
	/**                                                                                            \
	 * Add instructions to read container data from given location.                                \
	 * The container's default backend is used.                                                    \
	 * @param coord Coordinate value selecting location                                            \
	 * @note This function without backend parameter is needed due to python wrapping not being    \
	 * able to handle templated default arguments.                                                 \
	 */                                                                                            \
	PlaybackProgram::ContainerTicket<Type> read(typename Type::coordinate_type const& coord)       \
	    SYMBOL_VISIBLE;
#include "haldls/vx/container.def"

	/**
	 * Close PlaybackProgram build process and return executable program.
	 * @return Executable PlaybackProgram
	 */
	PlaybackProgram done() SYMBOL_VISIBLE;

	GENPYBIND(stringstream)
	friend std::ostream& operator<<(std::ostream& os, PlaybackProgramBuilder const& builder)
	    SYMBOL_VISIBLE;

private:
	template <class T, size_t... SupportedBackendIndex>
	void write_table_generator(
	    typename T::coordinate_type const& coord,
	    T const& config,
	    size_t backend_index,
	    std::index_sequence<SupportedBackendIndex...>);

	template <class T, size_t... SupportedBackendIndex>
	PlaybackProgram::ContainerTicket<T> read_table_generator(
	    typename T::coordinate_type const& coord,
	    size_t backend_index,
	    std::index_sequence<SupportedBackendIndex...>) SYMBOL_VISIBLE;

	std::unique_ptr<fisch::vx::PlaybackProgramBuilder> m_builder_impl;
};

} // namespace vx
} // namespace stadls