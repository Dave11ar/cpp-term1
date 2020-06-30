//
// Created by dave11ar on 28.04.2020.
//

#include "big_integer.h"
#include <algorithm>

big_integer::big_integer() : sign(false) {
  shared = new buffer(1, small_vector<uint32_t>(0, 1));
}

big_integer::big_integer(uint32_t val, size_t n) : sign(false) {
  shared = new buffer(1, small_vector<uint32_t>(val, n));
}

big_integer::big_integer(big_integer const &a) : sign(a.sign) {
  shared = a.shared;
  shared->ref_counter++;
}

big_integer::big_integer(int a) : sign(a < 0) {
  shared = new buffer(1, static_cast<uint32_t>(a < 0 ? -static_cast<uint64_t>(a) :  a), 1);
}

big_integer::big_integer(uint32_t a) : sign(false) {
  shared = new buffer(1, a, 1);
}

big_integer::big_integer(std::string const &str) : big_integer() {
  if (str.empty() || str == "0" || str == "-0") {
    return;
  }

  for (size_t digit = (str[0] == '-' || str[0] == '+'); digit < str.length(); digit++) {
    *this *= 10;
    *this += (str[digit] - '0');
  }
  if (str[0] == '-') {
    sign = true;
  }
}

big_integer::~big_integer() {
  shared->ref_counter--;

  if (shared->ref_counter == 0) {
    delete shared;
  }
}

big_integer& big_integer::operator=(big_integer const &a) {
  if (a == *this) {
    return *this;
  }

  if (shared->ref_counter == 1) {
    delete shared;
  } else {
    shared->ref_counter--;
  }
  sign = a.sign;
  shared = a.shared;
  shared->ref_counter++;

  return *this;
}

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
  return *this = (*this / a);
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
  if (size() == 1 && !get_elem(0)) {
    return *this;
  }

  big_integer tmp(*this);
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
    a.unshare();

    uint64_t tmp, carry = 0;

    size_t max_size = std::max(a.size(), b.size());
    for (size_t i = 0; i < max_size; i++) {
      if (i == a.size()) a.data_push_back(0);
      tmp = carry + a.get_elem(i) + (i < b.size() ? b.get_elem(i) : 0);
      carry = tmp > UINT32_MAX;
      a.get_elem(i) = (uint32_t) (tmp & UINT32_MAX);
    }
    if (carry > 0) {
      a.data_push_back(carry);
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
  a.unshare();

  a.sign = false;
  uint32_t carry = 0;
  int64_t tmp;
  for (size_t i = 0; i < a.size(); i++) {
    tmp = (int64_t) a.get_elem(i) - carry - (i < b.size() ? b.get_elem(i) : 0);
    carry = tmp < 0;
    a.get_elem(i) = tmp < 0 ? static_cast<uint32_t>(tmp + 1 + UINT32_MAX) : tmp;
  }
  a.remove_zero();

  return a;
}

big_integer operator*(big_integer a, big_integer const& b){
  if (a == 0 || b == 0) {
    return 0;
  }

  big_integer res(0, a.size() + b.size());
  res.sign = a.sign ^ b.sign;
  for (size_t i = 0; i < a.size(); i++) {
    uint64_t carry = 0;
    for (size_t j = 0; j < b.size(); j++) {
      uint64_t tmp = (uint64_t) a.get_elem(i) * b.get_elem(j) + carry + res.get_elem(i + j);
      res.get_elem(i + j) = static_cast<uint32_t>(tmp & UINT32_MAX);
      carry = tmp >> 32;
    }
    res.get_elem(i + b.size()) += carry;
  }
  res.remove_zero();
  return res;
}

void big_integer::short_div(uint32_t b) {
  uint64_t carry = 0;
  for (size_t i = size(); i != 0; i--) {
    uint64_t tmp = (carry << 32) + get_elem(i - 1);
    get_elem(i - 1) = tmp / b;
    carry = tmp % b;
  }
  remove_zero();
}
uint32_t big_integer::trial(big_integer const &b) {
  uint64_t dividend = (static_cast<uint64_t>(get_elem(size() - 1)) << 32) |
      (static_cast<uint64_t>(get_elem(size() - 2)));
  uint64_t divider = static_cast<uint64_t>(b.get_elem(b.size() - 1));

  return static_cast<uint32_t>((dividend / divider) & UINT32_MAX);
}

bool big_integer::smaller(big_integer const& b, size_t m) {
  for (size_t i = 1; i <= size(); i++) {
    uint32_t x = get_elem(size() - i);
    uint32_t y = (m - i < b.size() ? b.get_elem(m - i) : 0);
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
    int64_t diff = static_cast<int64_t>(get_elem(start + i)) - (i < b.size() ? b.get_elem(i) : 0) - borrow;
    get_elem(start + i) = (diff < 0 ? static_cast<uint32_t>(diff + 1 + UINT32_MAX) : diff);
    borrow = diff < 0;
  }
}

