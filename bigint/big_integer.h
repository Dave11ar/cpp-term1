//
// Created by dave11ar on 28.04.2020.
//

#ifndef HW02_BIG_INTEGER__BIG_INTEGER_H_
#define HW02_BIG_INTEGER__BIG_INTEGER_H_

#include <string>
#include <vector>
#include <functional>

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
  bool sign;
  std::vector<uint32_t> value;

  big_integer(uint32_t value);


  int32_t compare(big_integer const& b) const;

  size_t size() const;
  void pop_back();
  void push_back(uint32_t a);

  uint32_t& operator[](size_t i);
  uint32_t const& operator[](size_t i) const;

  std::pair<big_integer, big_integer> div_mod(big_integer const& b);
  void normalize();
  uint32_t short_div (uint32_t b);
  uint32_t trial(big_integer const &b);
  bool smaller(big_integer const &b, size_t m);
  void difference(big_integer const &b, size_t m);

  void additional_code();
  big_integer binary_operation(big_integer b, const std::function<uint32_t(uint32_t, uint32_t)>&);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif //HW02_BIG_INTEGER__BIG_INTEGER_H_
