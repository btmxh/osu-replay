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

  // Return Result<int> if func(value) return void
  template <typename Func>
  auto Map(Func func) -> Result<std::conditional_t<
      std::is_same_v<decltype(func(std::declval<T>())), void>, int,
      decltype(func(std::declval<T>()))>> {
    using RetType = decltype(func(value));
    if constexpr (std::is_same_v<RetType, void>) {
      if (init) {
        return Result<int>(0);
      } else {
        return Result<int>(error);
      }
    } else {
      if (init) {
        return Result<RetType>(func(value));
      } else {
        return Result<RetType>(error);
      }
    }
  }

  template <typename V>
  struct UnwrapResult {
    using Type = void;
  };

  template <typename V>
  struct UnwrapResult<Result<V>> {
    using Type = V;
  };

  template <typename Func,
            typename = std::enable_if_t<!std::is_same_v<
                UnwrapResult<decltype(std::declval<Func>()(std::declval<T>()))>,
                void>>>
  auto FlatMap(Func func) -> decltype(std::declval<Func>()(std::declval<T>())) {
    using RetType = decltype(std::declval<Func>()(std::declval<T>()));
    if (init) {
      return func(value);
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
