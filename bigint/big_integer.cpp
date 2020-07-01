//
// Created by dave11ar on 28.04.2020.
//

#include "big_integer.h"
#include <algorithm>

big_integer::big_integer() : sign(false), value(1) {}

big_integer::big_integer(big_integer const &a) = default;

big_integer::big_integer(int a) : sign(a < 0) {
  value =  {static_cast<uint32_t>(a < 0 ? -static_cast<int64_t>(a) : a)};
}

big_integer::big_integer(uint32_t a) : sign(false), value({a}) {}

big_integer::big_integer(std::string const &str) : big_integer() {
  if (str.empty() || str == "0" || str == "-0") {
    return;
  }

  for (size_t digit = str[0] == '-' || str[0] == '+'; digit < str.length(); digit++) {
    *this *= 10;
    *this += (str[digit] - '0');
  }
  if (str[0] == '-') {
    sign = true;
  }
  normalize();
}

big_integer::~big_integer() = default;

big_integer& big_integer::operator=(big_integer const &a) = default;

big_integer& big_integer::operator+=(big_integer const &a) {
  return *this = *this + a;
}

big_integer& big_integer::operator-=(big_integer const &a) {
  return *this = *this - a;
}

big_integer& big_integer::operator*=(big_integer const &a) {
  return *this = *this * a;
}

big_integer& big_integer::operator/=(big_integer const &a) {
  return *this = *this / a;
}

big_integer& big_integer::operator%=(big_integer const &a) {
  return *this = *this % a;
}

big_integer& big_integer::operator&=(big_integer const &a) {
  return *this = *this & a;
}

big_integer& big_integer::operator|=(big_integer const &a) {
  return *this = *this | a;
}

big_integer& big_integer::operator^=(big_integer const &a) {
  return *this = *this ^ a;
}

big_integer& big_integer::operator<<=(int a) {
  return *this = *this << a;
}

big_integer& big_integer::operator>>=(int a) {
  return *this = *this >> a;
}

big_integer big_integer::operator+() const {
  return big_integer(*this);
}

big_integer big_integer::operator-() const {
  if (size() == 1 && value[0] == 0) {
    return *this;
  }

  big_integer tmp = big_integer(*this);
  tmp.sign = !tmp.sign;
  tmp.normalize();
  return tmp;
}

big_integer big_integer::operator~() const {
  return -(*this) - 1;
}

big_integer &big_integer::operator++() {
  return *this += 1;
}

big_integer big_integer::operator++(int) {
  big_integer r = *this;
  ++*this;
  return r;
}

big_integer &big_integer::operator--() {
  return *this -= 1;
}

big_integer big_integer::operator--(int) {
  big_integer r = *this;
  --*this;
  return r;
}

big_integer operator+(big_integer a, big_integer const& b) {
  if (a.sign == b.sign) {
    uint64_t tmp, carry = 0;

    size_t max_size = std::max(a.size(), b.size());
    for (size_t i = 0; i < max_size; i++) {
      if (i == a.size()) a.push_back(0);
      tmp = carry + a[i] + (i < b.size() ? b[i] : 0);
      carry = tmp > UINT32_MAX;
      a[i] = tmp & UINT32_MAX;
    }
    if (carry > 0) {
      a.push_back(carry);
    }

    a.normalize();
    return a;
  } else {
    return (a.sign ? b - (-a) : a - (-b));
  }
}

big_integer operator-(big_integer a, big_integer const& b) {
  if (a == 0) {
    return -b;
  }
  if (b == 0) {
    return a;
  }

  if (a.sign != b.sign) {
    return (a.sign ? -(-a + b) : a + -b);
  }
  if (a.sign) {
    return (-b - (-a));
  }
  if (a < b) {
    return -(b - a);
  }

  // a >= b > 0;
  uint32_t carry = 0;
  int64_t tmp;
  for (size_t i = 0; i < a.value.size(); i++) {
    tmp = static_cast<int64_t>(a[i]) - carry - (i < b.size() ? b[i] : 0);
    carry = tmp < 0;
    a[i] = tmp < 0 ? static_cast<uint32_t>(tmp + 1 + UINT32_MAX) : tmp;
  }
  a.normalize();

  return a;
}

