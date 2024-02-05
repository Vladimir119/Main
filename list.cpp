#include <memory>

template<size_t N>
struct StackStorage {
  StackStorage  (): _shift(0) {}
  StackStorage  (const StackStorage& other) = delete;
  ~StackStorage () = default;
  StackStorage operator=(const StackStorage& other) = delete;

  char _array[N];
  size_t _shift;
};

template<class T, size_t N>
struct StackAllocator {
  using value_type = T;
  using propagate_on_copy_assignment = std::true_type;
  using pointer = T*;

  StackAllocator() = delete;
  StackAllocator(StackStorage<N>& stackStorage): _array(&stackStorage) {};
  ~StackAllocator() = default;

  template< class U>
  struct rebind
  {
    typedef StackAllocator<U, N> other;
  };

  template<class U>
  StackAllocator(const StackAllocator<U, N>& other): _array(other._array) {}

  StackAllocator& operator=(const StackAllocator& other) {
    StackAllocator temp(other);
    std::swap(_array, temp._array);
    return *this;
  };

  T* allocate(size_t size);

  void deallocate(T*, size_t) {}

  StackAllocator& select_on_container_copy_construction() { return *this; }
  
  bool operator==(const StackAllocator& other) const { return _array == other._array; }
  bool operator!=(const StackAllocator& other) const {return !(*this==other); }

  StackStorage<N>* _array;
};

template < class T, size_t N >
T* StackAllocator<T, N>::allocate(size_t size) {
  void* ptr = _array->_array + _array->_shift;
  size_t old_size = N - _array->_shift;
  if (std::align(alignof(T), size * sizeof(T), ptr, old_size)) {
    T* result = reinterpret_cast<T*>(ptr);
    _array->_shift += size * sizeof(T);
    return result;
  }
  return nullptr;
}

template<class T, class Allocator=std::allocator<T>>
class List {
 private:
  struct BaseNode {
    BaseNode* next;
    BaseNode* last;
  };

  struct Node : BaseNode {
    T value; 
  };

  template<bool is_const=false>
  class base_iterator {
   public:
    using  value_type = std::conditional_t<is_const, const T, T>;
    using   pointer = std::conditional_t<is_const, const T*, T*>;
    using                                  difference_type = int;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using    iterator_category = std::bidirectional_iterator_tag; 

    base_iterator(BaseNode* ptr): current(ptr) {}
    base_iterator(const BaseNode* ptr): current(const_cast<BaseNode*>(ptr)) {}
    base_iterator(const base_iterator& other): current(other.current) {}

    base_iterator& operator=(const base_iterator& other) {
      base_iterator temp(other);
      std::swap(current, temp.current);
      return *this;
    }

    base_iterator& operator++() {
      current = current->next;
      return *this;
    }

    base_iterator& operator--() {
      current = current->last;
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator temp = *this; 
      ++(*this);
      return temp;
    }

    base_iterator operator--(int) {
      base_iterator temp = *this; 
      --(*this);
      return temp;
    }

    operator base_iterator<true>() const {
      base_iterator<true> temp(current);
      return temp;
    }

    reference operator*() const { return static_cast<Node*>(current)->value; }
    
    pointer operator->() const { return &(static_cast<Node*>(current)->value); }

    bool operator==(const base_iterator& other) const {
      return current == other.current;
    }

    bool operator!=(const base_iterator& other) const {
      return !(*this == other);
    }

   private:

    friend class List;

    BaseNode* current;
  };
  
 public:
  using const_iterator = base_iterator<true>;
  using iterator = base_iterator<false>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using allocator_node = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

  List(): alloc(Allocator()), _size(0) { initialize_end_node(); }

  List(const List& other);

  explicit List(size_t count);

  List(size_t count, const T& value);

  explicit List(const Allocator& alloc): alloc(alloc), _size(0) { initialize_end_node(); }

  List(size_t count, const Allocator& alloc);

  List(size_t count, const T& value, const Allocator& alloc);

