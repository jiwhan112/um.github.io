#include "asset_cache.hpp"
#include <unordered_map>
#include <stdexcept>
#include <utility>
// 구현 파일만 컨테이너 정의를 포함하여 헤더 의존성을 제한한다.
struct AssetCache::Impl {
  std::unordered_map < std::string, int > entries;
};
AssetCache::AssetCache() : impl_(std::make_unique < Impl >()) {
}
AssetCache::~ AssetCache() = default;
AssetCache::AssetCache(AssetCache &&) noexcept = default;
AssetCache & AssetCache::operator =(AssetCache &&) noexcept = default;
bool AssetCache::Add(std::string key, int value) {
  // 도메인 오류를 먼저 검사하므로 기존 맵을 건드리지 않는다.
  if (key.empty() || value < 0) throw std::invalid_argument("invalid asset");
  if (! impl_) impl_ = std::make_unique < Impl >();
  // 이동 후 재사용 계약
  // 이미 있는 키는 값을 덮어쓰지 않는 삽입 정책이다.
  return impl_->entries.try_emplace(std::move(key), value).second;
}
int AssetCache::Find(const std::string & key) const {
  if (! impl_) return - 1;
  const auto it = impl_->entries.find(key);
  return it == impl_->entries.end() ? - 1 : it->second;
}
std::size_t AssetCache::Size() const noexcept {
  return impl_ ? impl_->entries.size() : 0;
}