big_integer operator*(big_integer a, big_integer const& b){
  if (a == 0 || b == 0) {
    return 0;
  }

  big_integer res;
  res.sign = a.sign ^ b.sign;
  res.value.resize(a.size() + b.size());
  for (size_t i = 0; i < a.size(); i++) {
    uint64_t carry = 0;
    for (size_t j = 0; j < b.size(); j++) {
      uint64_t tmp = static_cast<uint64_t>(a[i]) * b[j] + carry + res[i + j];
      res[i + j] = static_cast<uint32_t>(tmp & UINT32_MAX);
      carry = tmp >> 32;
    }
    res[i + b.size()] += carry;
  }
  res.normalize();

  return res;
}


uint32_t big_integer::short_div(uint32_t b) {
  uint64_t carry = 0;
  for (size_t i = size(); i != 0; i--) {
    uint64_t tmp = (carry << 32) + value[i - 1];
    value[i - 1] = tmp / b;
    carry = tmp % b;
  }
  normalize();

  return static_cast<uint32_t>(carry);
}

uint32_t big_integer::trial(big_integer const &b) {
  uint64_t dividend = (static_cast<uint64_t>(value.back()) << 32) |
      (static_cast<uint64_t>(value[size() - 2]));
  uint64_t divider = b.value.back();

  return static_cast<uint32_t>((dividend / divider) & UINT32_MAX);
}

bool big_integer::smaller(big_integer const &b, size_t m) {
  for (size_t i = 1; i <= value.size(); i++) {
    uint32_t x = value[size() - i];
    uint32_t y = (m - i < b.size() ? b[m - i] : 0);
    if (x != y) {
      return x < y;
    }
  }
  return false;
}

void big_integer::difference(big_integer const &b, size_t m) {
  int64_t borrow = 0;
  uint64_t start = size() - m;
  for (size_t i = 0; i < m; i++) {
    int64_t diff = static_cast<int64_t>(value[start + i]) - (i < b.size() ? b[i] : 0) - borrow;
    value[start + i] = (diff < 0 ? static_cast<uint32_t>(diff + 1 + UINT32_MAX) : diff);
    borrow = diff < 0;
  }
}

std::pair<big_integer, big_integer> big_integer::div_mod(big_integer const& b) {
  bool ans_sign = sign ^ b.sign;
  bool this_sign = sign;

  sign = b.sign;
  if ((!b.sign && b > *this) || (b.sign && b < *this)) {
    sign = this_sign;
    this->normalize();
    return {0, *this};
  }

  if (b.size() == 1) {
    big_integer sh = short_div(b[0]);
    sign = ans_sign;
    sh.sign = this_sign;

    this->normalize();
    sh.normalize();

    return {*this, sh};
  }

  uint32_t normalize_shift = (static_cast<uint64_t>(UINT32_MAX) + 1) / (static_cast<uint64_t>(b.value.back()) + 1);
  *this *= normalize_shift;
  big_integer divisor = b * normalize_shift;
  push_back(0);
  size_t m = divisor.size() + 1;
  big_integer ans, dq;
  ans.value.resize(size() - divisor.size());
  uint32_t qt = 0;

  for (size_t j = ans.size(); j != 0; j--) {
    qt = trial(divisor);
    dq = divisor * qt;

    while (smaller(dq, m)) {
      qt--;
      dq -= divisor;
    }
    ans[j - 1] = qt;
    difference(dq, m);
    pop_back();
  }

  ans.sign = ans_sign;
  ans.normalize();

  sign = this_sign;
  this->short_div(normalize_shift);
  this->normalize();

  return {ans, *this};
}

big_integer operator/(big_integer a, big_integer const& b) {
  return a.div_mod(b).first;
}

big_integer operator%(big_integer a, big_integer const& b) {
  return a.div_mod(b).second;
}

void big_integer::additional_code() {
  for (uint32_t & cur : value){
    cur = UINT32_MAX - cur;
  }

  sign = false;
  *this += 1;
}

