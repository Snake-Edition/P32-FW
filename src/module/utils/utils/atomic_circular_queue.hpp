#pragma once

#include <limits>
#include <atomic>

/**
 * @brief   Atomic Circular Queue class
 * @details Implementation of an atomic ring buffer data structure which can use all slots
 *          at the cost of strict index requirements
 * @note Inspired from https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
 */
template <typename T, typename index_t, index_t N>
class AtomicCircularQueue {
private:
    /**
     * @brief   Buffer structure
     * @details This structure consolidates all the overhead required to handle
     *          a circular queue such as the pointers and the buffer vector.
     */
    struct buffer_t {
        std::atomic<index_t> head;
        std::atomic<index_t> tail;
        T queue[N];
    } buffer;

    static index_t mask(index_t val) { return val & (N - 1); }

public:
    /**
     * @brief   Class constructor
     * @details This class requires two template parameters, T defines the type
     *          of item this queue will handle and N defines the maximum number of
     *          items that can be stored on the queue.
     */
    AtomicCircularQueue() {
        static_assert(std::numeric_limits<index_t>::is_integer, "Buffer index has to be an integer type");
        static_assert(!std::numeric_limits<index_t>::is_signed, "Buffer index has to be an unsigned type");
        static_assert(N < std::numeric_limits<index_t>::max(), "Buffer size bigger than the index can support");
        static_assert((N & (N - 1)) == 0, "The size of the queue has to be a power of 2");
        static_assert(decltype(buffer_t::head)::is_always_lock_free, "index_t is not lock-free");
        static_assert(decltype(buffer_t::tail)::is_always_lock_free, "index_t is not lock-free");
        buffer.head = buffer.tail = 0;
    }

    /**
     * @brief   Removes and returns a item from the queue
     * @details Removes the oldest item on the queue, pointed to by the
     *          buffer_t head field. The item is returned to the caller.
     * @return  type T item
     */
    T dequeue() {
        assert(!isEmpty());

        index_t index = buffer.head;
        T ret = buffer.queue[mask(index++)];
        buffer.head = index;
        return ret;
    }

    /**
     * @brief   Adds an item to the queue
     * @details Adds an item to the queue on the location pointed by the buffer_t
     *          tail variable. Returns false if no queue space is available.
     * @param   item Item to be added to the queue
     * @return  true if the operation was successful
     */
    bool enqueue(T const &item) {
        if (isFull()) {
            return false;
        }

        index_t index = buffer.tail;
        buffer.queue[mask(index++)] = item;
        buffer.tail = index;
        return true;
    }

    /**
     * @brief   Checks if the queue has no items
     * @details Returns true if there are no items on the queue, false otherwise.
     * @return  true if queue is empty
     */
    bool isEmpty() { return buffer.head == buffer.tail; }

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
    T peek() { return buffer.queue[mask(buffer.head)]; }

    /**
     * @brief Gets the number of items on the queue
     * @details Returns the current number of items stored on the queue.
     * @return number of items in the queue
     */
    index_t count() { return buffer.tail - buffer.head; }

    /**
     * @brief Clear the contents of the queue
     */
    void clear() { buffer.head.store(buffer.tail); }
};
