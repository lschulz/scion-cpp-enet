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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utilities.hpp"


TEST(PathMeta, threeHops)
{
    namespace pm = scion::path_meta;
    using namespace scion;
    using namespace std::chrono_literals;

    proto::daemon::v1::Path pb;
    {
        auto buf = loadPackets("path/data/path_metadata_3hops.bin").at(0);
        ASSERT_TRUE(pb.ParseFromArray(buf.data(), (int)(buf.size())));
    }

    static const std::array<pm::Hop, 3> expectedHops = {
        pm::Hop{
            .isdAsn = IsdAsn(0x1ff0000000111),
            .ingress = 0,
            .egress = 2,
        },
        pm::Hop{
            .isdAsn = IsdAsn(0x2ff0000000211),
            .ingress = 5,
            .egress = 4,
        },
        pm::Hop{
            .isdAsn = IsdAsn(0x2ff0000000222),
            .ingress = 3,
            .egress = 0,
        },
    };
    pm::Interfaces hops;
    hops.initialize(pb);
    EXPECT_THAT(hops.data, testing::ElementsAreArray(expectedHops));

    static const std::array<pm::HopMeta, 3> expectedHopMeta = {
        pm::HopMeta{
            .ingRouter = pm::GeoCoordinates{},
            .egrRouter = pm::GeoCoordinates{},
            .internalHops = 0,
            .notes = "AS1-ff00:0:111",
        },
        pm::HopMeta{
            .ingRouter = pm::GeoCoordinates{},
            .egrRouter = pm::GeoCoordinates{},
            .internalHops = 2,
            .notes = "AS2-ff00:0:211",
        },
        pm::HopMeta{
            .ingRouter = pm::GeoCoordinates{},
            .egrRouter = pm::GeoCoordinates{},
            .internalHops = 0,
            .notes = "AS2-ff00:0:222",
        },
    };
    pm::HopMetadata hmeta;
    hmeta.initialize(pb);
    EXPECT_THAT(hmeta.data, testing::ElementsAreArray(expectedHopMeta));

    static const std::array<pm::LinkMeta, 3> expectedLinks = {
        pm::LinkMeta{
            .type = pm::LinkType::Direct,
            .latency = 5ms,
            .bandwidth = 10'000'000,
        },
        pm::LinkMeta{
            .type = pm::LinkType::Internal,
            .latency = 10ms,
            .bandwidth = 1'000'000,
        },
        pm::LinkMeta{
            .type = pm::LinkType::Unspecified,
            .latency = 8ms,
            .bandwidth = 5'000'000,
        },
    };
    pm::LinkMetadata links;
    links.initialize(pb);
    EXPECT_THAT(links.data, testing::ElementsAreArray(expectedLinks));
}

// Test initializing metadata structs with some slightly invalid protobufs.
// There is no error checking, but the code should not crash and still provide
// an output reflecting the incorrect input data.
TEST(PathMeta, invalidMetadata)
{
    namespace pm = scion::path_meta;
    using namespace scion;
    using namespace std::chrono_literals;

    proto::daemon::v1::Path pb;
    {
        auto buf = loadPackets("path/data/path_metadata_invalid.bin").at(0);
        ASSERT_TRUE(pb.ParseFromArray(buf.data(), (int)(buf.size())));
    }

    static const std::array<pm::Hop, 2> expectedHops = {
        pm::Hop{
            .isdAsn = IsdAsn(0x1ff0000000111),
            .ingress = 0,
            .egress = 2,
        },
        pm::Hop{
            .isdAsn = IsdAsn(0x2ff0000000222),
            .ingress = 5,
            .egress = 3,
        },
    };
    pm::Interfaces hops;
    hops.initialize(pb);
    EXPECT_THAT(hops.data, testing::ElementsAreArray(expectedHops));

    static const std::array<pm::HopMeta, 2> expectedHopMeta = {
        pm::HopMeta{
            .ingRouter = pm::GeoCoordinates{},
            .egrRouter = pm::GeoCoordinates{},
            .internalHops = 0,
            .notes = "AS1-ff00:0:111",
        },
        pm::HopMeta{
            .ingRouter = pm::GeoCoordinates{},
            .egrRouter = pm::GeoCoordinates{},
            .internalHops = 0,
            .notes = "AS2-ff00:0:211",
        },
    };
    pm::HopMetadata hmeta;
    hmeta.initialize(pb);
    EXPECT_THAT(hmeta.data, testing::ElementsAreArray(expectedHopMeta));
}
