// Copyright(c) 2026, Jeroen Hoogers
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#pragma once

#include <limits>
#include <cstdint>

namespace hndl
{
	typedef uint16_t index_t;	// TODO: allow user defined index type uint32_t or uint64_t, and consider separating geneneration_t

	template <typename T>
	struct Handle {
		index_t index;
		index_t generation;
	};

	template <typename T>
	constexpr Handle<T> NullHandle = {
		.index = std::numeric_limits<index_t>::max(),
		.generation = std::numeric_limits<index_t>::max()
	};

	template <typename T>
	bool operator==(const Handle<T>& lhs, const Handle<T>& rhs)
	{
		return lhs.index == rhs.index && lhs.generation == rhs.generation;
	}
}
