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
	class HandleArray
	{
	public:

		HandleArray() { }

		HandleArray(std::vector<T>&& data) : m_data(std::move(data))
		{
			m_slots.resize(m_data.size());
			m_handles.resize(m_data.size());

			for (index_t i = 0; i < m_data.size(); i++)
			{
				m_slots[i].index = i;
				m_slots[i].generation = 0;
				m_handles[i] = Handle<T>{i, 0 };
			}
		}

		/// @brief Checks whether the given handle is still valid and points to a value in the array
		/// @param the handle
		/// @return true if the handle still points to a valid value, false otherwise
		inline bool has(Handle<T> handle) const {
			// bounds & generation check
			if (handle.index >= m_slots.size() || handle.generation != m_slots[handle.index].generation || m_slots[handle.index].index >= m_data.size()) {
				return false;
			}

			return true;
		}


		/// @brief Returns the packed index for the specified handle
		/// @param the handle
		/// @return index of the value in the packed `data()` vector
		inline index_t indexof(Handle<T> handle) const {
			if (!has(handle))
				throw std::invalid_argument("Invalid handle specified!");

			return m_slots[handle.index].index;
		}

		/// @brief Returns a ptr to the value belonging to the specified handle
		/// @param the handle
		/// @return T* ptr to the value or nullptr
		inline T* get(Handle<T> handle) {
			if (!has(handle))
				return nullptr;

			auto idx = m_slots[handle.index].index;
			assert(handle == m_handles[idx]);

			return &m_data[idx];
		}

		/// @brief Returns a const ptr to the value belonging to the specified handle
		/// @param the handle
		/// @return const T* ptr to the value or nullptr
		inline const T* get(Handle<T> handle) const {
			if (!has(handle))
				return nullptr;

			auto idx = m_slots[handle.index].index;
			assert(handle == m_handles[idx]);

			return &m_data[idx];
		}

		inline index_t size() const {
			return m_data.size();
		}

		inline std::vector<T>& data() {
			return m_data;
		}

		inline const std::vector<T>& data() const {
			return m_data;
		}

		inline const std::vector<Handle<T>>& handles () const {
			return m_handles;
		}

		inline void clear()
		{
			m_data.clear();
			m_handles.clear();
			m_freelist.clear();
			m_slots.clear();
		}

		inline void reserve(index_t size) {
			m_data.reserve(size);
			m_handles.reserve(size);
			m_slots.reserve(size);
		}

		/// @brief Returns the handle of the object at idx in the packed array
		/// @param idx the packed index of the object
		/// @return Handle
		inline Handle<T> get_handle(index_t idx) const {
			assert(m_data.size() == m_handles.size());

			if(idx >= m_data.size())
				throw std::invalid_argument("Index out of range!");

			return m_handles[idx];
		}

		/// @brief Adds a new object to the array
		/// @param data the object to add
		/// @return The handle to the newly added object
		inline Handle<T> add(T&& data)
		{
			Handle<T> handle;

			// allocate more memory if free list is empty
			if (m_freelist.empty())
			{
				assert(m_data.size() <= std::numeric_limits<index_t>::max());

				handle.index = m_slots.size();
				handle.generation = 0;
				m_slots.emplace_back(m_data.size(), handle.generation);
			}
			else
			{
				handle.index = m_freelist.back();
				m_freelist.pop_back();

				handle.generation = m_slots[handle.index].generation;
				m_slots[handle.index].index = m_data.size();
			}

			m_data.push_back(std::move(data));
			m_handles.push_back(handle);

			assert(m_data.size() == m_handles.size());

			return handle;
		}

		inline Handle<T> add(const T& data) {
			T tmp = data;
			return add(std::move(tmp));
		}

		/// @brief Removes an object from the array by handle
		/// @param handle the handle of the object to remove
		inline void remove(Handle<T> handle)
		{
			if(!has(handle))
				throw std::invalid_argument("Unable to remove, handle is invalid!");

			auto idx = m_slots[handle.index].index; // packed index

			assert(m_data.size() == m_handles.size());

			std::swap(m_data[idx], m_data[m_data.size() - 1]);
			m_data.pop_back();

			// update index of last item
			auto handle_to_update = m_handles[m_handles.size() - 1];
			m_handles[idx] = handle_to_update;
			m_handles.pop_back();

			m_slots[handle_to_update.index].index = idx;

			m_freelist.push_back(handle.index);
			m_slots[handle.index].index = INVALID_INDEX;
			m_slots[handle.index].generation++;
		}

	private:
		struct Slot {
		    index_t index;       // packed index in m_data
		    index_t generation;
		};

		static constexpr index_t INVALID_INDEX = std::numeric_limits<index_t>::max();

		std::vector<T> m_data;
		std::vector<Handle<T>> m_handles;
		std::vector<Slot> m_slots;
		std::vector<index_t> m_freelist;
	};
}
