#pragma once
#include <vector>

template < class T >
class SparseTable {
public:

  SparseTable(const std::vector<T>& arr): _arr(nullptr),
                                          _size( arr.size() ),
                                          _log_size (Log(_size)),
                                          _deg(std::vector<int>(_size + 1, 0)),
                                          _sparse(std::vector<std::vector<T>>(_log_size + 1, std::vector<T>(_size))) {
    _sparse[0] = arr;
    CalculateSparse();
  }

  SparseTable( T* arr, size_t size ): _arr( arr ),
                                    _size( size ),
                                    _log_size( Log(_size ) ),
                                    _deg(std::vector<int>(_size + 1, 0)),
                                    _sparse(std::vector<std::vector<T>>(_log_size + 1, std::vector<T>(_size))) {
    CalculateSparse();
  }

  T GetMin ( size_t l, size_t r ) {
    size_t k = _deg[ r - l + 1 ];
    return std::min(_sparse[ k ][ l ], _sparse[ k ][ r - ( 1 << k ) + 1 ] );
  }

private:

  size_t Log(size_t number) {
    size_t log = 0;
    size_t two_in_deg_log = 1;

    while (number > two_in_deg_log) {
      two_in_deg_log *= 2;
      ++log;
    }

    if (two_in_deg_log > number) {
      --log;
    }

    return  log;
  }

  void CalculateSparse() {

    //calculate _deg array
    _deg[0] = 0;
    _deg[1] = 0;
    for (size_t i = 2; i < _deg.size(); ++i) {
      _deg[i] = _deg[i - 1];
      if ((i & (i - 1)) == 0) {
        ++_deg[i];
      }
    }

    //calculate _sparse array
    if (_arr != nullptr) {
      for (int i = 0; i < _size; ++i) {
        _sparse[0][i] = _arr[i];
      }
    }

    for (int k = 1; k <= _log_size; ++k) {
      for (int i = 0; i <= _size - (1 << k); ++i) {
        size_t j = i + (1 << (k - 1));
        _sparse[k][i] = std::min(_sparse[k - 1][i], _sparse[k - 1][j]);
      }
    }
  }

  T* _arr;
  size_t _size;
  size_t _log_size;
  std::vector<int> _deg;
  std::vector<std::vector<T>> _sparse;

};
