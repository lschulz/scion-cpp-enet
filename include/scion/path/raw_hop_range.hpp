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
#include "scion/hdr/scion.hpp"

#include <cstdint>
#include <iterator>
#include <ranges>


namespace scion {

class RawHopSentinel {};

// Generator parsing the interface IDs from hop fields in a raw path.
class RawHopIter
{
public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<std::uint16_t, std::uint16_t>;

    value_type operator*() const
    {
        return std::make_pair(prevHop.second, curHop.first);
    }

    void operator++(int)
    {
        hdr::HopField hf;
        int steps = 1;
        if (segChange & (1ull << index)) steps = 2;
        for (int i = 0; i < steps; ++i) {
            if (!hf.serialize(rs, NullStreamError)) {
                stopIteration = true;
                return;
            }
            prevHop = curHop;
            if (hopDir & (1ull << index))
                curHop = std::make_pair(hf.consIngress, hf.consEgress);
            else
                curHop = std::make_pair(hf.consEgress, hf.consIngress);
            index++;
        }
    }

    RawHopIter& operator++()
    {
        (*this)++;
        return *this;
    }

    friend bool operator==(const RawHopIter& iter, RawHopSentinel)
    {
        return iter.stopIteration;
    }

private:
    RawHopIter(std::span<const std::byte> path)
        : rs(path)
    {
        hdr::PathMeta meta;
        hdr::InfoField info;
        if (!meta.serialize(rs, NullStreamError)) {
            stopIteration = true;
            return;
        }
        unsigned sum = 0;
        for (std::size_t i = 0; i < meta.segmentCount(); ++i) {
            if (!info.serialize(rs, NullStreamError)) {
                stopIteration = true;
                return;
            }

            unsigned len = meta.segLen[i];
            if (info.flags[hdr::InfoField::Flags::ConsDir])
                hopDir |= (~(~0ull << len)) << sum;
            // special case: keep peering segment changes
            if (!info.flags[hdr::InfoField::Flags::Peering] || i == 0)
                segChange |= (1ull << sum);
            sum += len;
        }
        (*this)++;
    }

    template <typename Path>
    friend class RawHopRange;

private:
    ReadStream rs;
    // Flag indicating the end of the path.
    bool stopIteration = false;
    // Ingress and egress interface in path direction of the last two hop
    // fields.
    value_type prevHop, curHop;
    std::uint32_t index = 0;
    // Bit mask where a one bit indicates that the hop field at the
    // corresponding index is in a segment in construction direction (1) or
    // against construction direction (0).
    std::uint64_t hopDir = 0;
    // Bit mask that is one if the hop field at that position is the first
    // at the changeover from one segment to the next. The very first hop field
    // is also considered a segment change, but not the very last one.
    std::uint64_t segChange = 0;
};

static_assert(std::input_iterator<RawHopIter>);

template <typename Path>
class RawHopRange
{
private:
    const Path& rp;

public:
    RawHopRange(const Path& path) : rp(path) {};

    auto begin() const
    {
        if (rp.type() != hdr::PathType::SCION) {
            return RawHopIter(std::span<const std::byte>());
        }
        return RawHopIter(rp.encoded());
    }

    auto end() const { return RawHopSentinel(); }
};

} // namespace scion
