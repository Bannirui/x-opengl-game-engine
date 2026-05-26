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
    UUID& operator=(const UUID&) = default;

    operator uint64_t() const { return m_UUID; }

private:
    uint64_t m_UUID;
};

namespace std
{
    template <>
    struct hash<UUID>
    {
        std::size_t operator()(const UUID& uuid) const { return (uint64_t)uuid; }
    };
}  // namespace std