big_integer big_integer::binary_operation(big_integer b, const std::function<uint32_t(uint32_t, uint32_t)>& func) {
  uint32_t new_sign = func(sign, b.sign);

  while (size() < b.size()) {
    push_back(0);
  }
  while (b.size() < size()) {
    b.push_back(0);
  }
  if (sign) {
    additional_code();
  }
  if (b.sign) {
    b.additional_code();
  }

  for (size_t i = 0; i < size(); i++) {
    value[i] = func(value[i], b.value[i]);
  }
  normalize();

  if (new_sign) {
    additional_code();
    normalize();
  }
  sign = new_sign;

  return *this;
}

big_integer operator&(big_integer a, big_integer const& b) {
  return a.binary_operation(b,[] (uint32_t a, uint32_t b) { return a & b; });
}
big_integer operator|(big_integer a, big_integer const& b) {
  return a.binary_operation(b,[] (uint32_t a, uint32_t b) { return a | b; });
}
big_integer operator^(big_integer a, big_integer const& b) {
  return a.binary_operation(b,[] (uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer operator<<(big_integer a, int b) {
  if (b < 0) {
    return a >> (-b);
  }

  a *= (static_cast<uint32_t>(1) << (b % 32));

  uint32_t tmp = b / 32;

  reverse(a.value.begin(), a.value.end());
  for (size_t i = 0; i < tmp; i++) {
    a.push_back(0);
  }
  reverse(a.value.begin(), a.value.end());

  a.normalize();
  return a;
}

big_integer operator>>(big_integer a, int b) {
  if (b < 0) {
    return a << (-b);
  }

  a /= (static_cast<uint32_t>(1) << (b % 32));

  uint32_t tmp = b / 32;

  reverse(a.value.begin(), a.value.end());
  for (size_t i = 0; i < tmp && !a.value.empty(); i++) {
    a.pop_back();
  }
  reverse(a.value.begin(), a.value.end());

  if (a.value.empty()) {
    a.push_back(0);
    a.sign = false;
  }

  a.normalize();
  return a.sign ? a - 1 : a;
}

int32_t big_integer::compare(big_integer const& b) const {
  if (sign != b.sign) {
    return sign ? -1 : 1;
  }

  if (size() > b.size()) {
    return sign ? -1 : 1;
  }
  if (size() < b.size()) {
    return sign ? 1 : -1;
  }

  for (size_t i = size(); i > 0; i--) {
    if (value[i - 1] > b[i - 1]) {
      return sign ? -1 : 1;
    }
    if (value[i - 1] < b[i - 1]) {
      return sign ? 1 : -1;
    }
  }

  return 0;
}

bool operator==(big_integer const &a, big_integer const &b) {
  return a.compare(b) == 0;
}

bool operator!=(big_integer const &a, big_integer const &b) {
  return a.compare(b) != 0;
}

bool operator<(big_integer const &a, big_integer const &b) {
  return a.compare(b) < 0;
}

bool operator>(big_integer const &a, big_integer const &b) {
  return a.compare(b) > 0;
}

bool operator<=(big_integer const &a, big_integer const &b) {
  return a.compare(b) <= 0;
}

bool operator>=(big_integer const &a, big_integer const &b) {
  return a.compare(b) >= 0;
}

std::string to_string(big_integer const &a) {
  if (a.size() == 1 && a[0] == 0) {
    return "0";
  }
  std::string s;
  big_integer tmp(a);

  while (tmp != 0) {
    s.push_back(static_cast<char> ((tmp % 10)[0] + static_cast<uint32_t>('0')));
    tmp /= 10;
  }
  if (a.sign) {
    s.push_back('-');
  }
  std::reverse(s.begin(), s.end());
  return s;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
  return s << to_string(a);
}

size_t big_integer::size() {
  return value.size();
}

uint32_t &big_integer::operator[](size_t i) {
  return value[i];
}

uint32_t const &big_integer::operator[](size_t i) const {
  return value[i];
}

size_t big_integer::size() const {
  return value.size();
}

void big_integer::pop_back() {
  value.pop_back();
}

void big_integer::push_back(uint32_t a) {
  value.push_back(a);
}

void big_integer::normalize() {
  while (size() > 1 && value.back() == 0) {
    value.pop_back();
  }

  if (size() == 1 && value[0] == 0) {
    sign = false;
  }
}
