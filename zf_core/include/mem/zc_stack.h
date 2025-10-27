namespace zf {
    template<typename tp_type>
    class c_stack {
    public:
        c_stack() = default;
        c_stack(const c_array<tp_type> arr, const int height = 0) : m_arr(arr), m_height(height) {}

        int Capacity() {
            return m_arr.Len();
        }

        int Height() {
            return m_height;
        }

        bool IsEmpty() {
            return m_height == 0;
        }

        bool IsFull() {
            return m_height == Capacity();
        }

        tp_type& operator[](const int index) const {
            assert(index >= 0 && index < m_height);
            return m_arr[index];
        }

        void Push(const tp_type& val) {
            assert(!IsFull());

            m_arr[m_height] = val;
            m_height++;
        }

        tp_type& Pop() {
            assert(!IsEmpty());

            m_height--;
            return m_arr[m_height];
        }

    private:
        c_array<tp_type> m_arr;
        int m_height;
    };
}
