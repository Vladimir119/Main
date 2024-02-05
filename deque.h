#pragma once

#include <iostream>

template <class T>
class Deque{
 private:
  struct block {
    static const size_t SIZE = 256;
    T* array;
    void clear_array() {
      delete[] reinterpret_cast<char*>(array);
    }
  };

  template <bool is_const>
  class base_iterator {
   public:
    using  value_type = std::conditional_t<is_const, const T, T>;
    using   pointer = std::conditional_t<is_const, const T*, T*>;
    using                                  difference_type = int;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using    iterator_category = std::random_access_iterator_tag;

    reference operator*() const { return *_element; }

    base_iterator& operator+=(difference_type distance) {
      if (distance >= 0) {
        plus_positive(distance);
      } else {
        minus_positive(-distance);
      }
      return *this;
    }

    base_iterator& operator-=(difference_type distance) {
      *this += -distance;
      return *this;
    }
    
    friend base_iterator operator+(base_iterator other, difference_type distance) {
      other += distance;
      return other;
    }

    friend base_iterator operator+(difference_type distance, base_iterator other) {
      other += distance;
      return other;
    }

    friend base_iterator operator-(base_iterator other ,difference_type distance) {
      other -= distance;
      return other;
    }

    base_iterator& operator++() {
      *this += 1;
      return *this;
    }

    base_iterator& operator--() {
      *this -= 1;
      return *this;
    }
  
    base_iterator operator++(difference_type) {
      base_iterator this_copy = *this;
      ++(*this);
      return this_copy;
    }

    base_iterator operator--(difference_type) {
      base_iterator this_copy = *this;
      --(*this);  
      return this_copy;
    }

    difference_type operator-(const base_iterator& other) const {
      return block::SIZE * ( _block - other._block) +
                           (_element - _begin_block) -
                           (other._element - other._begin_block);
    }

    operator base_iterator<true>() const {
      base_iterator<true> it;
      it._block = _block;
      it._begin_block  = _begin_block;
      it._element = _element;
      return it;
    }

    pointer operator->() const { return _element; }

    std::strong_ordering operator<=>(const base_iterator& other) const {
      difference_type distance = (*this) - other;
      if (distance > 0) {
        return std::strong_ordering::greater;
      }
      if (distance < 0 ) {
        return std::strong_ordering::less;
      }
      return std::strong_ordering::equal;
    }

    bool operator==(const base_iterator& other) const {
      return *this - other == 0;
    }

   private:
    void plus_positive(size_t distance) {
      size_t offset = _element - _begin_block;
      size_t new_distance = distance + offset;
      _block += new_distance/block::SIZE;
      _begin_block = _block->array;
      _element = _begin_block + new_distance % block::SIZE;
    }

    void minus_positive(size_t distance) {
      size_t offset = _element - _begin_block;
      if (offset >= distance) {
        _element -= distance;
      } else {
        size_t new_distance = distance - offset;
        _block -= new_distance / block::SIZE;
        if (new_distance % block::SIZE != 0) {
          _block -= 1;
          _begin_block = _block->array;
          _element = _begin_block + block::SIZE - (new_distance % block::SIZE);
        } else {
          _begin_block = _block->array;
          _element = _begin_block;
        }
      }
    }

    friend class Deque;

    block* _first_element_in_array;
    block* _block;
    T* _begin_block;
    T* _element;
  };

public:
  using const_iterator = base_iterator<true>;
  using iterator = base_iterator<false>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            Deque();
   explicit Deque(int count);
   explicit Deque(int count, const T& element);
            Deque(const Deque<T>& other);
           ~Deque();

  Deque& operator=(Deque<T> other);
  T&        operator[](size_t index);
  const T&  operator[](size_t index) const;

  size_t   size() const;
  const T& at(size_t index) const;
  T&       at(size_t index);
  void     push_back(const T& element);
  void     pop_back();
  void     push_front(const T& element);
  void     pop_front();
  void     insert(iterator it, const T& element);
  void     erase(iterator it);
 
  iterator begin(); 
  iterator end();
  const_iterator cbegin() const;
  const_iterator cend() const;
  const_iterator begin() const;
  const_iterator end() const;
  
  reverse_iterator rbegin();
  reverse_iterator rend();
  const_reverse_iterator crbegin() const;
  const_reverse_iterator crend() const;
  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;

 private:
  void clear_deque(iterator it) const;
  void resize();
  block* reserve_array(size_t size);
  void initializing_iterators(size_t end_offset_form_begin);

  iterator begin_it, end_it;
  block* _array;
  size_t _count_of_block;
};

template<class T>
typename Deque<T>::block* Deque<T>::reserve_array(size_t size) {
  block* temp_array = new block[size];
  for (size_t i = 0; i < size; ++i) {
    temp_array[i].array = reinterpret_cast<T*>(new char[block::SIZE * sizeof(T)]);
  }
  _count_of_block = size;
  return temp_array;
}

template<class T>
void Deque<T>::initializing_iterators(size_t end_offset_form_begin) {
  begin_it._first_element_in_array = _array;
  begin_it._block = _array + 1;
  begin_it._begin_block = begin_it._block->array;
  begin_it._element = begin_it._begin_block;
  end_it = begin_it + end_offset_form_begin;
}

template<class T>
void Deque<T>::clear_deque(Deque<T>::iterator it) const {
  for (auto it2 = begin(); it2 != it; ++it2) {
    (it2._element)->~T();
  }
  for (size_t i = 0; i < _count_of_block; ++i) {
     _array[i].clear_array();
  }
  delete[] _array;
}

