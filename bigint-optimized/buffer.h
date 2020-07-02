//
// Created by dave11ar on 30.06.2020.
//

#ifndef BIGINT__BUFFER_H_
#define BIGINT__BUFFER_H_

#include "shared_container.h"

template <typename T>
struct buffer {
  static constexpr size_t MAX_SMALL = 2;

  bool sign;

  buffer(bool sign, T a) : sign(sign), size(1), small(true) {
    small_data[0] = a;
  }

  buffer(buffer const& a) : sign(a.sign), size(a.size), small(a.small) {
    if (small) {
      for (size_t i = 0; i < size; i++) {
        small_data[i] = a.small_data[i];
      }
    } else {
      shared_data = a.shared_data;
      shared_data->increase_ref();
    }
  }

  ~buffer() {
    if (!small && shared_data->unique()) {
      delete shared_data;
    }
  }

  T& operator[](size_t i) {
    if (small) {
      return small_data[i];
    } else {
      unshare();
      return (*shared_data)[i];
    }
  }

  T const& operator[](size_t i) const {
    return small ? small_data[i] : (*shared_data)[i];
  }

  T const& back() const {
    return small ? small_data[size - 1] : shared_data->back();
  }

  size_t get_size() const {
    return size;
  }

  buffer& operator=(buffer const& a) {
    buffer tmp(a);
    swap(tmp);
    return *this;
  }

  void swap(buffer& a) {
    using std::swap;

    if (a.small) {
      if (small) {
        swap(small_data, a.small_data);
      } else {
        shared_container<T>* tmp = shared_data;
        std::copy(a.small_data, a.small_data + a.size, small_data);
        a.shared_data = tmp;
      }
    } else {
      if (small) {
        shared_container<T>* tmp = a.shared_data;
        std::copy(small_data, small_data + size, a.small_data);
        shared_data = tmp;
      } else {
        swap(shared_data, a.shared_data);
      }
    }
    swap(small, a.small);
    swap(sign, a.sign);
    swap(size, a.size);
  }

  friend bool operator==(buffer const& a, buffer const& b) {
    if (a.size != b.size || a.sign != b.sign) {
      return false;
    }

    for (size_t i = a.size; i > 0; i--) {
      if (a[i - 1] != b[i - 1]) {
        return false;
      }
    }
    return true;
  }

  void push_back(T a) {
    if (small) {
      if (size == MAX_SMALL) {
        small = false;

        std::vector<T> tmp(small_data, small_data + size);
        tmp.push_back(a);

        shared_data = new shared_container<T>(tmp);
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
    if (small) {
      small_data[size - 1].~T();
    } else {
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
    if (!shared_data->unique()) {
      shared_data = new shared_container<T>(*shared_data);
    }
  }

 private:
  size_t size;
  bool small;
  union {
    T small_data[MAX_SMALL];
    shared_container<T>* shared_data;
  };
};

#endif //BIGINT__BUFFER_H_
