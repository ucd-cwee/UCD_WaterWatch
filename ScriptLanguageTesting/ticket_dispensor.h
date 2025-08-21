#pragma region "Includes"
#pragma once
#include "aba_problem.h"
#include "atomic_vector.h"
#pragma endregion

// Good Language namespace
namespace GL {
    // Multi-threaded socket system for adding/removing "listeners" in parallel based on tickets, provided by the ticket_dispensor.
    // Tickets should be kept as small as possible and re-used as much as possible, to reduce the size of the sockets, which significantly impacts performance.
    template <typename T> class callback {
    public:
        class ScopedListener {
        public:
            ScopedListener()
                : _index(0), _parent(nullptr) {};
            ScopedListener(size_t index, callback& parent)
                : _index(index), _parent(&parent) {};
            ScopedListener(ScopedListener const& rhs) = delete;
            ScopedListener(ScopedListener&& rhs)
                : _index(std::move(rhs._index)), _parent(std::move(rhs._parent))
            {
                rhs._index = 0;
            };
            ScopedListener& operator=(ScopedListener const& rhs) = delete;
            ScopedListener& operator=(ScopedListener&& rhs)
            {
                if (_index > 0)
                    _parent->remove_listener(_index);

                _index = std::move(rhs._index);
                _parent = std::move(rhs._parent);
                rhs._index = 0;

                return *this;
            };
            ~ScopedListener() {
                if (_index > 0)
                    _parent->remove_listener(_index);
            };

        private:
            size_t _index;
            callback* _parent;
        };

    private:
        struct Wrap {
            long alive;
            long count;
            T* ptr;
            size_t call_version;
        };

        static size_t&
            _call_version() {
            static size_t call_version{ 0 };
            return call_version;
        };
        size_t
            _size{ 0 };
        GL::atomic_vector<Wrap>
            _listeners;
        void (T::* _callback)(long*, size_t);
        std::atomic<bool>
            alive{ false };

        // add a listener to the list
        __declspec(noinline) void add_listener(size_t index, T* p) {
            if (alive.load()) {
                if (_size <= index) {
                    if (_listeners.size() <= index) (void)_listeners.grow_to_at_least((index + 2) + ((index + 2) % 16));
                    // InterlockedIncrement(static_cast<volatile size_t*>(&_size)); // 
                    InterlockedExchange(static_cast<volatile size_t*>(&_size), index);
                }
                Wrap& wrap = _listeners[index/* - 1*/];
                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(p));
                InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1 << 8);
                InterlockedIncrement(static_cast<volatile long*>(&wrap.alive));
            }
        };
        // remove a listener from the list
        __declspec(noinline) void remove_listener(size_t index) {
            if (alive.load() && _listeners.size() >= index) {
                Wrap& wrap = _listeners[index/* - 1*/];
                InterlockedDecrement(static_cast<volatile long*>(&wrap.alive));
                if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), -(1 << 8)) == 0) {}
                else while (wrap.count != 0) if (!wrap.ptr) InterlockedExchange(static_cast<volatile long*>(&wrap.count), 0);
                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(nullptr));
            }
        };

    public:
        callback(void (T::* listener)(long*, size_t))
            : _callback{ listener }, alive{ true }
        {};
        ~callback() {
            alive = false;
        };

        ScopedListener listener(size_t index, T* p) {
            add_listener(index, p);
            return ScopedListener(index, *this);
        };
        // callback performed on all listeners
        __declspec(noinline) void speak(long* parent_alive, size_t call_number = 0) {
            if (call_number == 0)
                call_number = InterlockedIncrement(static_cast<volatile size_t*>(&_call_version()));

            for (size_t i = 0; i < _size; ++i) {
                Wrap& wrap = _listeners[i];
                if (wrap.alive) {
                    if (!parent_alive || *parent_alive) {
                        if (wrap.call_version >= call_number) { continue; }
                        else {
                            InterlockedExchange(static_cast<volatile size_t*>(&wrap.call_version), call_number);
                        }

                        if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1) >= (1 << 8))
                            (wrap.ptr->*_callback)(&wrap.alive, call_number); // _callback(wrap.ptr, &wrap.alive);
                        InterlockedAdd(static_cast<volatile long*>(&wrap.count), -1);
                    }
                    else break;
                }
            }
        };
    };

    // Manages tickets in the range of [1, INF) and assumes ticket 0 is already given to the owner of ticket_dispensor
    // Prints new tickets as needed, but recycles old tickets as much as possible. 
    class ticket_dispensor {
    public:
        class ScopedTicket {
        public:
            ScopedTicket()
                : _index(0), _parent(nullptr) {};
            ScopedTicket(size_t index, ticket_dispensor& parent)
                : _index(index), _parent(&parent) {};
            ScopedTicket(ScopedTicket const& rhs) = delete;
            ScopedTicket(ScopedTicket&& rhs) noexcept
                : _index(std::move(rhs._index)), _parent(std::move(rhs._parent))
            {
                rhs._index = 0;
            };
            ScopedTicket& operator=(ScopedTicket const& rhs) = delete;
            ScopedTicket& operator=(ScopedTicket&& rhs) noexcept
            {
                _index = std::move(rhs._index);
                _parent = std::move(rhs._parent);
                rhs._index = 0;
                return *this;
            };
            ~ScopedTicket() {
                if (_index)
                    _parent->return_ticket(_index);
            };

            size_t _index;
            ticket_dispensor* _parent;
        };

    public:
        aba_problem::stack<size_t>
            queue{};
        std::atomic<size_t>
            indexes{ 0 };

    public:
        size_t num_tickets() const {
            return indexes.load() + 1;
        };
        __declspec(noinline) ScopedTicket get_scoped_ticket() {
            return ScopedTicket(get_ticket(), *this);
        };
        __declspec(noinline) size_t get_ticket() {
            size_t out;
            if (!queue.try_pop(out)) {
                out = ++indexes;
            }
            return out;
        };
        __declspec(noinline) void return_ticket(size_t ticket) {
            queue.push(ticket);
        };
        void reserve(int n) {
            std::vector<size_t> tickets;
            tickets.reserve(n);

            for (int i = 0; i < n; i++) {
                tickets.push_back(this->get_ticket());
            }
            for (auto& x : tickets) {
                this->return_ticket(x);
            }
        };
    };
};