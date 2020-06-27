//
// Created by dave11ar on 16.05.2020.
//
#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <cassert>
#include <algorithm>

template <typename T>
struct vector
{
  typedef T* iterator;
  typedef T const* const_iterator;

  // O(1) nothrow
  vector() : data_(nullptr), capacity_(0), size_(0) {}

  // O(N) strong
  vector(vector const& other) : vector() {
    copy_all(other, other.size_);
  }

  // O(N) strong
  vector& operator=(vector const& other) {
    vector tmp(other);
    swap(tmp);
    return *this;
  }

  // O(N) nothrow
  ~vector() {
    clear();
    operator delete (data_);
  }

  // O(1) nothrow
  T& operator[](size_t i) {
    return data_[i];
  }
  T const& operator[](size_t i) const {
    return data_[i];
  }

  // O(1) nothrow
  T* data() {
    return data_;
  }

  // O(1) nothrow
  T const* data() const {
    return data_;
  }

  // O(1) nothrow
  size_t size() {
    return size_;
  }

  // O(1) nothrow
  T& front() {
    return data_[0];
  }

  // O(1) nothrow
  T const& front() const {
    return data_[0];
  }

  // O(1) nothrow
  T& back() {
    return data_[size_ - 1];
  }

  // O(1) nothrow
  T const& back() const {
    return data_[size_ - 1];
  }

  // O(1)* strong
  void push_back(T const& a) {
    if (size_ == capacity_) {
      push_back_realloc(a);
    } else {
      new(data_ + size_) T(a);
      size_++;
    }
  }

  // O(1) nothrow
  void pop_back() {
    data_[--size_].~T();
  }

  // O(1) nothrow
  bool empty() const {
    return size_ == 0;
  }

  // O(1) nothrow
  size_t capacity() const {
    return capacity_;
  }

  // O(N) strong
  void reserve(size_t new_capacity) {
    if (new_capacity < capacity_) {
      return;
    }
    copy_all(*this, new_capacity);
  }

  // O(N) strong
  void shrink_to_fit() {
    if (size_ == capacity_) {
      return;
    }
    vector tmp(*this);
    swap(tmp);
  }

  // O(N) nothrow
  void clear() {
    for(size_t i = size_; i > 0; i--) {
      data_[i - 1].~T();
    }

    size_ = 0;
  }

  // O(1) nothrow
  void swap(vector& other) {
    using std::swap;

    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
  }

  // O(1) nothrow
  iterator begin() {
    return data_;
  }

  // O(1) nothrow
  iterator end() {
    return data_ + size_;
  }

  // O(1) nothrow
  const_iterator begin() const {
    return data_;
  }

  // O(1) nothrow
  const_iterator end() const {
    return data_ + size_;
  }

  // O(N) weak
  iterator insert(const_iterator pos, T const& value) {
    size_t p = pos - begin();
    push_back(value);
    for (size_t i = size_ - 1; i > p; i--) {
      std::swap(data_[i - 1], data_[i]);
    }

    return data_ + p;
  }

  // O(N) weak
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  // O(N) weak
  iterator erase(const_iterator first, const_iterator last) {
    size_t erase_size = last - first;

    for (size_t i = first - begin(); i < size_ - erase_size; i++) {
      data_[i] = data_[i + erase_size];
    }
    for (size_t i = 0; i < erase_size; i++) {
      pop_back();
    }

    return (first - begin()) + begin();
  }

 private:
  size_t increase_capacity() const {
    return std::max(2 * capacity_, (size_t)1);
  }

  static T* new_buffer(size_t new_capacity) {
    return new_capacity == 0 ? nullptr : static_cast<T*>(operator new (new_capacity * sizeof(T)));
  }

  void push_back_realloc(T const& a) {
    T tmp(a);
    reserve(increase_capacity());

    new(data_ + size_) T(tmp);
    size_++;
  }

  // O(n) strong
  void copy_all(vector const& other, size_t new_capacity) {
    T *new_data = new_buffer(new_capacity);
    size_t i;

    try {
      for (i = 0; i < other.size_; i++) {
        new(new_data + i) T(other.data_[i]);
      }

      for(i = size_; i > 0; i--) {
        data_[i - 1].~T();
      }
      operator delete(data_);

      data_ = new_data;
      size_ = other.size_;
      capacity_ = new_capacity;
    } catch (...) {
      for ( ; i > 0; i--) {
        new_data[i - 1].~T();
      }

      operator delete(new_data);
      throw;
    }
  }

 private:
  T* data_;
  size_t size_{};
  size_t capacity_{};
};

#endif // VECTOR_H