big_integer operator/(big_integer a, big_integer const& b) {
  bool ans_sign = a.sign ^ b.sign;

  a.unshare();
  a.sign = b.sign;
  if (!b.sign) {
    if (b > a) {
      return 0;
    }
  } else {
    if (b < a)  {
      return 0;
    }
  }

  if (b.size() == 1) {
    a.short_div(b.get_elem(0));
    a.sign = ans_sign;
    return a;
  }

  uint32_t normalize = (static_cast<uint64_t>(UINT32_MAX) + 1) / (static_cast<uint64_t>(b.shared->data.back()) + 1);
  a *= normalize;
  big_integer divider = b * normalize;
  a.data_push_back(0);
  size_t m = divider.size() + 1;
  big_integer ans(0, a.size() - divider.size()), dq;
  uint32_t qt = 0;

  for (size_t j = ans.size(); j != 0; j--) {
    qt = a.trial(divider);
    dq = divider * qt;

    while (a.smaller(dq, m)) {
      qt--;
      dq -= divider;
    }
    ans.get_elem(j - 1) = qt;

    a.difference(dq, m);
    a.shared->data.pop_back();
  }

  ans.remove_zero();
  ans.sign = ans_sign;

  return ans;
}

big_integer operator%(big_integer a, big_integer const& b) {
  return a - (a / b) * b;
}

void big_integer::additional_code() {
  for (size_t i = 0; i < size(); i++){
    get_elem(i) = UINT32_MAX - get_elem(i);
  }

  sign = false;
  *this += 1;
}

big_integer big_integer::binary_operation(big_integer const& b, uint32_t (*func)(uint32_t, uint32_t)) {
  uint32_t new_sign = func(sign, b.sign);

  big_integer fir(*this), sec(b);
  fir.unshare();
  sec.unshare();

  while (fir.size() < sec.size()) {
    fir.data_push_back(0);
  }
  while (sec.size() < fir.size()) {
    sec.data_push_back(0);
  }
  if (fir.sign) {
    fir.additional_code();
  }
  if (sec.sign) {
    sec.additional_code();
  }

  for (size_t i = 0; i < fir.size(); i++) {
    fir.get_elem(i) = func(fir.get_elem(i), sec.get_elem(i));
  }
  fir.remove_zero();

  if (new_sign) {
    fir.additional_code();
    fir.remove_zero();
  }
  fir.sign = new_sign;

  return fir;
}

big_integer operator&(big_integer a, big_integer const& b) {
  return a.binary_operation(b, [] (uint32_t a, uint32_t b) { return a & b; });
}
big_integer operator|(big_integer a, big_integer const& b) {
  return a.binary_operation(b, [] (uint32_t a, uint32_t b) { return a | b; });
}
big_integer operator^(big_integer a, big_integer const& b) {
  return a.binary_operation(b,[] (uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer operator<<(big_integer a, int b) {
  if (b < 0) {
    return a >> (-b);
  }

  a.unshare();

  a *= (static_cast<uint32_t>(1) << (b % 32));

  uint32_t tmp = b / 32;

  a.shared->data.reverse();
  for (size_t i = 0; i < tmp; i++) {
    a.data_push_back(0);
  }
  a.shared->data.reverse();

  return a;
}
big_integer operator>>(big_integer a, int b) {
  if (b < 0) {
    return a << (-b);
  }

  a.unshare();

  a /= (static_cast<uint32_t>(1) << (b % 32));

  uint32_t tmp = b / 32;

  a.shared->data.reverse();
  for (size_t i = 0; i < tmp && a != 0; i++) {
    a.shared->data.pop_back();
  }
  a.shared->data.reverse();

  if (a == 0) {
    a.data_push_back(0);
    a.sign = false;
  }

  return a.sign ? a - 1 : a;
}

bool operator==(big_integer const &a, big_integer const &b) {
  return (a.size() == b.size() && a.size() == 1 &&
      a.get_elem(0) == 0 && b.get_elem(0) == 0) ||
      (a.sign == b.sign && a.shared->data == b.shared->data);
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

  if (a.size() != b.size()) {
    return a.size() < b.size();
  }

  for(size_t i = a.size(); i != 0; i--) {
    if (a.get_elem(i - 1) > b.get_elem(i - 1)) {
      return false;
    }
    if (a.get_elem(i - 1) < b.get_elem(i - 1)) {
      return true;
    }
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

std::string to_string(big_integer const& a) {
  if (a.size() == 1 && a.get_elem(0) == 0) {
    return "0";
  }
  std::string s;
  big_integer tmp(a);
  tmp.unshare();

  while (tmp != 0) {
    s.push_back(static_cast<char> ((tmp % 10).get_elem(0) + static_cast<uint32_t>('0')));
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
  while (size() > 1 && shared->data.back() == 0) {
    shared->data.pop_back();
  }
}

void big_integer::unshare() {
  if (shared->ref_counter == 1) {
    return;
  }

  shared->ref_counter--;
  shared = new buffer(1, shared->data);
}

size_t big_integer::size() const {
  return shared->data.size();
}

void big_integer::data_push_back(uint32_t x) {
  shared->data.push_back(x);
}

uint32_t& big_integer::get_elem(size_t i) const {
  return shared->data[i];
}