template<class T>
void Deque<T>::resize() {
  block* new_array = new block[_count_of_block * 3];
  for (size_t i = _count_of_block; i < _count_of_block * 2; ++i) {
    new_array[i] = _array[i - _count_of_block];
  }
  for (size_t i = 0; i < _count_of_block; ++i) {
    new_array[i].array = reinterpret_cast<T*>(new char[block::SIZE * sizeof(T)]);
  }
  for (size_t i = _count_of_block * 2; i < _count_of_block * 3; ++i) {
    new_array[i].array = reinterpret_cast<T*>(new char[block::SIZE * sizeof(T)]);
  }

  int old_index_end = end_it._block - end_it._first_element_in_array;
  int old_index_begin = begin_it._block - begin_it._first_element_in_array;

  delete[] _array;
  _array = new_array;

  begin_it._first_element_in_array = _array;
  end_it._first_element_in_array = _array;

  begin_it._block = _array + _count_of_block + old_index_begin;
  end_it._block = _array + _count_of_block + old_index_end;

  _count_of_block *= 3;
}

template <class T>
Deque<T>::Deque(): _array(reserve_array(3)) {
  initializing_iterators(0);
}

template<class T>
Deque<T>::Deque(int count): _array(reserve_array(count / block::SIZE + count % block::SIZE + 2)) {
  initializing_iterators(count);

  for (auto it = begin(); it != end(); ++it) {
    try {
      new(it._element) T();
    } catch(...) {
      clear_deque(it);
      throw;
    }
  }
}

template <class T>
Deque<T>::~Deque() {
  clear_deque(end());
}

template <class T>
Deque<T>::Deque(int count, const T& element): _array(reserve_array(count / block::SIZE + count % block::SIZE + 2)) {  
  initializing_iterators(count);

  for (auto it = begin(); it != end(); ++it) {
    try {
      new(it._element) T(element);
    } catch(...) {
      clear_deque(it);
      throw;
    }
  }
}

template <class T>
Deque<T>::Deque(const Deque<T>& other): _array(reserve_array(other._count_of_block)) {
  int len = other.end() - other.begin();
  initializing_iterators(len);
  for (int i = 0; i < len; ++i) {
    try {
      new((begin()+i)._element) T(*(other.begin()+i));
    } catch (...) {
      clear_deque(begin() + i);
      throw;
    }
  }
}

template<class T>
Deque<T>& Deque<T>::operator=(Deque<T> other) {
  std::swap(other._array, _array);
  std::swap(other._count_of_block, _count_of_block);
  std::swap(other.begin_it, begin_it);
  std::swap(other.end_it, end_it);

  return *this;
}

template<class T>
T& Deque<T>::operator[](size_t index) {
  return *(begin() + index);
}

template<class T>
const T& Deque<T>::operator[](size_t index) const {
  return *(begin() + index);
}

template<class T>
size_t Deque<T>::size() const {
  return end_it - begin_it;
}

template<class T>
T& Deque<T>::at(size_t index) {
  if (index >= size()) {
    throw std::out_of_range("Out of range!");
  } else {
    return (*this)[index];
  }
}

template<class T>
const T& Deque<T>::at(size_t index) const {
  if (index >= size()) {
    throw std::out_of_range("Out of range!");
  } else {
    return (*this)[index];
  }
}

template<class T>
void Deque<T>::push_back(const T& element) {
  try {
    new(end()._element) T(element);
  } catch(...) {
    throw;
  }
  end_it += 1;
  if (end_it._block == (&_array[_count_of_block - 1])) {
    resize();
  }
}

template<class T>
void Deque<T>::pop_back() {
  ((end() - 1)._element)->~T();
  end_it -= 1;
}

template<class T>
void Deque<T>::push_front(const T& element) {
   try {
    new((begin() - 1)._element) T(element);
  } catch(...) {
    throw;
  }
  begin_it -= 1;
  if (begin_it._block == (&_array[0])) {
    resize();
  }
}

template<class T>
void Deque<T>::pop_front() {
  (begin()._element)->~T();
  begin_it += 1;
}

template<class T>
void Deque<T>::insert(Deque<T>::iterator it, const T& el) {
  for (auto it_cur = end() - 1; it_cur >= it; --it_cur) {
    new((it_cur + 1)._element) T(*it_cur);
    (it_cur._element)->~T();
  }
  new(it._element) T(el);
  end_it += 1;
  if (end_it._block == (&_array[_count_of_block - 1])) {
    resize();
  }
}

template<class T>
void Deque<T>::erase(Deque<T>::iterator it) {
  for (auto it_cur = it; it_cur < end() - 1; ++it_cur) {
    (it_cur._element)->~T();
    new(it_cur._element) T(*(it_cur + 1));
  }
  ((end()-1)._element)->~T();
  end_it -= 1;
}

template<class T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  return const_iterator(begin_it);
}

template<class T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  return const_iterator(end_it);
}

template<class T>
typename Deque<T>::iterator Deque<T>::begin() {
  return begin_it;
}

template<class T>
typename Deque<T>::iterator Deque<T>::end() {
  return end_it;
}

template<class T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
  return cbegin();
}

template<class T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return cend();
}

template<class T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
  return const_reverse_iterator(end());
}

template<class T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
  return const_reverse_iterator(begin());
}

template<class T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
  return reverse_iterator(end());
}

template<class T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
  return reverse_iterator(begin());
}

template<class T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const {
  return const_reverse_iterator(end());
}

template<class T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const{
  return const_reverse_iterator(begin());
}