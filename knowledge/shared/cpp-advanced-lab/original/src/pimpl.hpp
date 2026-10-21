#ifndef STUDY_PIMPL_HPP
#define STUDY_PIMPL_HPP
#include <memory>
#include <string>
class Settings {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    explicit Settings(std::string name);
    ~Settings();
    Settings(Settings&&) noexcept;
    Settings& operator=(Settings&&) noexcept;
    Settings(const Settings&);
    Settings& operator=(const Settings&);
    void Rename(std::string name);
    std::string Name() const;
    bool Empty() const noexcept;
    friend void swap(Settings&, Settings&) noexcept;
};
#endif
