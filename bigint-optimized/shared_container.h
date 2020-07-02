//
// Created by dave11ar on 02.07.2020.
//

#ifndef BIGINT__SHARED_CONTAINER_H_
#define BIGINT__SHARED_CONTAINER_H_

#include <vector>
#include <algorithm>

struct shared_container {
  ~shared_container()  = default;

  explicit shared_container(std::vector<uint32_t> const& a) : ref_counter(1), vec(a) {}
  shared_container(shared_container const& a) : ref_counter(1), vec(a.vec) {}

  bool unique() {
    return ref_counter == 1;
  }

  shared_container* make_unique() {
    if (unique()) {
      return this;
    } else {
      ref_counter--;
      return new shared_container(*this);
    }
  }

  void delete_instance() {
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

  void push_back(uint32_t a) {
    vec.push_back(a);
  }

  uint32_t const& back() const {
    return vec.back();
  }

  uint32_t & operator[](size_t i) {
    return vec[i];
  }

  uint32_t const& operator[](size_t i) const {
    return vec[i];
  }

  void increase_ref() {
    ref_counter++;
  }

 private:
  size_t ref_counter;
  std::vector<uint32_t> vec;
};

#endif //BIGINT__SHARED_CONTAINER_H_