  ~List() { clear(); }

  List& operator=(const List& other);

  size_t size() const { return _size; }

  void push_back(const T& value) { insert(end(), value); }
  void push_front(const T& value) {  insert(begin(), value); }
  void pop_back() { erase(--end()); }
  void pop_front() { erase(begin()); }
  void insert(const_iterator it, const T& value) { emplace(it, value); }
  void erase(const_iterator it);

  Allocator get_allocator() const { return alloc; }

  iterator begin() { return iterator(_end_node.next); }
  iterator end() { return iterator(&_end_node); }

  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }
  const_iterator cbegin() const { return static_cast<const_iterator>(iterator(_end_node.next)); }
  const_iterator cend() const { return static_cast<const_iterator>(iterator(&_end_node)); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const { return crbegin(); }
  const_reverse_iterator rend() const { return crend(); }
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

 private:
  void initialize_end_node();

  void clear();

  template <class... Args>
  void emplace(const_iterator it, const Args&... args);

  [[no_unique_address]] allocator_node alloc;
  BaseNode _end_node;
  size_t _size;
};

template<class T, class Allocator>
List<T, Allocator>::List(const List& other): alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)), _size(0) {
  initialize_end_node();
  try {
    for (auto& it : other) {
      push_back(it);
    }
  } catch(...) {
    clear();
    throw;
  }
}

template<class T, class Allocator>
List<T, Allocator>::List(size_t count): List() {
  try{ 
    for (size_t i = 0; i < count; ++i) {
      emplace(end());
    }
  } catch(...) {
    clear();
    throw;
  }
}

template<class T, class Allocator>
List<T, Allocator>::List(size_t count, const T& value): List() {
  try{ 
    for (size_t i = 0; i < count; ++i) {
      push_back(value);
    }
  } catch(...) {
    clear();
    throw;
  }
}

template<class T, class Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc): List(alloc) {
  try{ 
    for (size_t i = 0; i < count; ++i) {
      emplace(end());
    }
  } catch(...) {
    clear();
    throw;
  } 
}

template<class T, class Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc): List(alloc) {
  try{ 
    for (size_t i = 0; i < count; ++i) {
      push_back(value);
    }
  } catch(...) {
    clear();
    throw;
  }
}

template<class T, class Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  List temp(other);
  std::swap(_size, temp._size);
  std::swap(_end_node, temp._end_node);
  std::swap(_end_node.last->next, temp._end_node.last->next);
  std::swap(_end_node.next->last, temp._end_node.next->last);
  std::swap(alloc, temp.alloc);
  if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value == true) {
    alloc = other.alloc;
  }
  return *this;
}

template<class T, class Allocator>
void List<T, Allocator>::erase(List<T, Allocator>::const_iterator it) {
  --_size;
  it.current->last->next = it.current->next;
  it.current->next->last = it.current->last;
  std::allocator_traits<allocator_node>::destroy(alloc, static_cast<Node*>(it.current));
  std::allocator_traits<allocator_node>::deallocate(alloc, static_cast<Node*>(it.current), 1);
}

template<class T, class Allocator>
void List<T, Allocator>::initialize_end_node() {
  _end_node.last = &_end_node;
  _end_node.next = &_end_node;
}

template<class T, class Allocator>
void List<T, Allocator>::clear() {
  while(_size != 0) {
    pop_back();
  }
}

template<class T, class Allocator>
template <class... Args>
void List<T, Allocator>::emplace(const_iterator it, const Args&... args) {
    Node* current;
    try{
        current = std::allocator_traits<allocator_node>::allocate(alloc, 1);
        std::allocator_traits<allocator_node>::construct(alloc, &current->value, args...);

        current->next = it.current;
        current->last = it.current->last;
        it.current->last->next = current;
        it.current->last = current;
    } catch(...) {
        std::allocator_traits<allocator_node>::deallocate(alloc ,current, 1);
        throw;
    }
    ++_size;
}