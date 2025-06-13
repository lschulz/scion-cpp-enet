// Copyright (c) 2024-2025 Lars-Christian Schulz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "scion/addr/isd_asn.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <format>
#include <ostream>
#include <system_error>
#include <variant>


namespace scion {

enum class HostAddrType : std::uint8_t
{
    IPv4 = 0x0,
    IPv6 = 0x3,
};

template <typename Host> struct AddressTraits
{
    static_assert(sizeof(Host) < 0, "AddressTraits is not specialized for this type");
};

/// \brief Global SCION address consisting of an ISD-ASN and a host address.
/// \tparam T Type of AS-local host address.
template <typename T>
class Address
{
public:
    /// \brief Type of AS-local host addresses.
    using HostAddr = T;

private:
    IsdAsn ia;
    HostAddr host;

public:
    Address() = default;
    Address(IsdAsn isdAsn, HostAddr addr)
        : ia(isdAsn), host(std::move(addr))
    {}

    IsdAsn getIsdAsn() const { return ia; }
    IsdAsn& getIsdAsn() { return ia; }

    HostAddr getHost() const { return host; };
    HostAddr& getHost() { return host; };

    std::strong_ordering operator<=>(const Address<T>&) const = default;

    /// \brief Returns true if this endpoint does not contain an unspecified
    /// ISD-ASN or IP.
    bool isFullySpecified() const
    {
        return !ia.isUnspecified()
            && !AddressTraits<HostAddr>::isUnspecified(getHost());
    }

    /// \brief Returns true if this address is equal to `other`. Also returns
    /// true if the ISD-ASM or host address part does not match `other` but
    /// is unspecified (a wildcard address) in this address.
    bool matches(const Address<T>& other) const
    {
        if (!ia.matches(other.ia)) return false;
        return AddressTraits<HostAddr>::isUnspecified(host) || host == other.host;
    }

    /// \brief Parse SCION and host address separated by a comma.
    static Maybe<Address<T>> Parse(std::string_view text)
    {
        auto comma = text.find(',');
        if (comma == text.npos)
            return Error(ErrorCode::SyntaxError);

        auto ia = IsdAsn::Parse(text.substr(0, comma));
        if (isError(ia)) return propagateError(ia);

        auto host = AddressTraits<T>::fromString(text.substr(comma + 1));
        if (isError(host)) return propagateError(host);

        return Address<T>(get(ia), get(host));
    }

    std::uint32_t checksum() const
    {
        return ia.checksum() + AddressTraits<HostAddr>::checksum(host);
    }

    std::size_t size() const { return ia.size() + AddressTraits<HostAddr>::size(host); }

    friend std::formatter<scion::Address<T>>;
};

} // namespace scion

template<typename T>
struct std::formatter<scion::Address<T>>
{
    constexpr auto parse(auto& ctx)
    {
        return ctx.begin();
    }

    auto format(const scion::Address<T>& addr, auto& ctx) const
    {
        return std::format_to(ctx.out(), "{},{}", addr.ia, addr.host);
    }
};

namespace scion {
template <typename T>
inline std::ostream& operator<<(std::ostream& stream, const Address<T>& addr)
{
    stream << std::format("{}", addr);
    return stream;
}
} // namespace scion

template <typename T>
struct std::hash<scion::Address<T>>
{
    std::size_t operator()(const scion::Address<T>& addr) const noexcept
    {
        std::size_t h = 0;
        h ^= hash<scion::IsdAsn>{}(addr.getIsdAsn());
        h ^= hash<T>{}(addr.getHost());
        return h;
    }
};
