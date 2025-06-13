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

#include "scion/path/path_meta.hpp"


namespace scion {
namespace path_meta {

void Interfaces::initialize(const proto::daemon::v1::Path& pb)
{
    if (pb.interfaces().size() > 0)
    {
        auto iface = pb.interfaces().begin();
        auto ifaceEnd = pb.interfaces().end();
        const std::size_t numIfaces = pb.interfaces().size();
        const std::size_t numASes = numIfaces / 2 + 1;

        data.resize(numASes);
        for (std::size_t i = 0; i < numASes; ++i) {
            // Interface IDs (2x per transit AS, 1x for first and last AS)
            if (i != 0 && iface != ifaceEnd) {
                data[i].isdAsn = IsdAsn(iface->isd_as());
                data[i].ingress = (iface++)->id();
            }
            if (iface != ifaceEnd) {
                data[i].isdAsn = IsdAsn(iface->isd_as());
                data[i].egress = (iface++)->id();
            }
        }
    }
}

void HopMetadata::initialize(const proto::daemon::v1::Path& pb)
{
    // Copy AS (interface) metadata
    if (pb.interfaces().size() > 0)
    {
        auto geo = pb.geo().begin();
        auto geoEnd = pb.geo().end();
        auto hops = pb.internal_hops().begin();
        auto hopsEnd = pb.internal_hops().end();
        auto notes = pb.notes().begin();
        auto notesEnd = pb.notes().end();

        const std::size_t numIfaces = pb.interfaces().size();
        const std::size_t numASes = numIfaces / 2 + 1;
        data.resize(numASes);

        for (std::size_t i = 0; i < numASes; ++i) {
            // Router locations (2x per transit AS, 1x for first and last AS)
            if (i != 0 && geo != geoEnd) {
                data[i].ingRouter = GeoCoordinates{
                    geo->latitude(),
                    geo->longitude(),
                    geo->address(),
                };
                ++geo;
            }
            if (geo != geoEnd) {
                data[i].egrRouter = GeoCoordinates{
                    geo->latitude(),
                    geo->longitude(),
                    geo->address(),
                };
                ++geo;
            }

            // Internal hops (1x per AS, except first and last AS)
            if (i != 0 && hops != hopsEnd) {
                data[i].internalHops = *hops++;
            }

            // Notes (1x per AS)
            if (notes != notesEnd) {
                data[i].notes = *notes++;
            }
        }
    }
}

void LinkMetadata::initialize(const proto::daemon::v1::Path& pb)
{
    if (pb.latency().size() > 0)
    {
        auto type = pb.link_type().begin();
        auto typeEnd = pb.link_type().end();
        auto lat = pb.latency().begin();
        auto latEnd = pb.latency().end();
        auto bw = pb.bandwidth().begin();
        auto bwEnd = pb.bandwidth().end();

        const std::size_t numLinks = pb.latency().size();
        data.reserve(numLinks);

        for (std::size_t i = 0; i < numLinks; ++i) {
            LinkMeta link;
            if (i % 2 == 0 && type != typeEnd) {
                link.type = details::linkTypeFromProtobuf(*type++);
            } else {
                link.type = LinkType::Internal;
            }
            if (lat != latEnd) {
                link.latency = ::scion::details::durationFromProtobuf(*lat++);
            }
            if (bw != bwEnd) {
                link.bandwidth = *bw++;
            }
            data.push_back(link);
        }
    }
}

} // namespace path_meta
} // namespace scion
