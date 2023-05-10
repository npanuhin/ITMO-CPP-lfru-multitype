#include "acp/Pool.hpp"

PoolAllocator::PoolAllocator(std::size_t const block_size, std::initializer_list<std::size_t> sizes)
    : m_block_size(block_size), m_sizes(sizes) {
    std::sort(m_sizes.begin(), m_sizes.end());

    m_starts.resize(sizes.size());
    std::size_t cur_size = 0;
    for (std::size_t i = 0; i < sizes.size(); i++) {
        m_starts[i] = cur_size;
        cur_size += round_block_size(block_size, m_sizes[i]);
    }

    m_used_map.resize(cur_size);
    m_storage.resize(cur_size);
}

std::size_t PoolAllocator::round_block_size(std::size_t const block_size, std::size_t const size) {
    return block_size / size * size;
}

std::size_t PoolAllocator::get_pos(const std::size_t size) const {
    const auto it = std::lower_bound(m_sizes.begin(), m_sizes.end(), size);
    if (it == m_sizes.end() || *it != size) {
        return NOT_FOUND;
    }
    std::size_t block_start = m_starts[it - m_sizes.begin()];
    for (std::size_t i = 0; i < round_block_size(m_block_size, size); i += size) {
        std::size_t pos = block_start + i;
        if (!m_used_map[pos]) {
            return pos;
        }
    }
    return NOT_FOUND;
}

void *PoolAllocator::allocate(std::size_t const n) {
    const auto pos = get_pos(n);
    if (pos == NOT_FOUND) {
        throw std::bad_alloc{};
    }
    m_used_map[pos] = true;
    return &m_storage[pos];
}

void PoolAllocator::deallocate(void const *ptr) {
    const auto *byte_ptr = static_cast<const std::byte *>(ptr);
    if (&m_storage.front() <= byte_ptr && byte_ptr <= &m_storage.back()) {
        m_used_map[byte_ptr - &m_storage.front()] = false;
    }
}
