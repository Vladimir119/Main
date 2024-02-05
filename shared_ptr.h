#include<iostream>
#include<memory>
#include <type_traits>

template<class T>
class SharedPtr;

template<class T>
class WeakPtr;

struct BaseControlBlock {
  virtual void useDeleter() = 0;
  virtual void deallocate() = 0;
  virtual ~BaseControlBlock() = default;

  size_t shared_count = 1;
  size_t weak_count = 0;
};

template<class T, class Deleter, class Alloc>
struct ControlBlockRegular: BaseControlBlock {
  ControlBlockRegular(T* object, Deleter deleter, Alloc alloc) : object(object), deleter(deleter), alloc(alloc) {}

  void useDeleter() override { deleter(object); }
  void deallocate() override;
  ~ControlBlockRegular() override = default;

  T* object;
  Deleter deleter;
  Alloc alloc;
};

template< class T, class Alloc >
struct ControlBlockMakeShared: BaseControlBlock {
  template< class... Args>
  ControlBlockMakeShared(Alloc alloc, Args&&... args): alloc(alloc), object(std::forward<Args>(args)...) {}

  void deallocate() override;
  void useDeleter() override { std::allocator_traits<Alloc>::destroy(alloc, &object); }
  ~ControlBlockMakeShared() override = default;

  Alloc alloc;
  T object;
};

template<class T>
class SharedPtr{
  using element_type = T;
  using weak_type = WeakPtr<T>;

  template< class Y, class... Args >
  friend SharedPtr<Y> makeShared(Args&&...);

  template< class Y, class Alloc, class... Args >
  friend SharedPtr<Y> allocateShared(const Alloc&, Args&&...);

  template< class Y >
  friend class WeakPtr;

  template< class Y >
  friend class SharedPtr;

 public:
  SharedPtr(): _ptr(nullptr), _cb(nullptr) {}

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  explicit SharedPtr( Y* ptr);

  template< class Y, class Deleter > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  SharedPtr( Y* ptr, Deleter deleter );

  template< class Y, class Deleter, class Alloc > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  SharedPtr( Y* ptr, Deleter deleter, Alloc alloc );

