// Copyright(c) 2026, Jeroen Hoogers
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <stdexcept>
#include <vector>
#include <cassert>
#include "handle.h"

namespace hndl
{
	template <typename T>
	class HandlePool
	{
	public:

		HandlePool() { }

		HandlePool(std::vector<T>&& data) : m_data(std::move(data)), m_generations(data.size()) { }

		inline bool has(Handle<T> handle) const {
			// bounds & generation check
			if (handle.index >= m_data.size() || handle.generation != m_generations[handle.index])
				return false;

			return true;
		}

		inline index_t size() const {
			return m_data.size();
		}

		inline T* get(Handle<T> handle) {
			if (!has(handle))
				return nullptr;

			return &m_data[handle.index];
		}

		inline const T* get(Handle<T> handle) const {
			if (!has(handle))
				return nullptr;

			return &m_data[handle.index];
		}

		inline void clear()
		{
			m_data.clear();
			m_generations.clear();
			m_freelist.clear();
		}

		inline void reserve(index_t size) {
			m_data.reserve(size);
			m_generations.reserve(size);
		}

		/// @brief Adds a new object to the pool
		/// @param data the object to add
		/// @return The handle to the newly added object
		inline Handle<T> add(T&& data)
		{
			Handle<T> handle;

			// grow data array if free list is empty
			if (m_freelist.empty())
			{
				assert(m_data.size() <= std::numeric_limits<index_t>::max());

				handle.index = m_data.size();
				handle.generation = 0;

				m_data.push_back(std::move(data));
				m_generations.push_back(handle.generation);
			}
			else
			{
			    // replace value at freelist index
				handle.index = m_freelist.back();
				m_freelist.pop_back();

				handle.generation = m_generations[handle.index];
				m_data[handle.index] = data;
			}

			assert(m_data.size() == m_generations.size());

			return handle;
		}

		/// @brief Adds a new object to the pool
		/// @param data the object to add
		/// @return The handle to the newly added object
		inline Handle<T> add(T& data) {
			T tmp = data;
			return add(std::move(tmp));
		}

		/// @brief Removes an object from the pool by handle
		/// @param handle the handle of the object to remove
		inline void remove(Handle<T> handle)
		{
			if(!has(handle))
				throw std::invalid_argument("remove() - invalid ID specified");

			m_freelist.push_back(handle.index);
			m_generations[handle.index]++;
		}

	private:
		std::vector<T> m_data;
		std::vector<index_t> m_generations;
		std::vector<index_t> m_freelist;
	};
}
