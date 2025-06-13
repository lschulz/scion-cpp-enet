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

#include "scion/bit_stream.hpp"
#include "scion/hdr/details.hpp"

#include <array>
#include <concepts>
#include <cstdint>
#include <format>


namespace scion {
namespace hdr {

enum class EtherType : std::uint16_t
{
    IPv4 = 0x0800,
    IPv6 = 0x86dd,
};

/// \brief Ethernet header.
class Ethernet
{
public:
    std::array<std::byte, 6> dst = {};
    std::array<std::byte, 6> src = {};
    EtherType type = EtherType::IPv4;

    std::size_t size() const { return 14; }

    template <typename Stream, typename Error>
    bool serialize(Stream& stream, Error& err)
    {
        if (!stream.serializeBytes(dst, err)) return err.propagate();
        if (!stream.serializeBytes(src, err)) return err.propagate();
        auto typeValue = static_cast<std::uint16_t>(type); // avoid strict aliasing rule violation
        if (!stream.serializeUint16(typeValue, err)) return err.propagate();
        type = static_cast<EtherType>(typeValue);
        return true;
    }

    auto print(auto out, int indent) const
    {
        using namespace details;
        out = std::format_to(out, "###[ Ethernet ]###\n");
        out = formatIndented(out, indent, "dst  = ");
        out = formatBytes(out, dst);
        out = std::format_to(out, "\n");
        out = formatIndented(out, indent, "src  = ");
        out = formatBytes(out, src);
        out = std::format_to(out, "\n");
        out = formatIndented(out, indent, "type = {:#04x}\n", (std::uint16_t)type);
        return out;
    }
};

} // namespace hdr
} // namespace scion
