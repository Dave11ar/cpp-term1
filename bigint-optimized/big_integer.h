//
// Created by dave11ar on 28.04.2020.
//

#ifndef HW02_BIG_INTEGER__BIG_INTEGER_H_
#define HW02_BIG_INTEGER__BIG_INTEGER_H_

#include <string>
#include <utility>
#include "buffer.h"

struct big_integer {
  big_integer();
  big_integer(big_integer const& value);
  big_integer(int value);
  explicit big_integer(std::string const& str);
  ~big_integer();

  big_integer& operator=(big_integer const& a);

  big_integer& operator+=(big_integer const& a);
  big_integer& operator-=(big_integer const& a);
  big_integer& operator*=(big_integer const& a);
  big_integer& operator/=(big_integer const& a);
  big_integer& operator%=(big_integer const& a);
  big_integer& operator&=(big_integer const& a);
  big_integer& operator|=(big_integer const& a);
  big_integer& operator^=(big_integer const& a);

  big_integer& operator<<=(int value);
  big_integer& operator>>=(int value);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend big_integer operator+(big_integer a, big_integer const& b);
  friend big_integer operator-(big_integer a, big_integer const& b);
  friend big_integer operator/(big_integer a, big_integer const& b);
  friend big_integer operator*(big_integer a, big_integer const& b);
  friend big_integer operator%(big_integer a, big_integer const& b);

  friend big_integer operator&(big_integer a, big_integer const& b);
  friend big_integer operator|(big_integer a, big_integer const& b);
  friend big_integer operator^(big_integer a, big_integer const& b);

  friend big_integer operator<<(big_integer a, int b);
  friend big_integer operator>>(big_integer a, int b);

  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);

 private:
  buffer<uint32_t> value;

  big_integer(uint32_t value);
  explicit big_integer(size_t n);

  uint32_t& operator[](size_t i);
  bool& sign();
  void push_back(uint32_t x);
  void pop_back();
  void remove_zero();
  void reverse();

  uint32_t const& operator[](size_t i) const;
  bool const& sign() const;
  size_t const& size() const;
  uint32_t const& back() const;

  void short_div(uint32_t b);
  uint32_t trial(big_integer const &b);
  bool smaller(big_integer const &b, size_t m);
  void difference(big_integer const &b, size_t m);

  void additional_code();
  big_integer binary_operation(big_integer const& b, uint32_t (*func)(uint32_t, uint32_t));
};

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif //HW02_BIG_INTEGER__BIG_INTEGER_H_
