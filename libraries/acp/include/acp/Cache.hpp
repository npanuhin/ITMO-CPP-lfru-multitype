#ifndef ACP_CACHE_HPP
#define ACP_CACHE_HPP

#include <cstddef>
#include <new>
#include <ostream>
#include <list>

template <class Key, class KeyProvider, class Allocator>
class Cache {
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&...alloc_args)
        : m_max_top_size(cache_size), m_max_low_size(cache_size), m_alloc(std::forward<AllocArgs>(alloc_args)...) {}

    std::size_t size() const { return m_top_queue.size() + m_low_queue.size(); }

    bool empty() const { return size() == 0; }

    template <class T>
    T &get(const Key &key);

    std::ostream &print(std::ostream &strm) const;

    friend std::ostream &operator<<(std::ostream &strm, const Cache &cache) { return cache.print(strm); }

private:
    const std::size_t m_max_top_size, m_max_low_size;
    Allocator m_alloc;
    std::list<KeyProvider *> m_top_queue, m_low_queue;
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T &Cache<Key, KeyProvider, Allocator>::get(const Key &key) {
    auto it = std::find_if(m_top_queue.begin(), m_top_queue.end(), [&key](const KeyProvider *ptr) {
        return *ptr == key;
    });
    if (it != m_top_queue.end()) {
        m_top_queue.splice(m_top_queue.begin(), m_top_queue, it);
        return *static_cast<T *>(m_top_queue.front());
    }

    it = std::find_if(m_low_queue.begin(), m_low_queue.end(), [&key](const KeyProvider *ptr) {
        return *ptr == key;
    });
    bool in_low = (it != m_low_queue.end());
    if (in_low) {
        m_top_queue.splice(m_top_queue.begin(), m_low_queue, it);

        if (m_top_queue.size() <= m_max_top_size) {
            return *static_cast<T *>(m_top_queue.front());
        }

        m_low_queue.push_front(m_top_queue.back());
        m_top_queue.pop_back();

    } else {
        m_low_queue.push_front(m_alloc.template create<T>(key));
    }

    if (m_max_low_size < m_low_queue.size()) {
        m_alloc.template destroy<KeyProvider>(m_low_queue.back());
        m_low_queue.pop_back();
    }

    return *static_cast<T *>(in_low ? m_top_queue.front() : m_low_queue.front());
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream &Cache<Key, KeyProvider, Allocator>::print(std::ostream &strm) const {
    strm << "Priority:";
    for (const auto ptr : m_top_queue) {
        strm << " " << *ptr;
    }
    strm << "\nRegular:";
    for (const auto ptr : m_low_queue) {
        strm << " " << *ptr;
    }
    return strm << '\n';
}

#endif  // ACP_CACHE_HPP
