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
  if (value.size() == 1 && value[0] == 0) {
    return *this;
  }

  big_integer tmp = big_integer(*this);
  tmp.sign = !tmp.sign;
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

    size_t max_size = std::max(a.value.size(), b.value.size());
    for (size_t i = 0; i < max_size; i++) {
      if (i == a.value.size()) a.value.push_back(0);
      tmp = carry + a.value[i] + (i < b.value.size() ? b.value[i] : 0);
      carry = tmp > UINT32_MAX;
      a.value[i] = static_cast<uint32_t>(tmp & UINT32_MAX);
    }
    if (carry > 0) {
      a.value.push_back(carry);
    }

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
  a.sign = false;
  uint32_t carry = 0;
  int64_t tmp;
  for (size_t i = 0; i < a.value.size(); i++) {
    tmp = (int64_t)a.value[i] - carry - (i < b.value.size() ? b.value[i] : 0);
    carry = tmp < 0;
    a.value[i] = tmp < 0 ? static_cast<uint32_t>(tmp + 1 + UINT32_MAX) : tmp;
  }
  a.remove_zero();

  return a;
}

big_integer operator*(big_integer const& a, big_integer const& b){
  if (a == 0 || b == 0) {
    return 0;
  }

  big_integer res;
  res.sign = a.sign ^ b.sign;
  res.value.resize(a.value.size() + b.value.size());
  for (size_t i = 0; i < a.value.size(); i++) {
    uint64_t carry = 0;
    for (size_t j = 0; j < b.value.size(); j++) {
      uint64_t tmp = static_cast<uint64_t>(a.value[i]) * b.value[j] + carry + res.value[i + j];
      res.value[i + j] = static_cast<uint32_t>(tmp & UINT32_MAX);
      carry = tmp >> 32;
    }
    res.value[i + b.value.size()] += carry;
  }
  res.remove_zero();

  return res;
}


void big_integer::short_div(uint32_t b) {
  uint64_t carry = 0;
  for (size_t i = value.size(); i != 0; i--) {
    uint64_t tmp = (carry << 32) + value[i - 1];
    value[i - 1] = tmp / b;
    carry = tmp % b;
  }
  remove_zero();
}

uint32_t big_integer::trial(big_integer const &b) {
  uint64_t dividend = (static_cast<uint64_t>(value.back()) << 32) |
      (static_cast<uint64_t>(value[value.size() - 2]));
  uint64_t divider = static_cast<uint64_t>(b.value.back());

  return static_cast<uint32_t>((dividend / divider) & UINT32_MAX);
}

bool big_integer::smaller(big_integer const &b, size_t m) {
  for (size_t i = 1; i <= value.size(); i++) {
    uint32_t x = value[value.size() - i];
    uint32_t y = (m - i < b.value.size() ? b.value[m - i] : 0);
    if (x != y) {
      return x < y;
    }
  }
  return false;
}

void big_integer::difference(big_integer const &b, size_t m) {
  int64_t borrow = 0;
  uint64_t start = value.size() - m;
  for (size_t i = 0; i < m; i++) {
    int64_t diff = static_cast<int64_t>(value[start + i]) - (i < b.value.size() ? b.value[i] : 0) - borrow;
    value[start + i] = (diff < 0 ? static_cast<uint32_t>(diff + 1 + UINT32_MAX) : diff);
    borrow = diff < 0;
  }
}

big_integer operator/(big_integer a, big_integer const& b) {
  bool ans_sign = a.sign ^ b.sign;
  a.sign = b.sign;
  if (!b.sign) {
    if (b > a) return 0;
  } else {
    if (b < a) return 0;
  }

  if (b.value.size() == 1) {
    a.short_div(b.value[0]);
    a.sign = ans_sign;
    return a;
  }

  uint32_t normalize = (static_cast<uint64_t>(UINT32_MAX) + 1) / (static_cast<uint64_t>(b.value.back()) + 1);
  a *= normalize;
  big_integer divisor = b * normalize;
  a.value.push_back(0);
  size_t m = divisor.value.size() + 1;
  big_integer ans, dq;
  ans.value.resize(a.value.size() - divisor.value.size());
  uint32_t qt = 0;

  for (size_t j = ans.value.size(); j != 0; j--) {
    qt = a.trial(divisor);
    dq = divisor * qt;

    while (a.smaller(dq, m)) {
      qt--;
      dq -= divisor;
    }
    ans.value[j - 1] = qt;
    a.difference(dq, m);
    a.value.pop_back();
  }

  ans.remove_zero();
  ans.sign = ans_sign;
  return ans;
}

big_integer operator%(big_integer const& a, big_integer const& b) {
  return a - (a / b) * b;
}

void big_integer::additional_code() {
  for (uint32_t & cur : value){
    cur = UINT32_MAX - cur;
  }

  sign = false;
  *this += 1;
}

big_integer big_integer::binary_operation(big_integer b, uint32_t (*func)(uint32_t, uint32_t)) {
  uint32_t new_sign = func(sign, b.sign);

  while (value.size() < b.value.size()) {
    value.push_back(0);
  }
  while (b.value.size() < value.size()) {
    b.value.push_back(0);
  }
  if (sign) {
    additional_code();
  }
  if (b.sign) {
    b.additional_code();
  }

  for (size_t i = 0; i < value.size(); i++) {
    value[i] = func(value[i], b.value[i]);
  }
  remove_zero();

  if (new_sign) {
    additional_code();
    remove_zero();
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
    a.value.push_back(0);
  }
  reverse(a.value.begin(), a.value.end());

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
    a.value.pop_back();
  }
  reverse(a.value.begin(), a.value.end());

  if (a.value.empty()) {
    a.value.push_back(0);
    a.sign = false;
  }

  return a.sign ? a - 1 : a;
}

bool operator==(big_integer const &a, big_integer const &b) {
  return (a.value.size() == b.value.size() && a.value.size() == 1 && a.value[0] == 0 && b.value[0] == 0) ||
      (a.sign == b.sign && a.value == b.value);
}

bool operator!=(big_integer const &a, big_integer const &b) {
  return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
  if (a.sign != b.sign) {
    return a.sign;
  }
  if (a.sign) {
    return (-a > -b);
  }

  if (a.value.size() != b.value.size()) {
    return a.value.size() < b.value.size();
  }

  for(size_t i = a.value.size(); i != 0; i--) {
    if (a.value[i - 1] > b.value[i - 1]) return false;
    if (a.value[i - 1] < b.value[i - 1]) return true;
  }
  return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
  return (!(a < b) && (a != b));
}

bool operator<=(big_integer const &a, big_integer const &b) {
  return ((a < b) || (a == b));
}

bool operator>=(big_integer const &a, big_integer const &b) {
  return ((a > b) || (a == b));
}

std::string to_string(big_integer const &a) {
  if (a.value.size() == 1 && a.value[0] == 0) {
    return "0";
  }
  std::string s;
  big_integer tmp(a);

  while (tmp != 0) {
    s.push_back(static_cast<char> ((tmp % 10).value[0] + static_cast<uint32_t>('0')));
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

void big_integer::remove_zero() {
  while (value.size() > 1 && value.back() == 0) {
    value.pop_back();
  }
}
