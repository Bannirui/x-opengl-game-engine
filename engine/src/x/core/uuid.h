//
// Created by rui ding on 2026/3/10.
//

#pragma once

class UUID
{
public:
    UUID();
    UUID(uint64_t uuid);
    UUID(const UUID&) = default;

    // 类型转换
    operator uint64_t() const { return m_UUID; }

private:
    uint64_t m_UUID;
};

namespace std
{
    template <typename T>
    struct hash;

    // 向std命名空间增加hash的特化 让std容器调用
    template <>
    struct hash<UUID>
    {
        std::size_t operator()(const UUID& uuid) const { return (uint64_t)uuid; }
    };
}  // namespace std