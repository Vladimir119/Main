#include <iostream>
#include <vector>
#include <optional>

template <class Key,
          class Value,
          class Hash = std::hash<Key>,
          class Equal = std::equal_to<Key>,
          class Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap;

template <class NodeType, class Alloc>
class List {
private:
  struct BaseNode {
    BaseNode(): next(nullptr), last(nullptr) {};
    BaseNode* next;
    BaseNode* last;
  };

  struct Node : public BaseNode {
    NodeType value;
    size_t hash;
  };

 template<bool is_const=false>
  class base_iterator {      
   public:
    using  value_type = std::conditional_t<is_const, const NodeType, NodeType>;
    using   pointer = std::conditional_t<is_const, const NodeType*, NodeType*>;
    using                                  difference_type = int;
    using reference = std::conditional_t<is_const, const NodeType&, NodeType&>;
    using    iterator_category = std::bidirectional_iterator_tag;

    base_iterator(): current(nullptr) {}
    base_iterator(BaseNode* ptr): current(ptr) {}
    base_iterator(const BaseNode* ptr): current(const_cast<BaseNode*>(ptr)) {}
    base_iterator(const base_iterator& other) = default;

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
      base_iterator<true> temp(this->current);
      return temp;
    }

    reference operator*() const { return static_cast<Node*>(current)->value; }
    
    pointer operator->() const { return &(static_cast<Node*>(current)->value); }

    Node* get_node() { return static_cast<Node*>(current); }

    bool operator==(const base_iterator& other) const {
      return current == other.current;
    }

    bool operator!=(const base_iterator& other) const {
      return !(*this == other);
    }

    private:

    friend class List;

    template <class Key, class Value, class Hash, class Equal, class Alloc_map>
    friend class UnorderedMap;

    BaseNode* current;
  };

public:
  using const_iterator = base_iterator<true>;
  using iterator = base_iterator<false>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using allocator_node = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;

  List(): alloc(Alloc()), _size(0) { initialize_end_node(); }

  List(const List& other): alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.alloc)), _size(0) {
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

  List(List&& other) noexcept: alloc(std::move(other.alloc)), _end_node(std::move(other._end_node)), _size(other._size) {
    other._end_node.last->next = &_end_node;
    other._end_node.next->last = &_end_node;

    other._size = 0;
    other.initialize_end_node();
  }

  List(Alloc alloc): alloc(alloc), _size(0) { initialize_end_node(); }

  ~List() { clear(); }

  List& operator=(const List& other) noexcept {
    List temp(other);
    swap(temp);
    return *this;
  }

  List& operator=(List&& other) noexcept {
    clear();
    _size = other._size;
    if (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value == true) {
      alloc = std::move(other.alloc);
    }
    _end_node = std::move(other._end_node);
    other._end_node.last->next = &_end_node;
    other._end_node.next->last = &_end_node;

    other._size = 0;
    other.initialize_end_node();

    return *this;
  }

  void swap(List& other) {
    std::swap(_size, other._size);
    std::swap(_end_node, other._end_node);
    std::swap(_end_node.last->next, other._end_node.last->next);
    std::swap(_end_node.next->last, other._end_node.next->last);
    std::swap(alloc, other.alloc);
    if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value == true) {
      alloc = other.alloc;
    }
  }

  size_t size() const { return _size; }

  void push_back(const NodeType& value) { insert(end(), value); }
  void push_back(NodeType&& value) { insert(end(), std::move(value)); }
  void push_front(const NodeType& value) { insert(begin(), value); }
  void push_front(NodeType&& value) { insert(begin(), std::move(value)); }
  void pop_back() { erase(--end()); }
  void pop_front() { erase(begin()); }
  void insert(const_iterator it, const NodeType& value) { emplace(it, value); }
  void insert(const_iterator it, NodeType&& value) { emplace(it, std::move(value)); }
  void erase(const_iterator it);

  void insertNode(iterator it, Node* newNode);

  template <class... Args>
  void emplace(const_iterator it, Args&&... args);

  Alloc get_allocator() const { return alloc; }
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
  template <class Key, class Value, class Hash, class Equal, class Alloc_map>
  friend class UnorderedMap;

  void initialize_end_node();

  void clear();

  [[no_unique_address]] allocator_node alloc;
  BaseNode _end_node;
  size_t _size;
};


