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
  shared = new buffer(1, a < 0 ? static_cast<uint32_t>(-static_cast<uint64_t>(a)) :  static_cast<uint32_t>(a), 1);
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

big_integer operator+(big_integer const& a, big_integer const& b) {
  if (a.sign == b.sign) {
    big_integer ans = big_integer(a);
    ans.unshare();

    uint64_t tmp, carry = 0;

    size_t max_size = std::max(ans.size(), b.size());
    for (size_t i = 0; i < max_size; i++) {
      if (i == ans.size()) ans.data_push_back(0);
      tmp = carry + ans.get_elem(i) + (i < b.size() ? b.get_elem(i) : 0);
      carry = tmp > UINT32_MAX;
      ans.get_elem(i) = (uint32_t) (tmp & UINT32_MAX);
    }
    if (carry > 0) {
      ans.data_push_back(carry);
    }

    return ans;
  } else {
    return (a.sign ? b - (-a) : a - (-b));
  }
}

big_integer operator-(big_integer const& a, big_integer const& b) {
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
  big_integer ans = big_integer(a);
  ans.unshare();

  ans.sign = false;
  uint32_t carry = 0;
  int64_t tmp;
  for (size_t i = 0; i < ans.size(); i++) {
    tmp = (int64_t) ans.get_elem(i) - carry - (i < b.size() ? b.get_elem(i) : 0);
    carry = tmp < 0;
    ans.get_elem(i) = tmp < 0 ? static_cast<uint32_t>(tmp + 1 + UINT32_MAX) : tmp;
  }
  ans.remove_zero();

  return ans;
}

big_integer operator*(big_integer const& a, big_integer const& b){
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

void big_integer::short_div(big_integer& a, uint32_t b) {
  uint64_t carry = 0;
  for (size_t i = a.size(); i != 0; i--) {
    uint64_t tmp = (carry << 32) + a.get_elem(i - 1);
    a.get_elem(i - 1) = tmp / b;
    carry = tmp % b;
  }
  a.remove_zero();
}

uint32_t big_integer::trial(big_integer const& a, big_integer const& b) {
  __uint128_t dividend = ((static_cast<__uint128_t>(a.get_elem(a.size() - 1)) << 64) |
      (static_cast<__uint128_t>(a.get_elem(a.size() - 2)) << 32) |
      (static_cast<__uint128_t>(a.get_elem(a.size() - 3))));
  __uint128_t divider = ((static_cast<__uint128_t>(b.get_elem(b.size() - 1)) << 32) |
      (static_cast<__uint128_t>(b.get_elem(b.size() - 2))));

  return static_cast<uint32_t>((dividend / divider) & UINT32_MAX);
}

bool big_integer::smaller(big_integer const& a, big_integer const& b, size_t m) {
  for (size_t i = 1; i <= a.size(); i++) {
    uint32_t x = a.get_elem(a.size() - i);
    uint32_t y = (m - i < b.size() ? b.get_elem(m - i) : 0);
    if (x != y) {
      return x < y;
    }
  }
  return false;
}

void big_integer::difference(big_integer &a, big_integer const &b, size_t m) {
  int64_t borrow = 0;
  uint64_t start = a.size() - m;
  for (size_t i = 0; i < m; i++) {
    int64_t diff = static_cast<int64_t>(a.get_elem(start + i)) - (i < b.size() ? b.get_elem(i) : 0) - borrow;
    a.get_elem(start + i) = (diff < 0 ? static_cast<uint32_t>(diff + 1 + UINT32_MAX) : diff);
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
    big_integer::short_div(a, b.get_elem(0));
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
    qt = big_integer::trial(a, divider);
    dq = divider * qt;

    if (big_integer::smaller(a, dq, m)) {
      qt--;
      dq -= divider;
    }
    ans.get_elem(j - 1) = qt;

    big_integer::difference(a, dq, m);
    a.shared->data.pop_back();
  }

  ans.remove_zero();
  ans.sign = ans_sign;

  return ans;
}

big_integer operator%(big_integer const& a, big_integer const& b) {
  return a - (a / b) * b;
}

void big_integer::additional_code(big_integer &a) {
  for (size_t i = 0; i < a.size(); i++){
    a.get_elem(i) = UINT32_MAX - a.get_elem(i);
  }

  a.sign = false;
  a += 1;
}

big_integer big_integer::binary_operation(big_integer const& a, big_integer const& b, uint32_t (*func)(uint32_t, uint32_t)) {
  uint32_t new_sign = func(a.sign, b.sign);

  big_integer fir(a), sec(b);
  fir.unshare();
  sec.unshare();

  while (fir.size() < sec.size()) {
    fir.data_push_back(0);
  }
  while (sec.size() < fir.size()) {
    sec.data_push_back(0);
  }
  if (fir.sign) {
    additional_code(fir);
  }
  if (sec.sign) {
    additional_code(sec);
  }

  for (size_t i = 0; i < fir.size(); i++) {
    fir.get_elem(i) = func(fir.get_elem(i), sec.get_elem(i));
  }
  fir.remove_zero();

  if (new_sign) {
    additional_code(fir);
    fir.remove_zero();
  }
  fir.sign = new_sign;

  return fir;
}

big_integer operator&(big_integer const& a, big_integer const& b) {
  return big_integer::binary_operation((big_integer &) a, (big_integer &) b,
                                       [] (uint32_t a, uint32_t b) { return a & b; });
}
big_integer operator|(big_integer const& a, big_integer const& b) {
  return big_integer::binary_operation((big_integer &) a, (big_integer &) b,
                                       [] (uint32_t a, uint32_t b) { return a | b; });
}
big_integer operator^(big_integer const& a, big_integer const& b) {
  return big_integer::binary_operation((big_integer &) a, (big_integer &) b,
                                       [] (uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer operator<<(big_integer const& a, int b) {
  if (b < 0) {
    return a >> (-b);
  }

  big_integer ans(a);
  ans.unshare();

  ans *= (static_cast<uint32_t>(1) << (b % 32));

  uint32_t tmp = b / 32;

  ans.shared->data.reverse();
  for (size_t i = 0; i < tmp; i++) {
    ans.data_push_back(0);
  }
  ans.shared->data.reverse();

  return ans;
}
big_integer operator>>(big_integer const& a, int b) {
  if (b < 0) {
    return a << (-b);
  }
  big_integer ans(a);
  ans.unshare();

  ans /= (static_cast<uint32_t>(1) << (b % 32));

  uint32_t tmp = b / 32;

  ans.shared->data.reverse();
  for (size_t i = 0; i < tmp && ans != 0; i++) {
    ans.shared->data.pop_back();
  }
  ans.shared->data.reverse();

  if (ans == 0) {
    ans.data_push_back(0);
    ans.sign = false;
  }

  return ans.sign ? ans - 1 : ans;
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
    s.push_back((char) ((tmp % 10).get_elem(0) + '0'));
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
  shared = new buffer((size_t)1, shared->data);
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


