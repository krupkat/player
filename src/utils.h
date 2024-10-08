#pragma once

#include <functional>
#include <optional>

namespace player::utils {
class DestructorCallback {
 public:
  template <typename TFunc>
  explicit DestructorCallback(TFunc func) : callback(func) {}
  ~DestructorCallback() {
    if (callback) {
      (*callback)();
    }
  };

  DestructorCallback(const DestructorCallback& other) = delete;
  DestructorCallback& operator=(const DestructorCallback&) = delete;
  DestructorCallback(DestructorCallback&& other) { *this = std::move(other); };
  DestructorCallback& operator=(DestructorCallback&& other) {
    if (this != &other) {
      std::swap(this->callback, other.callback);
    }
    return *this;
  };

 private:
  std::optional<std::function<void()>> callback;
};
}  // namespace player::utils
