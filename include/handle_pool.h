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

		// Pool Iterator
		// @brief iterate over all data elements in the pool, skipping over empty slots
		// WARNING: this is very inefficient and should generally not be used, for quicker iteration consider using handle_array instead.
		struct Iterator
		{
			const HandlePool<T>* pool = nullptr;
			std::size_t index;

			Iterator(const HandlePool<T>* p, std::size_t i) : pool(p), index(i) {
				skip_dead();
			}

			void skip_dead() {
				while (index < pool->m_data.size() && pool->empty(index))
				++index;
			}

			T operator*() const { return pool->m_data[index]; }
			T* operator->() const { return &pool->m_data[index]; }

			Iterator& operator++() {
				++index;
				skip_dead();
				return *this;
			}

			Iterator operator++(int) {
				Iterator tmp(*this);
				++(*this);
				return tmp;
			}

			bool operator==(const Iterator& other) const {
				return pool == other.pool && index == other.index;
			}

			bool operator!=(const Iterator& other) const {
				return !(*this == other);
			}
		}; // end Iterator

		inline Iterator begin() const { return Iterator(this, 0); }
		inline Iterator end() const { return Iterator(this, m_data.size()); }

		inline Iterator cbegin() const { return begin(); }
		inline Iterator cend() const { return end(); }

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
				throw std::invalid_argument("Unable to remove, handle is invalid!");

			m_freelist.push_back(handle.index);
			m_generations[handle.index]++;
		}


	private:

		inline bool empty(index_t index) const {
			for (index_t idx : m_freelist) {
				if (idx == index) {
					return true;
				}
			}
			return false;
		}

		std::vector<T> m_data;
		std::vector<index_t> m_generations;
		std::vector<index_t> m_freelist;
	};
}
