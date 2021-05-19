#pragma once

#include <cassert>
#include <system_error>
#include <utility>

namespace osrp {
template <typename T>
class Result {
 public:
  explicit Result(const T& value) : value{value}, init{true} {}
  explicit Result(T&& value) : value{std::move(value)}, init{true} {}
  explicit Result(std::error_code error) : error{error}, init{false} {}

  ~Result() { Destroy(); }
  Result(Result const& expected) : init(expected.init) {
    if (init)
      new (&value) T{expected.value};
    else
      error = expected.error;
  }
  Result(Result&& expected) : init(expected.init) {
    if (init)
      new (&value) T{std::move(expected.value)};
    else
      error = std::move(expected.error);
    expected.Destroy();
  }

  Result& operator=(const T& expect) {
    Destroy();
    init = true;
    new (&value) T{expect};
    return *this;
  }
  Result& operator=(T&& expect) {
    Destroy();
    init = true;
    new (&value) T{std::move(expect)};
    return *this;
  }
  Result& operator=(const std::error_code& err) {
    Destroy();
    init = false;
    error = err;
    return *this;
  }
  // clang-format off
	const T* operator-> () const { assert (init); return &value; }
	T*       operator-> ()       { assert (init); return &value; }
	const T& operator* () const& { assert (init);	return value; }
	T&       operator* () &      { assert (init); return value; }
	T&&      operator* () &&	 { assert (init); return std::move (value); }
	const T&  Value () const&    { assert (init); return value; }
	T&        Value () &         { assert (init); return value; }
	const T&& Value () const&&   { assert (init); return std::move (value); }
	T&&       Value () &&        { assert (init); return std::move (value); }

  // std::error_code associated with the error
  std::error_code Error() const { assert (!init); return error; }
  // clang-format on

  bool HasValue() const { return init; }
  explicit operator bool() const { return init; }

  template <typename Func>
  decltype(auto) Map(Func func) {
    using RetType = decltype(func(value));
    if (init) {
      return RetType(func(value));
    } else {
      return RetType(error);
    }
  }

 private:
  void Destroy() {
    if (init) value.~T();
  }
  union {
    T value;
    std::error_code error;
  };
  bool init;
};
}  // namespace osrp
