//
// Created by dave11ar on 30.06.2020.
//

#ifndef BIGINT__BUFFER_H_
#define BIGINT__BUFFER_H_

#include <vector>
const size_t MAX_SMALL = 2;

template <typename T>
struct shared_container {
  size_t ref_counter;
  std::vector<T> vec;

  ~shared_container()  = default;

  explicit shared_container(std::vector<T> const& a) : ref_counter(1), vec(a) {}
};

template <typename T>
struct buffer {
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
      shared_data->ref_counter++;
    }
  }

  ~buffer() {
    if (!small) {
      shared_data->ref_counter--;
      if (shared_data->ref_counter == 0) {
        delete shared_data;
      }
    } else {
      destruct_small();
    }
  }

  T& operator[](size_t i) {
    if (small) {
      return small_data[i];
    } else {
      unshare();
      return shared_data->vec[i];
    }
  }

  T const& operator[](size_t i) const {
    return small ? small_data[i] : shared_data->vec[i];
  }

  T const& back() const {
    return small ? small_data[size - 1] : shared_data->vec.back();
  }

  size_t const& get_size() const {
    return size;
  }

  buffer& operator=(buffer const& a) {
    this->~buffer();

    sign = a.sign;
    size = a.size;
    small = a.small;
    if (small) {
      for (size_t i = 0; i < size; i++) {
        small_data[i] = a.small_data[i];
      }
    } else {
      shared_data = a.shared_data;
      shared_data->ref_counter++;
    }

    return *this;
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

        std::vector<T> tmp;
        for (size_t i = 0; i < size; i++) {
          tmp.push_back(small_data[i]);
        }
        tmp.push_back(a);

        destruct_small();
        shared_data = new shared_container<T>(tmp);
      } else {
        small_data[size] = a;
      }
    } else {
      unshare();
      shared_data->vec.push_back(a);
    }
    size++;
  }

  void pop_back() {
    if (small) {
      small_data[size - 1].~T();
    } else {
      unshare();
      shared_data->vec.pop_back();
    }
    size--;
  }

  void reverse() {
    size_t mid = size / 2;
    if (small) {
      for (size_t i = 0; i < mid; i++) {
        std::swap(small_data[i], small_data[size - 1 - i]);
      }
    } else {
      unshare();
      for (size_t i = 0; i < mid; i++) {
        std::swap(shared_data->vec[i], shared_data->vec[size - 1 - i]);
      }
    }
  }

  void destruct_small() {
    for (size_t i = size; i > 0; i--) {
      small_data[i].~T();
    }
  }

  void unshare() {
    if (shared_data->ref_counter != 1) {
      shared_data->ref_counter--;
      shared_data = new shared_container<T>(shared_data->vec);
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