template <class Key, class Value, class Hash, class Equal, class Alloc>
class UnorderedMap {
 public:
  using NodeType = std::pair<const Key, Value>;
  using Node = typename List<NodeType, Alloc>::Node;
  using BaseNode = typename List<NodeType, Alloc>::BaseNode;
  using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using const_iterator = typename List<NodeType, Alloc>::const_iterator;
  using iterator = typename List<NodeType, Alloc>::iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using optional_iterator_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<std::optional<iterator>>;

  UnorderedMap(const Hash &hash_func = Hash(),
               const Equal &equal_func = Equal(),
               const Alloc &alloc = Alloc()) : _hash_func(hash_func),
                                               _equal_func(equal_func),                                              
                                               _alloc(alloc),
                                               _node_alloc(alloc),
                                               _elements(alloc),
                                               _table(13, alloc),
                                               _max_load_factor(1) {}

  UnorderedMap(const UnorderedMap &other) : _hash_func(other._hash_func),
                                            _equal_func(other._equal_func),
                                            _alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(other._alloc)),
                                            _node_alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(other._node_alloc)),
                                            _elements(_alloc),
                                            _table(other._table.size(), _alloc),
                                            _max_load_factor(other._max_load_factor) {
    for (auto& i : other) {
      insert(i); 
    }
  }

  UnorderedMap(UnorderedMap &&other) noexcept = default;

  UnorderedMap &
  operator=(const UnorderedMap &other) noexcept {
    UnorderedMap copy = other;
    swap(copy);
    if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value == true) {
      _alloc = other._alloc;
    }
    return *this;
  }

  UnorderedMap &
  operator=(UnorderedMap &&other) noexcept {
    _hash_func = std::move(other._hash_func);
    _equal_func = std::move(other._equal_func);
    _table = std::move(other._table);
    _elements = std::move(other._elements);
    _max_load_factor = std::move(other._max_load_factor);
    if (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value == true) {
      _alloc = std::move(other._alloc);
      _node_alloc = std::move(other._node_alloc);
    }
    other._max_load_factor = 0;
    return *this;
  }

  ~UnorderedMap() = default;

  Value &
  operator[](const Key &key) {
    iterator it = find(key);
    if (it == end()) {
      size_t hash = _hash_func(key) % _table.size();
      if (_table[hash] == std::nullopt) {
        it = end();
        _table[hash] = end();
      } else {
        it = *_table[hash];
      }
      _elements.emplace(it, key, std::move(Value()));
      --it;
      --(*_table[hash]);
      static_cast<Node*>(it.current)->hash = hash;
    }
    return it->second;
  }

  Value &
  operator[](Key &&key) {
    iterator it = find(key);
    if (it == end()) {
      size_t hash = _hash_func(key) % _table.size();
      if (_table[hash] == std::nullopt) {
        it = end();
        _table[hash] = end();
      } else {
        it = *_table[hash];
      }
      _elements.emplace(it, std::move(key), std::move(Value()));
      --it;
      --(*_table[hash]);
      static_cast<Node*>(it.current)->hash = hash;
    }
    return it->second;
  }

  Value &
  at(const Key &key) {
    iterator it = find(key);
    if (it == end()) {
      throw std::out_of_range("out of range");
    }
    return it->second;
  }

  const Value &
  at(const Key &key) const {
    const_iterator it = find(key);
    if (it == cend()) {
      throw std::out_of_range("out of range");
    }
    return it->second;
  }

  size_t
  size() const { return _elements.size(); }

   template<class... Args>
   void insert(Args&&... args) {
     emplace(std::forward<Args>(args)...);
   }

  std::pair<iterator, bool>
  insert(const NodeType& element_) {
    iterator it = find(element_.first);
    if (it == end()) {
      size_t hash = _hash_func(element_.first) % _table.size();

      if (_table[hash] == std::nullopt) {
        _table[hash] = end();
      } 

      _elements.insert((*_table[hash]), element_);
      --(*_table[hash]);
      Node* temp = static_cast<Node*>((*_table[hash]).current);
      temp->hash = hash;

      if (load_factor() >= _max_load_factor) {
        rehash(_table.size() * 2);
      }

      return {(*_table[temp->hash]), true};
    }
    return {it, false};
  }

  std::pair<iterator, bool>
  insert(NodeType &&element_) {
    NodeType nodeType(NodeType(std::move(const_cast<Key&>(element_.first)), std::move(element_.second)));
    iterator it = find(nodeType.first);
    if (it == end()) {
      size_t hash = _hash_func(nodeType.first) % _table.size(); 

      if (_table[hash] == std::nullopt) {
        _table[hash] = end();
      }

      _elements.insert((*_table[hash]), std::move(nodeType));
      --(*_table[hash]);
      Node* temp = static_cast<Node*>((*_table[hash]).current);
      temp->hash = hash;

      if (load_factor() >= _max_load_factor) {
        rehash(_table.size() * 2);
      }

      return {*_table[temp->hash], true};
    }
    return {it, false};
  }

  template <class InputIterator>
  void
  insert(InputIterator begin_it, InputIterator end_it) {
    for (auto it = begin_it; it != end_it; ++it) {
      insert(std::forward<decltype(*it)>(*it));
    }

  }

  const_iterator
  erase(const_iterator it) {
    iterator next_it = it.current;
    ++next_it;
    Node* temp = static_cast<Node*>(it.current);
    size_t hash = temp->hash;

    if (it == (*_table[hash])) {
      if (next_it != end()) {
        Node* temp_next = static_cast<Node*>(next_it.current);
        size_t next_hash = temp_next->hash;
        if (next_hash == hash) {
          _table[hash] = next_it;
        } else {
          _table[hash] = std::nullopt;
        }
      } else {
        _table[hash] = std::nullopt;
      }
    }
    _elements.erase(it);
    return next_it;
  }

  void
  erase(const_iterator begin_it, const_iterator end_it) {
    const_iterator cur_it = begin_it;
    const_iterator next_it = cur_it;
    ++next_it;
    while (cur_it != end_it) {
      erase(cur_it);
      cur_it = next_it;
      ++next_it;
    }
  }

  template <class... Args>
  std::pair<iterator, bool>
  emplace(Args &&...args) {
    Node* temp = std::allocator_traits<NodeAlloc>::allocate(_node_alloc, 1);
    std::allocator_traits<Alloc>::construct(_alloc, &(temp->value), std::forward<Args>(args)...);

    iterator it = find(temp->value.first);

    if (it != end()) {
      std::allocator_traits<Alloc>::destroy(_alloc, &temp->value);
      std::allocator_traits<NodeAlloc>::deallocate(_node_alloc, temp, 1);
      return {it, false};
    }

    size_t hash = _hash_func(temp->value.first) % _table.size();

    if (_table[hash] == std::nullopt) {
      _table[hash] = end();
    }

    _elements.insertNode((*_table[hash]), temp);
    --(*_table[hash]);
    static_cast<Node*>(_table[hash]->current)->hash = hash;

    if (load_factor() > _max_load_factor) {
      rehash(2 * _table.size());
    }

    return {*_table[temp->hash], true};
  }

  iterator
  find(const Key &value) {
    size_t hash = _hash_func(value);
    hash %= _table.size();

    if (_table[hash] == std::nullopt) {
      return end();
    }

    for (iterator it = (*_table[hash]); it != end(); ++it) {
      if (it.get_node()->hash != hash) {
        return end();
      }
      if (_equal_func(value, it.get_node()->value.first)) {
        return it;
      }
    }
    return end();
  }

  const_iterator
  find(const Key &value) const {
    size_t hash = _hash_func(value);
    hash %= _table.size();

    if (_table[hash] == std::nullopt) {
      return cend();
    }

    for (const_iterator it = (*_table[hash]); it != cend(); ++it) {
      if (it.get_node()->hash != hash) {
        return cend();
      }
      if (_equal_func(value, it.get_node()->value.first)) {
        return it;
      }
    }
    return cend();
  }

  void
  reserve(size_t size) { rehash(static_cast<size_t>(size / _max_load_factor)); }

  void rehash(size_t size) {
    if (size <= _table.size()) {
      return;
    }
    _table.clear();
    _table.resize(size);

    iterator cur_it = begin();
    iterator next_it = cur_it;
    ++next_it;
    while (cur_it != end()) {
      size_t hash = _hash_func((*cur_it).first);
      hash %= _table.size();
      
      if (_table[hash] == std::nullopt) {
        _table[hash] = end();
      }

      iterator used_it = (*_table[hash]);
      if (used_it != end()) {
        cur_it.current->last->next = next_it.current;
        next_it.current->last = cur_it.current->last;

        cur_it.current->next = used_it.current;
        cur_it.current->last = used_it.current->last;

        used_it.current->last = cur_it.current;
        cur_it.current->last->next = cur_it.current;
      }
      _table[hash] = cur_it;
      static_cast<Node*>(cur_it.current)->hash = hash;
      cur_it = next_it;
      ++next_it;
    }
  }

  double
  load_factor() const { return static_cast<double>(_elements.size()) / _table.size(); }

  size_t
  max_load_factor() const { return _max_load_factor; }

  void
  max_load_factor(double new_max_load_factor) { _max_load_factor = new_max_load_factor; }

  void
  swap(UnorderedMap &other) {
    std::swap(_table, other._table);
    std::swap(_hash_func, other._hash_func);
    std::swap(_equal_func, other._equal_func);
    if (std::allocator_traits<Alloc>::propagate_on_container_swap::value == true) {
      std::swap(_alloc, other._alloc);
    }
    _elements.swap(other._elements);
    std::swap(_max_load_factor, other._max_load_factor);
  }

  iterator begin() { return _elements.begin(); }
  iterator end() { return _elements.end(); }

  const_iterator cbegin() const { return static_cast<const_iterator>(_elements.begin()); }
  const_iterator cend() const { return static_cast<const_iterator>(_elements.end()); }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
  const_reverse_iterator rbegin() const { return crbegin(); }
  const_reverse_iterator rend() const { return crend(); }

