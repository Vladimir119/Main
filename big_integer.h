#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

class BigInteger;

BigInteger operator*(const BigInteger& first, const BigInteger& second);

BigInteger operator-(const BigInteger& first, const BigInteger& second);

BigInteger operator/(const BigInteger& first, const BigInteger& second);

BigInteger operator+(const BigInteger& first, const BigInteger& second);

bool operator==(const BigInteger& first, const BigInteger& second);

bool operator!=(const BigInteger& first, const BigInteger& second);

bool operator<(const BigInteger& first, const BigInteger& second);

bool operator>(const BigInteger& first, const BigInteger& second);

bool operator>=(const BigInteger& first, const BigInteger& second);

bool operator<=(const BigInteger& first, const BigInteger& second);

class BigInteger {
public:
  BigInteger(long long number);

  BigInteger() : digits_(std::vector<long long>(1, 0)) {}

  BigInteger(const BigInteger& other);

  BigInteger(size_t size, int element) : digits_(std::vector<long long>(size, element)) {}

  BigInteger(const std::string& str);

  explicit operator bool() { return !(digits_.size() == 1 && digits_[0] == 0); }

  BigInteger& operator=(const BigInteger& other);

  BigInteger& operator+=(const BigInteger& other);

  BigInteger& operator-=(const BigInteger& other);

  BigInteger& operator*=(const BigInteger& other);

  BigInteger& operator--();

  BigInteger operator--(int);

  BigInteger operator++(int);

  BigInteger& operator++();

  BigInteger& operator/=(BigInteger divider);

  BigInteger& operator%=(const BigInteger& bi);

  BigInteger operator-();

  friend bool operator==(const BigInteger& first, const BigInteger& second);

  friend bool operator<(const BigInteger& first, const BigInteger& second);

  friend std::istream& operator>>(std::istream& in, BigInteger& bi);

  std::string toString() const;

  void delete_leading_zeros();

  const BigInteger abs() const;

  bool is_negative() const { return is_negative_; }

private:
  const long long kBase_ = 1'000'000'000;
  const size_t kCount_cell_ = 9;

  void invert() { is_negative_ = !is_negative_; }

  void sum_absolut_value(const BigInteger& other);

  void difference_absolut_value(const BigInteger& other);

  std::vector<long long> digits_;
  bool is_negative_ = false;
};

