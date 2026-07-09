#pragma once

#include "atomic_vector.h"
#include "ticket_dispensor.h"

namespace GL {
    // An unsorted list of items. Items, when inserted, are given a unique (non-contiguous) index to access them later. Erasing items allows for the re-use of their index in the future.
    // Insertion, access, and erasure are all atomic actions.
    // Memory corruption is not prevented if attempting to access an index after it has been erased. 
    template<class type> class atomic_bag {
    private:
        GL::atomic_vector<std::pair<type, bool>>
            items;
        GL::fast_ticket_dispensor<false>
            tickets;

    public:
        // re-uses the position of previous slots as much as is possible. Returns the index or "ticket" for that item.
        size_t push_back(type&& rhs) {
            auto ticket = tickets.get_ticket();
            items.get_or_make(ticket) = { std::forward<type>(rhs), true };
            return ticket;
        };
        // re-uses the position of previous slots as much as is possible. Returns the index or "ticket" for that item.
        size_t push_back(type const& rhs) {
            auto ticket = tickets.get_ticket();
            items.get_or_make(ticket) = { rhs, true };
            return ticket;
        };
        // accessor using a valid position or ticket value.
        type& operator[](size_t position) {
            return items[position];
        };
        // accessor using a valid position or ticket value.
        type const& operator[](size_t position) const {
            return items[position];
        };
        // returns a position for re-use later. May not destroy the object until the position is re-used or until the list is destroyed. Thread-safe. 
        void erase(size_t position) {
            items.at(position).second = false;
            tickets.return_ticket(position);
        };
        // for-each loop on the current, valid items. Not thread-safe. 
        template <typename F> void unsafe_for_each(F const& func) {
            for (auto& x : items) {
                if (x.second) {
                    func(x.first);
                }
            }
        };
    };
};