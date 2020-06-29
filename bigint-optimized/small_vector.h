//
// Created by dave11ar on 06.06.2020.
//

#ifndef BIGINT__SMALL_VECTOR_H_
#define BIGINT__SMALL_VECTOR_H_

#include <cstdint>
#include <vector>
#include <algorithm>

template <typename T>
struct small_vector {
  small_vector() : small_vector(0, 1) {}

  small_vector(uint32_t val, size_t n) : small(n == 1) {
    if (n > 1) {
      vec = new std::vector<T>(n);
      (*vec)[0] = val;
    } else {
      num = val;
    }
  }

  small_vector(small_vector<T> const &a) : small(a.small) {
    if (a.small) {
      num = a.num;
    } else {
      vec = new std::vector<T>(*a.vec);
    }
  }

  ~small_vector() {
    if (!small) {
      delete vec;
    }
  }

  T& operator[](size_t i){
    return small ? num : (*vec)[i];
  }

  void push_back(T a) {
    if (small) {
      small = false;
      vec = new std::vector<T>({num, a});
    } else {
      vec->push_back(a);
    }
  }

  void pop_back() {
    if (small) {
      num = 0;
    } else {
      if (vec->size() == 2) {
        small = true;
        uint32_t a = (*vec)[0];
        delete vec;
        num = a;
      } else {
        vec->pop_back();
      }
    }
  }

  T back() const {
    return small ? num : vec->back();
  }

  void reverse() {
    if (!small) {
      std::reverse(vec->begin(), vec->end());
    }
  }

  size_t size() {
    return small ? 1 : vec->size();
  }

  bool operator==(small_vector const &a) {
    if (small != a.small) {
      return false;
    }

    return small ? num == a.num : *vec == *a.vec;
  }

 private:
  bool small;

  union {
    T num;
    std::vector<T>* vec;
  };
};

#endif //BIGINT__SMALL_VECTOR_H_