bool operator==(const BigInteger& first, const BigInteger& second) {
  if (first.digits_.size() != second.digits_.size() || (first.is_negative_ != second.is_negative_)) {
    return false;
  }
  for (size_t i = 0; i < first.digits_.size(); ++i) {
    if (first.digits_[i] != second.digits_[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const BigInteger& first, const BigInteger& second) { return !(first == second); }

bool operator<(const BigInteger& first, const BigInteger& second) {
  if (!first.is_negative_ && second.is_negative_) {
    return false;
  }
  if (first.is_negative_ && !second.is_negative_) {
    return true;
  }
  if (!first.is_negative_ && !second.is_negative_) {
    if (first.digits_.size() != second.digits_.size()) {
      return first.digits_.size() < second.digits_.size();
    }
    for (size_t i = first.digits_.size(); i >= 1; --i) {
      if (first.digits_[i - 1] != second.digits_[i - 1]) {
        return first.digits_[i - 1] < second.digits_[i - 1];
      }
    }
    return false;
  }
  if (first.digits_.size() != second.digits_.size()) {
    return first.digits_.size() > second.digits_.size();
  }
  for (size_t i = first.digits_.size(); i >= 1; --i) {
    if (first.digits_[i - 1] != second.digits_[i - 1]) {
      return first.digits_[i - 1] > second.digits_[i - 1];
    }
  }
  return false;
}

bool operator>(const BigInteger& first, const BigInteger& second) { return second < first; }

bool operator<=(const BigInteger& first, const BigInteger& second) { return !(first > second); }

bool operator>=(const BigInteger& first, const BigInteger& second) { return !(first < second); }

BigInteger operator+(const BigInteger& first, const BigInteger& second) {
  BigInteger temp = first;
  temp += second;
  return temp;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
  BigInteger temp = first;
  temp -= second;
  return temp;
}

BigInteger operator*(const BigInteger& first, const BigInteger& second) {
  BigInteger temp = first;
  temp *= second;
  return temp;
}

BigInteger operator/(const BigInteger& first, const BigInteger& second) {
  BigInteger temp(first);
  temp /= second;
  return temp;
}

BigInteger operator%(const BigInteger& first, const BigInteger& second) {
  BigInteger temp(first);
  temp %= second;
  return temp;
}

BigInteger operator "" _bi(unsigned long long num) { return BigInteger(num); }

BigInteger operator "" _bi(const char* num, size_t) { return BigInteger(num); }

std::ostream& operator<<(std::ostream& out, const BigInteger& bigInteger) {
  out << bigInteger.toString();
  return out;
}

std::istream& operator>>(std::istream& in, BigInteger& bigInteger) {
  std::string str;
  str.clear();
  in >> str;
  bigInteger = BigInteger(str);
  return in;
}

BigInteger GCD(BigInteger numerator, BigInteger denominator) {
  while (numerator != denominator && numerator != 0 && denominator != 0) {
    if (numerator > denominator) {
      numerator %= denominator;
    } else {
      denominator %= numerator;
    }
  }
  return std::max(numerator, denominator);
}

class Rational {
public:
  Rational() : numerator(0), denominator(1) {}

  Rational(const BigInteger& bi) : numerator(bi), denominator(1) {}

  Rational(const BigInteger& bi1, const BigInteger& bi2) : numerator(bi1), denominator(bi2) { reduce_fraction(); }

  Rational(int num) : numerator(num), denominator(1) {}

  Rational(const Rational& rational) : numerator(rational.numerator), denominator(rational.denominator) {}

  Rational& operator=(const Rational& rational);

  void reduce_fraction();

  Rational& operator+=(const Rational& rational);

  Rational& operator-=(const Rational& rational);

  Rational& operator*=(const Rational& rational);

  Rational& operator/=(const Rational& rational);

  friend bool operator<(const Rational& rational1, const Rational& rational2);

  friend bool operator==(const Rational& rational1, const Rational& rational2);

  std::string toString() const;

  std::string asDecimal(size_t precision) const;

  explicit operator bool() { return numerator != 0; }

  explicit operator double() {
    std::string temp = asDecimal(15);
    double this_double = std::stod(temp);
    return this_double;
  }

private:
  BigInteger numerator;
  BigInteger denominator;
};

Rational operator+(const Rational& rational1, const Rational& rational2) {
  Rational temp(rational1);
  temp += rational2;
  return temp;
}

Rational operator-(const Rational& rational1, const Rational& rational2) {
  Rational temp(rational1);
  temp -= rational2;
  return temp;
}

Rational operator*(const Rational& rational1, const Rational& rational2) {
  Rational temp(rational1);
  temp *= rational2;
  return temp;
}

Rational operator/(const Rational& rational1, const Rational& rational2) {
  Rational temp(rational1);
  temp /= rational2;
  return temp;
}

Rational operator-(const Rational& rational) { return rational * -1; }

bool operator<(const Rational& rational1, const Rational& rational2) {
  if (!rational1.numerator.is_negative() && rational2.numerator.is_negative()) {
    return false;
  }
  if (rational1.numerator.is_negative() && !rational2.numerator.is_negative()) {
    return true;
  }
  Rational temp = rational1 / rational2;

  if (!rational1.numerator.is_negative() && !rational2.numerator.is_negative()) {
    return temp.numerator < temp.denominator;
  }
  return temp.numerator > temp.denominator;
}

bool operator>(const Rational& rational1, const Rational& rational2) { return rational2 < rational1; }

bool operator<=(const Rational& rational1, const Rational& rational2) { return !(rational1 > rational2); }

bool operator>=(const Rational& rational1, const Rational& rational2) { return !(rational1 < rational2); }

bool operator==(const Rational& rational1, const Rational& rational2) {
  return rational1.numerator == rational2.numerator && rational1.denominator == rational2.denominator;
}

bool operator!=(const Rational& rational1, const Rational& rational2) { return !(rational1 == rational2); }

BigInteger::BigInteger(long long number) {
  if (number < 0) {
    number *= -1;
    is_negative_ = true;
  }
  while (number != 0) {
    digits_.push_back(number % kBase_);
    number /= kBase_;
  }
  if (digits_.size() == 0) {
    digits_.push_back(0);
  }
}

BigInteger::BigInteger(const BigInteger& other) {
  is_negative_ = other.is_negative_;
  digits_ = other.digits_;
}

BigInteger::BigInteger(const std::string& str) {
  std::string s = str;
  std::reverse(s.begin(), s.end());
  is_negative_ = s[s.size() - 1] == '-';
  if (is_negative_) {
    s.pop_back();
  }
  digits_.push_back(0);
  long long current_pow = 1;
  for (size_t i = 0; i < s.size(); ++i) {
    digits_.back() += (s[i] - '0') * current_pow;
    current_pow *= 10;
    if (current_pow == kBase_) {
      digits_.push_back(0);
      current_pow = 1;
    }
  }
  delete_leading_zeros();
}

BigInteger& BigInteger::operator=(const BigInteger& other) {
  is_negative_ = other.is_negative_;
  digits_ = other.digits_;
  return *this;
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
  if (is_negative_ == other.is_negative_) {
    sum_absolut_value(other);
    is_negative_ = other.is_negative_;
  } else if (is_negative_ && !other.is_negative_) {
    invert();
    if (*this <= other) {
      BigInteger temp = other;
      temp.difference_absolut_value(*this);
      *this = temp;
      is_negative_ = false;
    } else {
      difference_absolut_value(other);
      is_negative_ = true;
    }
  } else if (!is_negative_ && other.is_negative_) {
    invert();
    if (*this > other) {
      BigInteger temp = other;
      temp.difference_absolut_value(*this);
      *this = temp;
      is_negative_ = false;
    } else {
      difference_absolut_value(other);
      is_negative_ = true;
    }
    invert();
  }
  delete_leading_zeros();
  return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& other) {
  invert();
  *this += other;
  invert();
  return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& other) {
  if (other.digits_.size() == 1 && other.digits_[0] == 1 && other.is_negative_) {
    invert();
    return *this;
  }

  bool isNegative_temp = (is_negative_ ^ other.is_negative_);
  BigInteger result(digits_.size() + other.digits_.size(), 0);

  for (size_t i = 0; i < other.digits_.size(); ++i) {
    for (size_t j = 0; j < digits_.size(); j++) {
      long long multiply = digits_[j] * other.digits_[i];
      result.digits_[i + j] += multiply % kBase_;
      result.digits_[i + j + 1] += ((multiply / kBase_) + (result.digits_[i + j] / kBase_));
      result.digits_[i + j] %= kBase_;
    }
  }
  *this = result;
  is_negative_ = isNegative_temp;
  delete_leading_zeros();
  return *this;
}

BigInteger& BigInteger::operator/=(BigInteger divider) {
  bool isBiNegative = divider.is_negative_, isThisNegative = is_negative_;
  if (isBiNegative) {
    divider.invert();
  }
  if (isThisNegative) {
    invert();
  }
  if (divider > *this) {
    (*this) = 0;
  } else if (divider == *this) {
    *this = 1;
  } else if (divider != 1) {
    BigInteger current = 0;
    std::vector<long long> result;
    for (int i = static_cast<int>(digits_.size()) - 1; i >= 0; --i) {
      current *= kBase_;
      current += digits_[i];
      if (current >= divider) {
        int l = 0, r = kBase_;
        while (r - l > 1) {
          int m = (r + l) / 2;
          if (m * divider <= current) l = m;
          else r = m;
        }
        current -= l * divider;
        result.push_back(l);
      } else result.push_back(0);
    }
    std::reverse(result.begin(), result.end());
    digits_ = result;
    delete_leading_zeros();
  }
  is_negative_ = isThisNegative ^ isBiNegative;
  if (digits_.size() == 1 && digits_[0] == 0) {
    is_negative_ = false;
  }
  return *this;
}

BigInteger BigInteger::operator-() {
  BigInteger temp = *this;
  if (temp.digits_.size() != 1 || temp.digits_[0] != 0) {
    temp.invert();
  }
  return temp;
}

std::string BigInteger::toString() const {
  std::string s;
  std::string::size_type new_capacity = static_cast<std::string::size_type>(digits_.size() * kCount_cell_ + 1);
  s.reserve(new_capacity);
  if (is_negative_) {
    s.push_back('-');
  }
  for (size_t i = digits_.size(); i >= 1; --i) {
    if (i != digits_.size()) {
      long long temp = digits_[i - 1];
      int temp_count = 0;
      while (temp != 0) {
        temp /= 10;
        ++temp_count;
      }
      for (size_t j = temp_count; j < kCount_cell_; ++j) {
        s.push_back('0');
      }
      if (digits_[i - 1] == 0) {
        s.pop_back();
      }
    }
    s += std::to_string(digits_[i - 1]);
  }
  return s;
}

void BigInteger::delete_leading_zeros() {
  size_t index = digits_.size();
  while (index > 1 && digits_[index - 1] == 0) {
    digits_.pop_back();
    --index;
  }
  if (digits_[0] == 0 && digits_.size() == 1) {
    is_negative_ = false;
  }
}

const BigInteger BigInteger::abs() const {
  BigInteger temp = *this;
  if (temp < 0) {
    temp *= -1;
  }
  return temp;
}

void BigInteger::sum_absolut_value(const BigInteger& other) {
  size_t max_size = std::max(digits_.size(), other.digits_.size()) + 1;
  digits_.resize(max_size);
  long long remained = 0;
  for (size_t i = 0; i < max_size; ++i) {
    if (digits_.size() > i) {
      remained += digits_[i];
    }
    if (other.digits_.size() > i) {
      remained += other.digits_[i];
    }
    digits_[i] = remained % kBase_;
    remained /= kBase_;
  }
  delete_leading_zeros();
}

void BigInteger::difference_absolut_value(const BigInteger& other) {
  bool isTransfer = false;
  for (size_t i = 0; i < digits_.size(); ++i) {
    long long temp_num = isTransfer ? -1 : 0;
    isTransfer = false;

    temp_num += digits_[i];

    if (other.digits_.size() > i) {
      temp_num -= other.digits_[i];
    }

    if (temp_num < 0) {
      isTransfer = true;
      temp_num += kBase_;
    }
    digits_[i] = temp_num;
  }
  delete_leading_zeros();
}

BigInteger& BigInteger::operator--() {
  *this -= 1;
  return *this;
}

BigInteger BigInteger::operator--(int) {
  BigInteger copy(*this);
  *this -= 1;
  return copy;
}

BigInteger BigInteger::operator++(int) {
  BigInteger copy(*this);
  *this += 1;
  return copy;
}

BigInteger& BigInteger::operator++() {
  *this += 1;
  return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& bi) {
  *this -= ((*this / bi) * bi);
  delete_leading_zeros();
  return *this;
}

Rational& Rational::operator=(const Rational& rational) {
  Rational temp(rational);
  numerator = temp.numerator;
  denominator = temp.denominator;
  return *this;
}

void Rational::reduce_fraction() {
  BigInteger temp = GCD(numerator.abs(), denominator);
  numerator /= temp;
  denominator /= temp;
}

Rational& Rational::operator+=(const Rational& rational) {
  numerator = numerator * rational.denominator + denominator * rational.numerator;
  denominator *= rational.denominator;
  reduce_fraction();
  return *this;
}

Rational& Rational::operator-=(const Rational& rational) {
  numerator = numerator * rational.denominator - denominator * rational.numerator;
  denominator *= rational.denominator;
  reduce_fraction();
  return *this;
}

Rational& Rational::operator*=(const Rational& rational) {
  numerator *= rational.numerator;
  denominator *= rational.denominator;
  reduce_fraction();
  return *this;
}

Rational& Rational::operator/=(const Rational& rational) {
  numerator *= rational.denominator;
  if (rational.numerator < 0) {
    numerator *= -1;
  }
  denominator *= rational.numerator.abs();
  reduce_fraction();
  return *this;
}

std::string Rational::toString() const {
  if (denominator == 1) {
    return numerator.toString();
  }
  return numerator.toString() + '/' + denominator.toString();
}

std::string Rational::asDecimal(size_t precision) const {
  BigInteger temp(numerator);
  for (size_t i = 0; i < precision; ++i) {
    temp *= 10;
  }
  temp /= denominator;
  std::string result = temp.toString();
  std::string cur;
  if (precision >= result.size() || (precision + 1 >= result.size() && result[0] == '-')) {
    if (result[0] == '-') {
      cur = '-';
    }
    cur += "0.";
    size_t it;
    if (result[0] != '-') {
      it = precision - result.size();
    } else {
      it = precision + 1 - result.size();
    }
    for (size_t i = 0; i < it; ++i) {
      cur += '0';
    }
    if (result[0] == '-') {
      for (size_t j = 1; j < result.size(); ++j) {
        cur += result[j];
      }
    } else {
      cur += result;
    }
    result = cur;
  } else if (precision != 0) {
    result.insert(result.end() - precision, '.');
  }
  return result;
}
