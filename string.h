#pragma once

#include <iostream>
#include <cstring>

class String {
public:
  String(size_t count, char symbol);

  String(const char* str);

  String();

  String(const String& str);

  ~String() { delete[] chars_; }

  size_t length() const { return len_; }

  size_t size() const { return len_; }

  size_t capacity() { return capacity_; }

  char* data() { return chars_; }

  const char* data() const { return chars_; }

  String& operator=(const String& str);

  char& operator[](size_t index) { return chars_[index]; }

  const char& operator[](size_t index) const { return chars_[index]; }

  String& operator+=(const String& other);

  String& operator+=(char symbol);

  void push_back(char symbol);

  void pop_back() { chars_[--len_] = '\0'; }

  char& front() { return chars_[0]; }

  char& back() { return chars_[len_ - 1]; }

  const char& front() const { return chars_[0]; }

  const char& back() const { return chars_[len_ - 1]; }

  size_t find(const String& substring) const;

  size_t rfind(const String& substring) const;

  String substr(size_t start, size_t count) const;

  bool empty() { return (len_ == 0); }

  void clear();

  void shrink_to_fit() { change_capacity(len_); }

private:
  void change_capacity(size_t _capacity);

  void swap(String& str);

  size_t len_ = 0;
  size_t capacity_ = 0;
  char* chars_ = nullptr;
};

String::String(size_t count, char symbol) : len_(count), capacity_(count), chars_(new char[count + 1]) {
  memset(chars_, symbol, count);
  chars_[count] = '\0';
}

String::String(const char* str) : len_(strlen(str)), capacity_(len_), chars_(new char[len_ + 1]) {
  memcpy(chars_, str, len_ + 1);
  chars_[len_] = '\0';
}

String::String() : len_(0), capacity_(0), chars_(new char[1]) { chars_[0] = '\0'; }

String::String(const String& str) : len_(str.len_), capacity_(str.capacity_), chars_(new char[capacity_ + 1]) {
  memcpy(chars_, str.chars_, len_ + 1);
}

void String::swap(String& str) {
  std::swap(str.len_, len_);
  std::swap(str.capacity_, capacity_);
  std::swap(str.chars_, chars_);
}

String& String::operator=(const String& str) {
  if (this == &str) {
    return *this;
  }
  String temp(str);
  swap(temp);
  return *this;
}

void String::push_back(char symbol) {
  if (capacity_ - len_ < 1) {
    change_capacity(2 * (capacity_ + 1));
  }
  chars_[len_++] = symbol;
  chars_[len_] = '\0';
}

String& String::operator+=(const String& other) {
  if (capacity_ - len_ < other.length()) {
    change_capacity((len_ + other.length()) * 2);
  }
  memcpy(chars_ + len_, other.data(), other.length() + 1);
  len_ += other.length();
  return *this;
}

String& String::operator+=(char symbol) {
  push_back(symbol);
  return *this;
}

size_t String::find(const String& substring) const {
  if (substring.length() > len_) {
    return len_;
  }
  for (size_t i = 0; i <= len_ - substring.length(); ++i) {
    for (size_t j = 0; j < substring.length(); ++j) {
      if ((*this)[i + j] != substring[j]) {
        break;
      } else if (j == substring.length() - 1) {
        return i;
      }
    }
  }
  return len_;
}

size_t String::rfind(const String& substring) const {
  if (substring.length() > len_) {
    return len_;
  }
  for (size_t i = len_ - substring.length() + 1; i >= 1; --i) {
    for (size_t j = 0; j < substring.length(); ++j) {
      if ((*this)[i + j - 1] != substring[j]) {
        break;
      } else if (j == substring.length() - 1) {
        return i - 1;
      }
    }
  }
  return len_;
}

String String::substr(size_t start, size_t count) const {
  String s(count, 'a');
  memcpy(s.data(), this->data() + start, count);
  return s;
}

void String::clear() {
  len_ = 0;
  chars_[0] = '\0';
}

void String::change_capacity(size_t _capacity) {
  char* temp = new char[_capacity + 1];
  size_t new_len = std::min(_capacity, this->len_);
  memcpy(temp, chars_, new_len + 1);
  delete[] chars_;
  chars_ = temp;
  len_ = new_len;
  this->capacity_ = _capacity;
}

bool operator==(const String& left, const String& right) {
  if (left.length() != right.length()) {
    return false;
  }
  int temp = memcmp(left.data(), right.data(), left.size());
  return temp == 0;
}

bool operator!=(const String& left, const String& right) { return !(left == right); }

bool operator<(const String& left, const String& right) {
  size_t min_len = std::min(left.length(), right.length());
  int temp = memcmp(left.data(), right.data(), min_len);
  if (temp < 0) return true;
  if (temp > 0) return false;
  if (left.length() >= right.length()) return false;
  return true;
}

bool operator>(const String& left, const String& right) { return (right < left); }

bool operator<=(const String& left, const String& right) { return !(right < left); }

bool operator>=(const String& left, const String& right) { return !(left < right); }

String operator+(const String& str, char symbol) {
  String s(str);
  s.push_back(symbol);
  return s;
}

String operator+(char symbol, const String& str) {
  String s;
  s.push_back(symbol);
  s += str;
  return s;
}

String operator+(const String& left, const String& right) {
  String s = left;
  s += right;
  return s;
}

std::ostream& operator<<(std::ostream& out, const String& str) {
  out << str.data();
  return out;
}

std::istream& operator>>(std::istream& in, String& str) {
  str.clear();
  char c;
  while (!in.eof() && in.get(c)) {
    if (std::isspace(c)) {
      break;
    }
    str.push_back(c);
  }
  return in;
}