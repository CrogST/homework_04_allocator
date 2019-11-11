#include <iostream>
#include <vector>
#include <map>
#include <tuple>

#include <memory>

template<typename T, int N>
struct alloc {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind {
        using other = alloc<U, N>;
    };

    T *allocate(std::size_t n) {
        std::cout << __FUNCTION__ << std::endl;
        if(mem_pool.size() % N == 0) {
            auto p = std::malloc(N * sizeof(T));
            std::cout << "malloc result: " << std::hex << "0x" << p << std::endl;
            if (!p) throw std::bad_alloc();
            auto ptr = reinterpret_cast<T *>(p);
            auto size = mem_pool.size();
            for(int i = 1; i <= N; i++) {
                auto val = ptr + (N - i);
                std::cout << "add address to mempool: " << std::hex << "0x" << val << std::endl;
                mem_pool.push_back(val);
            }
        }

        auto ptr = mem_pool[mem_pool.size() - 1];
        mem_pool.pop_back();
        std::cout << "return: " << std::hex << "0x" << ptr << std::endl;
        return ptr;
    }

    void deallocate(T *p, std::size_t n) {
        std::cout << __FUNCTION__ << std::endl;
        std::cout << "add to mempool: " << std::hex << "0x" << p << std::endl;
        mem_pool.push_back(p);
        if(mem_pool.size() % N == 0) {
            std::cout << "free: " << std::hex << "0x" << p << std::endl;
            std::free(p);
            std::cout << "clear mempool last " << N << " entities" << std::endl;
            mem_pool.erase(mem_pool.end() - N, mem_pool.end());
        }
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) const {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        new(p) U(std::forward<Args>(args)...);
    }

    void destroy(T *p) const {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        p->~T();
    }
private:
    std::vector<T*> mem_pool;

};

struct hard {
    int i;
    double d;

    hard(int i, double d) : i(i), d(d) { std::cout << __PRETTY_FUNCTION__ << std::endl; };

    hard(const hard &)
    //    = delete;
    { std::cout << __PRETTY_FUNCTION__ << std::endl; };

    hard(hard &&) noexcept
    //    = delete;
    { std::cout << __PRETTY_FUNCTION__ << std::endl; };

    ~hard() { std::cout << __PRETTY_FUNCTION__ << std::endl; };
};

template <int T>
using type = std::map< int, hard, std::less<int>, alloc< std::pair<const int, hard>, T> >;

template <int T>
auto add_value(int i, type<T> & h) {
  h.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(i, i));
};

int main(int, char *[]) {

    const int N = 5;
    static_assert (N > 2, "так надо!");

    auto h = type<N>{};

    std::cout << "Выделяем память. Заполняем, но не всю" << std::endl;
    for (size_t i = 0; i < N; ++i) {
        add_value(h.size(), h);
        std::cout << "______" << std::endl;
    }

    std::cout << "********************" << std::endl;

    for (size_t i = 0; i < 2; ++i)
        h.erase(h.find(h.size() - 1)); //удаление посл. элемента

    std::cout << "********************" << std::endl;

    for (size_t i = 0; i < N; ++i) {
        add_value(h.size(), h);
        std::cout << "______" << std::endl;
    }

    std::cout << "********************" << std::endl;

    for (size_t i = 0; i < N; ++i)
        h.erase(h.find(h.size() - 1)); //удаление посл. элемента

    std::cout << "********************" << std::endl;

    return 0;
}