  SharedPtr( const SharedPtr& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  SharedPtr( const SharedPtr<Y>& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  SharedPtr( SharedPtr<Y>&& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  explicit SharedPtr( const WeakPtr<Y>& other );

  SharedPtr& operator=( const SharedPtr& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  SharedPtr& operator=( const SharedPtr<Y>& other ) noexcept;

  SharedPtr& operator=( SharedPtr&& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  SharedPtr& operator=( SharedPtr<Y>&& other ) noexcept;

  void reset() noexcept { SharedPtr().swap(*this); }

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  void reset( Y* ptr ) { SharedPtr(ptr).swap(*this); }

  template< class Y, class Deleter > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  void reset( Y* ptr, Deleter deleter ) { SharedPtr(ptr, deleter).swap(*this); }

  template< class Y, class Deleter, class Alloc > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  void reset( Y* ptr, Deleter deleter, Alloc alloc ) { SharedPtr(ptr, deleter, alloc).swap(*this); }

  void swap( SharedPtr& other ) noexcept;

  element_type* get() const noexcept { return _ptr; }

  element_type& operator*() const noexcept { return *get(); }
  element_type* operator->() const noexcept { return get(); }

  size_t use_count() const noexcept;

  bool unique() const noexcept { return use_count() == 1; }

  explicit operator bool() const noexcept { return get() != nullptr; }

  ~SharedPtr();

 private:
  template< class Alloc>
  SharedPtr(ControlBlockMakeShared<element_type, Alloc>* cb): _ptr(&cb->object), _cb(cb) {}

  element_type* _ptr;
  BaseControlBlock* _cb;
};

template<class T>
class WeakPtr{
  template< class Y >
  friend class WeakPtr;

  template< class Y >
  friend class SharedPtr;

 public:

  constexpr WeakPtr() noexcept : _ptr(nullptr), _cb(nullptr) {}

  WeakPtr( const WeakPtr& other ) noexcept;

  template< class Y  > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  WeakPtr( const WeakPtr<Y>& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  WeakPtr( const SharedPtr<Y>& other ) noexcept;

  WeakPtr( WeakPtr&& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  WeakPtr( WeakPtr<Y>&& other ) noexcept;

  WeakPtr& operator=( const WeakPtr& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  WeakPtr& operator=( const WeakPtr<Y>& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  WeakPtr& operator=( const SharedPtr<Y>& other ) noexcept;

  WeakPtr& operator=( WeakPtr&& other ) noexcept;

  template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
  WeakPtr& operator=( WeakPtr<Y>&& other ) noexcept;

  void swap( WeakPtr& other ) noexcept;

  void reset() noexcept { WeakPtr().swap(*this); }

  size_t use_count() const noexcept;

  bool expired() const noexcept { return use_count() == 0; }

  SharedPtr<T> lock() const noexcept { return expired() ? SharedPtr<T>() : SharedPtr<T>(*this); }

  ~WeakPtr();

 private:
  T* _ptr;
  BaseControlBlock* _cb;
};

template<class T, class... Args>
SharedPtr<T> makeShared(Args&&... args) {
  auto cb = new ControlBlockMakeShared<T, std::allocator<T>>(std::allocator<T>(), std::forward<Args>(args)...);
  return SharedPtr<T>(cb);
}

template<class T, class Alloc, class... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
  using alloc_cb_make_shared = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
  alloc_cb_make_shared alloc_ = alloc;
  auto cb = std::allocator_traits<alloc_cb_make_shared>::allocate(alloc_, 1);
  std::allocator_traits<alloc_cb_make_shared>::construct(alloc_, cb, alloc, std::forward<Args>(args)...);
  return SharedPtr<T>(cb);
}

/////////REALIZATION///////////

template<class T, class Deleter, class Alloc>
void ControlBlockRegular<T, Deleter, Alloc>::deallocate() {
  using control_block_regular_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
  control_block_regular_alloc alloc_ = alloc;
  std::allocator_traits<control_block_regular_alloc>::deallocate(alloc_, this, 1);
}

template< class T, class Alloc >
void ControlBlockMakeShared<T, Alloc>::deallocate() {
  using control_block_make_shared_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared< T, Alloc >>;
  control_block_make_shared_alloc alloc_ = alloc;
  std::allocator_traits<control_block_make_shared_alloc>::deallocate(alloc_, this, 1);
}

template< class T >
class EnableSharedFromThis {
  template < class U >
  friend class SharedPtr;

 public:
  SharedPtr<T> shared_from_this() const { return wptr.lock(); }

 private:
  WeakPtr<T> wptr;
};

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>::SharedPtr( Y* ptr): _ptr(ptr), _cb(new ControlBlockRegular<element_type,
                                                std::default_delete<element_type>,
                                                std::allocator<element_type>>(_ptr, std::default_delete<element_type>(),
                                                std::allocator<element_type>())) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
    _ptr->wptr = *this;
  }
}

template< class T >
template< class Y, class Deleter > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>::SharedPtr( Y* ptr, Deleter deleter ): _ptr(ptr),
                                      _cb(new ControlBlockRegular<Y, Deleter, std::allocator<Y>>
                                      (ptr, deleter, std::allocator<Y>())) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
    _ptr->wptr = *this;
  }
}

template< class T >
template< class Y, class Deleter, class Alloc > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>::SharedPtr( Y* ptr, Deleter deleter, Alloc alloc ): _ptr(ptr) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
    _ptr->wptr = *this;
  }
  using control_block_regular_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<element_type, Deleter, Alloc>>;
  control_block_regular_alloc _alloc = alloc;
  auto cb = std::allocator_traits<control_block_regular_alloc>::allocate(_alloc, 1);
  new (cb) ControlBlockRegular<element_type, Deleter, Alloc>(_ptr, deleter, alloc);
  _cb = cb;
}

template< class T >
SharedPtr<T>::SharedPtr( const SharedPtr& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  if (_cb != nullptr) {
    ++_cb->shared_count;
  }
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>::SharedPtr( const SharedPtr<Y>& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  if (_cb != nullptr) {
    ++_cb->shared_count;
  }
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>::SharedPtr( SharedPtr<Y>&& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  other._cb = nullptr;
  other._ptr = nullptr;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>::SharedPtr( const WeakPtr<Y>& other ) : _ptr(other._ptr), _cb(other._cb) {
  if (_cb != nullptr) {
    ++_cb->shared_count;
  }
}

template< class T >
SharedPtr<T>& SharedPtr<T>::operator=( const SharedPtr& other ) noexcept {
  SharedPtr(other).swap(*this);
  return *this;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>& SharedPtr<T>::operator=( const SharedPtr<Y>& other ) noexcept {
  SharedPtr(other).swap(*this);
  return *this;
}

template< class T >
SharedPtr<T>& SharedPtr<T>::operator=( SharedPtr&& other ) noexcept {
  SharedPtr(std::move(other)).swap(*this);
  return *this;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
SharedPtr<T>& SharedPtr<T>::operator=( SharedPtr<Y>&& other ) noexcept {
  SharedPtr(std::move(other)).swap(*this);
  return *this;
}

template< class T >
void SharedPtr<T>::swap( SharedPtr& other ) noexcept {
  std::swap(_ptr, other._ptr);
  std::swap(_cb, other._cb);
}

template< class T >
size_t SharedPtr<T>::use_count() const noexcept {
  if (_cb != nullptr) {
    return _cb->shared_count;
  }
  return 0;
}

template< class T >
SharedPtr<T>::~SharedPtr() {
  if (_cb != nullptr) {
    --_cb->shared_count;
    if (_cb->shared_count == 0) {
      _cb->useDeleter();
      if (_cb->weak_count == 0) {
        _cb->deallocate();
      }
    }
  }
}

template< class T >
WeakPtr<T>::WeakPtr( const WeakPtr& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  if (_cb != nullptr) {
    ++_cb->weak_count;
  }
}

template< class T >
template< class Y  > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
WeakPtr<T>::WeakPtr( const WeakPtr<Y>& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  if (_cb != nullptr) {
    ++_cb->weak_count;
  }
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
WeakPtr<T>::WeakPtr( const SharedPtr<Y>& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  if (_cb != nullptr) {
    ++_cb->weak_count;
  }   
}

template< class T >
WeakPtr<T>::WeakPtr( WeakPtr&& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  other._ptr = nullptr;
  other._cb = nullptr;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
WeakPtr<T>::WeakPtr( WeakPtr<Y>&& other ) noexcept : _ptr(other._ptr), _cb(other._cb) {
  other._ptr = nullptr;
  other._cb = nullptr;
}

template< class T >
WeakPtr<T>& WeakPtr<T>::operator=( const WeakPtr& other ) noexcept {
  WeakPtr(other).swap(*this);
  return *this;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
WeakPtr<T>& WeakPtr<T>::operator=( const WeakPtr<Y>& other ) noexcept {
  WeakPtr(other).swap(*this);
  return *this;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
WeakPtr<T>& WeakPtr<T>::operator=( const SharedPtr<Y>& other ) noexcept {
  WeakPtr(other).swap(*this);
  return *this;
}

template< class T >
WeakPtr<T>& WeakPtr<T>::operator=( WeakPtr&& other ) noexcept {
  WeakPtr(std::move(other)).swap(*this);
  return *this;
}

template< class T >
template< class Y > requires std::is_same_v < T, Y > || std::is_base_of_v< T, Y >
WeakPtr<T>& WeakPtr<T>::operator=( WeakPtr<Y>&& other ) noexcept {
  WeakPtr(std::move(other)).swap(*this);
  return *this;
}

template< class T >
void WeakPtr<T>::swap( WeakPtr& other ) noexcept {
  std::swap(other._ptr, _ptr);
  std::swap(other._cb, _cb);
}

template< class T >
size_t WeakPtr<T>::use_count() const noexcept {
  if (_cb != nullptr) {
    return _cb->shared_count;
  }
  return 0;
}

template< class T >
WeakPtr<T>::~WeakPtr() {
  if (_cb != nullptr) {
    --_cb->weak_count;

    if (_cb->weak_count == 0 && _cb->shared_count == 0) {
      _cb->deallocate();
    }
  }
}