private:
  template <class NodeType_list, class Alloc_list>
  friend class List;

  [[no_unique_address]] Hash _hash_func;
  [[no_unique_address]] Equal _equal_func;
  [[no_unique_address]] Alloc _alloc;
  [[no_unique_address]] NodeAlloc _node_alloc;
  List<NodeType, Alloc> _elements;
  std::vector<std::optional<iterator>, optional_iterator_alloc> _table;
  double _max_load_factor;
};

template < class NodeType, class Alloc>
void List<NodeType, Alloc>::initialize_end_node() {
  _end_node.last = &_end_node;
  _end_node.next = &_end_node;
}

template < class NodeType, class Alloc>
void List<NodeType, Alloc>::clear() {
  while(_size != 0) {
    this->pop_back();
  }
}

template < class NodeType, class Alloc>
void List<NodeType, Alloc>::erase(const_iterator it) {
  if (it == cend()) {
    return;
  }
  --_size;
  it.current->last->next = it.current->next;
  it.current->next->last = it.current->last;
  std::allocator_traits<allocator_node>::destroy(alloc, static_cast<Node*>(it.current));
  std::allocator_traits<allocator_node>::deallocate(alloc, static_cast<Node*>(it.current), 1);
}

template < class NodeType, class Alloc>
void List<NodeType, Alloc>::insertNode(iterator it, Node* newNode) {
  ++_size;
  it.current->last->next = newNode;
  newNode->last = it.current->last;
  it.current->last = newNode;
  newNode->next = it.current;
}

template < class NodeType, class Alloc>
template <class... Args>
void List<NodeType, Alloc>::emplace(const_iterator it, Args&&... args) {
  Node* current;
  try{
    current = std::allocator_traits<allocator_node>::allocate(alloc, 1);
    std::allocator_traits<allocator_node>::construct(alloc, &current->value, std::forward<Args>(args)...);

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