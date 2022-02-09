
#ifndef _RACE_SEMAPHORE_HPP_
#define _RACE_SEMAPHORE_HPP_


#include <atomic>
#include <concepts>

template<std::integral T = uint64_t>
class race_semaphore
{
	std::atomic<T> m_gate{ 0 };
	std::atomic<T> m_write_gate{ 0 };

public:
	race_semaphore() = default;
	race_semaphore(const race_semaphore&) = delete;
	race_semaphore(race_semaphore&&) = delete;

	race_semaphore& operator=(const race_semaphore&) = delete;
	race_semaphore& operator=(race_semaphore&&) = delete;

	void acquire_for_read() noexcept;
	void acquire_for_write() noexcept;
	void release_from_read() noexcept;
	void release_from_write() noexcept;

	bool try_acquire_for_read() noexcept;
	bool try_acquire_for_write() noexcept;
};




template<std::integral T>
inline void race_semaphore<T>::acquire_for_read() noexcept
{
	while (true)
	{
		T current = this->m_write_gate;
		while (current != 0)
		{
			this->m_write_gate.wait(current);
			current = this->m_write_gate;
		}

		++this->m_gate;
		if (this->m_write_gate == 0)
			break;
		else
			--this->m_gate;
	}
}

template<std::integral T>
inline void race_semaphore<T>::acquire_for_write() noexcept
{
	while (true)
	{
		T current = this->m_gate;
		while (current != 0)
		{
			this->m_gate.wait(current);
			current = this->m_gate;
		}

		if (!this->m_gate.compare_exchange_strong(current, current + 1))
			continue;
		current = 0;
		if (!this->m_write_gate.compare_exchange_strong(current, current + 1))
		{
			--this->m_gate;
			continue;
		}
		break;
	}
}

template<std::integral T>
inline void race_semaphore<T>::release_from_read() noexcept
{
	--this->m_gate;
	this->m_gate.notify_one();
}

template<std::integral T>
inline void race_semaphore<T>::release_from_write() noexcept
{
	this->release_from_read();
	
	--this->m_write_gate;
	this->m_write_gate.notify_one();
}

template<std::integral T>
inline bool race_semaphore<T>::try_acquire_for_read() noexcept
{
	T current = this->m_write_gate;
	if (current != 0)
		return false;

	++this->m_gate;
	if (this->m_write_gate == 0)
		return true;
	else
	{
		--this->m_gate;
		return false;
	}
}

template<std::integral T>
inline bool race_semaphore<T>::try_acquire_for_write() noexcept
{
	if (this->m_gate != 0)
		return false;

	T current = 0;
	if (!this->m_gate.compare_exchange_strong(current, current + 1))
		return false;

	current = 0;
	if (!this->m_write_gate.compare_exchange_strong(current, current + 1))
	{
		--this->m_gate;
		return false;
	}
	return true;
}

#endif //!_RACE_SEMAPHORE_HPP_
