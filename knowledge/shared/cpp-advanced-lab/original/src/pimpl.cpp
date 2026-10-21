#include "pimpl.hpp"
#include <stdexcept>
#include <utility>
struct Settings::Impl { std::string name; };
Settings::Settings(std::string name) : impl_(std::make_unique<Impl>(Impl{std::move(name)})) {}
Settings::~Settings() = default;
Settings::Settings(Settings&&) noexcept = default;
Settings& Settings::operator=(Settings&&) noexcept = default;
Settings::Settings(const Settings& other) : impl_(other.impl_ ? std::make_unique<Impl>(*other.impl_) : nullptr) {}
Settings& Settings::operator=(const Settings& other) {
    if (this != &other) { Settings copy(other); swap(*this, copy); }
    return *this;
}
void Settings::Rename(std::string name) {
    auto candidate = std::make_unique<Impl>(Impl{std::move(name)});
    impl_.swap(candidate);
}
std::string Settings::Name() const {
    if (!impl_) throw std::logic_error("empty moved-from settings");
    return impl_->name;
}
bool Settings::Empty() const noexcept { return !impl_; }
void swap(Settings& a, Settings& b) noexcept { a.impl_.swap(b.impl_); }
