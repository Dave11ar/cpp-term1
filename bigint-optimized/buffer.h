//
// Created by dave11ar on 30.06.2020.
//

#ifndef BIGINT__BUFFER_H_
#define BIGINT__BUFFER_H_

#include "shared_container.h"

struct buffer {
  static constexpr size_t MAX_SMALL = 2;

  explicit buffer(uint32_t a) : size(1), small(true) {
    small_data[0] = a;
  }

  buffer(buffer const& a) : size(a.size), small(a.small) {
    if (small) {
      std::copy(a.small_data, a.small_data + a.size, small_data);
    } else {
      shared_data = a.shared_data;
      shared_data->increase_ref();
    }
  }

  ~buffer() {
    if (!small) {
      shared_data->delete_instance();
    }
  }

  uint32_t& operator[](size_t i) {
    if (small) {
      return small_data[i];
    } else {
      unshare();
      return (*shared_data)[i];
    }
  }

  uint32_t const& operator[](size_t i) const {
    return small ? small_data[i] : (*shared_data)[i];
  }

  uint32_t const& back() const {
    return small ? small_data[size - 1] : shared_data->back();
  }

  size_t get_size() const {
    return size;
  }

  buffer& operator=(buffer const& a) {
    if (*this == a) {
      return *this;
    }

    this->~buffer();
    size = a.size;
    small = a.small;
    if (a.small) {
      std::copy(a.small_data, a.small_data + a.size, small_data);
    } else {
      shared_data = new shared_container(*a.shared_data);
    }

    return *this;
  }

  friend bool operator==(buffer const& a, buffer const& b) {
    if (a.size != b.size) {
      return false;
    }

    for (size_t i = a.size; i > 0; i--) {
      if (a[i - 1] != b[i - 1]) {
        return false;
      }
    }
    return true;
  }

  void push_back(uint32_t a) {
    if (small) {
      if (size == MAX_SMALL) {
        small = false;

        std::vector<uint32_t> tmp(small_data, small_data + size);
        tmp.push_back(a);
        shared_data = new shared_container(tmp);
      } else {
        small_data[size] = a;
      }
    } else {
      unshare();
      shared_data->push_back(a);
    }
    size++;
  }

  void pop_back() {
    if (!small) {
      unshare();
      shared_data->pop_back();
    }
    size--;
  }

  void reverse() {
    if (small) {
      std::reverse(small_data, small_data + size);
    } else {
      unshare();
      shared_data->reverse();
    }
  }

  void unshare() {
    shared_data = shared_data->make_unique();
  }

 private:
  size_t size;
  bool small;
  union {
    uint32_t small_data[MAX_SMALL];
    shared_container* shared_data;
  };
};

#endif //BIGINT__BUFFER_H_
