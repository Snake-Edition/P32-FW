#pragma once

#include <limits>
#include <atomic>
#include <cassert>

/**
 * @brief   SPSC (Single Producer Single Consumer) Atomic Circular Queue class
 * @details Implementation of an atomic ring buffer data structure which can use all slots
 *          at the cost of strict index requirements.
 *          Please note that the "atomicity" only means here that enqueing and dequeing can be in different threads.
 *          All enqueues have to be done in the same thread however, and all the dequeues too.
 *
 * @note Inspired from https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 */
template <typename T, typename index_t, index_t N>
class AtomicCircularQueue {
    static_assert(std::numeric_limits<index_t>::is_integer, "Buffer index has to be an integer type");
    static_assert(!std::numeric_limits<index_t>::is_signed, "Buffer index has to be an unsigned type");

    // Note: We cannto allow N == max, because then we wouldn't be able to distinct between full and empty queue.
    // There always needs to be a one "extra" index value free
    static_assert(N < std::numeric_limits<index_t>::max(), "Buffer size bigger than the index can support");
    static_assert((N & (N - 1)) == 0, "The size of the queue has to be a power of 2");

    static_assert(std::atomic<index_t>::is_always_lock_free, "index_t is not lock-free");

private:
    std::atomic<index_t> head = 0;
    std::atomic<index_t> tail = 0;
    T queue[N];

    static index_t mask(index_t val) { return val & (N - 1); }

public:
    /// Removes an item from the queue and stores it into \param target
    /// \returns false if the queue was empty
    [[nodiscard]] bool dequeue(T &target) {
        if (isEmpty()) {
            return false;
        }

        index_t index = head;
        target = std::move(queue[mask(index++)]);
        head = index;
        return true;
    }

    /**
     * @brief   Removes and returns a item from the queue
     * @details Removes the oldest item on the queue, pointed to by the
     *          buffer_t head field. The item is returned to the caller.
     * @return  type T item
     */
    T dequeue() {
        assert(!isEmpty());

        index_t index = head;
        T ret = std::move(queue[mask(index++)]);
        head = index;
        return ret;
    }

    /**
     * @brief   Adds an item to the queue
     * @details Adds an item to the queue on the location pointed by the buffer_t
     *          tail variable. Returns false if no queue space is available.
     * @param   item Item to be added to the queue
     * @return  true if the operation was successful
     */
    [[nodiscard]] bool enqueue(const T &item) {
        if (isFull()) {
            return false;
        }

        index_t index = tail;
        queue[mask(index++)] = item;
        tail = index;
        return true;
    }

    /**
     * @brief   Adds an item to the queue
     * @details Adds an item to the queue on the location pointed by the buffer_t
     *          tail variable. Returns false if no queue space is available.
     * @param   item Item to be added to the queue
     * @return  true if the operation was successful
     */
    [[nodiscard]] bool enqueue(T &&item) {
        if (isFull()) {
            return false;
        }

        index_t index = tail;
        queue[mask(index++)] = std::move(item);
        tail = index;
        return true;
    }

    /**
     * @brief   Checks if the queue has no items
     * @details Returns true if there are no items on the queue, false otherwise.
     * @return  true if queue is empty
     */
    bool isEmpty() { return head == tail; }

    /**
     * @brief   Checks if the queue is full
     * @details Returns true if the queue is full, false otherwise.
     * @return  true if queue is full
     */
    bool isFull() { return count() == N; }

    /**
     * @brief   Gets the queue size
     * @details Returns the maximum number of items a queue can have.
     * @return  the queue size
     */
    index_t size() { return N; }

    /**
     * @brief   Gets the next item from the queue without removing it
     * @details Returns the next item in the queue without removing it
     *          or updating the pointers.
     * @return  first item in the queue
     */
    const T &peek() { return queue[mask(head)]; }

    /**
     * @brief Gets the number of items on the queue
     * @details Returns the current number of items stored on the queue.
     * @return number of items in the queue
     */
    index_t count() { return tail - head; }

    /**
     * @brief Clear the contents of the queue
     */
    void clear() { head.store(tail); }
};
