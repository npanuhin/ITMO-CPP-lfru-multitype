#ifndef ACP_POOL_HPP
#define ACP_POOL_HPP

#include <cstddef>
#include <initializer_list>
#include <new>
#include <vector>
#include <algorithm>

class PoolAllocator {
public:
    PoolAllocator(std::size_t block_size, std::initializer_list<std::size_t> sizes);

    void* allocate(std::size_t n);

    void deallocate(void const* ptr);

private:
    static std::size_t round_block_size(std::size_t block_size, std::size_t size);
    std::size_t get_pos(std::size_t n) const;

    const std::size_t m_block_size;     // Размер одного блока
    std::vector<std::size_t> m_sizes;   // m_sizes[i] - размер элемента i
    std::vector<std::size_t> m_starts;  // m_starts[i] - позиции начала блока i
    std::vector<std::byte> m_storage;   // Хранилище данных
    std::vector<bool> m_used_map;       // m_used_map[i] - занят ли блок i

    static constexpr std::size_t NOT_FOUND = -1;
};

#endif  // ACP_POOL_HPP
