#pragma once
#include <memory>
#include <string>
#include <cstddef>
class AssetCache {
  struct Impl;
  std::unique_ptr < Impl > impl_;
  public : AssetCache();
  ~ AssetCache();
  // 불완전한 Impl의 삭제는 정의가 보이는 cpp에서 수행한다.
  AssetCache(AssetCache &&) noexcept;
  AssetCache & operator =(AssetCache &&) noexcept;
  AssetCache(const AssetCache &) = delete;
  AssetCache & operator =(const AssetCache &) = delete;
  bool Add(std::string key, int value);
  int Find(const std::string & key) const;
  std::size_t Size() const noexcept;
};
