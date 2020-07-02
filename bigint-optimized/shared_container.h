//
// Created by dave11ar on 02.07.2020.
//

#ifndef BIGINT__SHARED_CONTAINER_H_
#define BIGINT__SHARED_CONTAINER_H_

#include <vector>
#include <algorithm>

template <typename T>
struct shared_container {
  ~shared_container()  = default;

  explicit shared_container(std::vector<T> const& a) : ref_counter(1), vec(a) {}
  shared_container(shared_container<T> const& a) : ref_counter(1), vec(a.vec) {}

  bool unique() {
    return ref_counter == 1;
  }

  shared_container<T>* make_unique() {
    if (unique()) {
      return this;
    } else {
      ref_counter--;
      return new shared_container<T>(*this);
    }
  }

  void clear_mem() {
    if (unique()) {
      delete this;
    } else {
      ref_counter--;
    }
  }

  void reverse() {
    std::reverse(vec.begin(), vec.end());
  }

  void pop_back() {
    vec.pop_back();
  }

  void push_back(T a) {
    vec.push_back(a);
  }

  T const& back() const {
    return vec.back();
  }

  T& operator[](size_t i) {
    return vec[i];
  }

  T const& operator[](size_t i) const {
    return vec[i];
  }

  void increase_ref() {
    ref_counter++;
  }

 private:
  size_t ref_counter;
  std::vector<T> vec;
};

#endif //BIGINT__SHARED_CONTAINER_H_
