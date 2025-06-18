#include "scion/error_codes.hpp"
#include "scion/addr/endpoint.hpp"
#include "scion/addr/generic_ip.hpp"
#include "scion/bsd/sockaddr.hpp"
#include "console.hpp"
#include "format.hpp"

#define ENET_IMPLEMENTATION
#include "enet/enet.h"

#include <CLI/CLI.hpp>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <span>
#include <string>


struct Arguments
{
    std::string sciond = "127.0.0.1:30255";
    std::string bindAddr;
    std::string connectTo;
};

int parseCommandLine(int argc, char* argv[], Arguments& args)
{
    CLI::App app{"SCION-ENet Demo"};
    app.add_option("-d,--sciond", args.sciond,
        "SCION daemon address (default \"127.0.0.1:30255\")")
        ->envname("SCION_DAEMON_ADDRESS");
    app.add_option("-b,--bind", args.bindAddr, "Bind address")
        ->required();
    app.add_option("-c,--connect", args.connectTo, "Server to connect to")
        ->group("Client");
    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    return 0;
}

int run(Arguments&);
int runServer(ENetAddress&);
int runClient(ENetAddress&, ENetAddress&);

int main(int argc, char* argv[])
{
    Arguments args;
    if (int ec = parseCommandLine(argc, argv, args); ec) {
        return ec;
    }

    if (enet_initialize(args.sciond.c_str())) {
        std::cerr << "Error during ENet initialization.\n";
        return EXIT_FAILURE;
    }

    int result = run(args);

    enet_deinitialize();
    return result;
}

std::error_code parseENetAddr(std::string_view raw, ENetAddress& addr)
{
    using namespace scion;
    if (auto bind = generic::IPEndpoint::Parse(raw); isError(bind)) {
        return bind.error();
    } else {
        if (auto host = scion::generic::toUnderlay<in6_addr>(bind->getHost().map4in6()); isError(host)) {
            return host.error();
        } else {
            addr.host = *host;
        }
        addr.isdAsn = 0;
        addr.port = bind->getPort();
        addr.sin6_scope_id = 0;
    }
    return ErrorCode::Ok;
}

std::error_code parseENetScionAddr(std::string_view raw, ENetAddress& addr)
{
    using namespace scion;
    if (auto ep = Endpoint<generic::IPEndpoint>::Parse(raw); isError(ep)) {
        return ep.error();
    } else {
        if (auto host = scion::generic::toUnderlay<in6_addr>(ep->getHost().map4in6()); isError(host)) {
            return host.error();
        } else {
            addr.host = *host;
        }
        addr.isdAsn = ep->getIsdAsn();
        addr.port = ep->getPort();
        addr.sin6_scope_id = 0;
    }
    return ErrorCode::Ok;
}

template <>
struct std::formatter<ENetAddress>
{
    constexpr auto parse(auto& ctx)
    {
        return ctx.begin();
    }

    auto format(const ENetAddress& addr, auto& ctx) const
    {
        return std::format_to(ctx.out(), "[{}]:{}", addr.host, addr.port);
    }
};

int run(Arguments& args)
{
    ENetAddress bindAddress = {};
    if (parseENetAddr(args.bindAddr, bindAddress)) {
        std::cerr << std::format("Not a valid bind address: {}\n", args.bindAddr);
        return EXIT_FAILURE;
    }

    int result = 0;
    if (args.connectTo.empty()) {
        result = runServer(bindAddress);
    } else {
        ENetAddress address = {};
        if (parseENetScionAddr(args.connectTo, address)) {
            std::cerr << std::format("Not a valid address: {}\n", args.connectTo);
            return EXIT_FAILURE;
        }
        result = runClient(bindAddress, address);
    }
    return result;
}

int runServer(ENetAddress& bindAddress)
{
    const int MAX_CLIENTS = 32;
    ENetHost* server = enet_host_create(&bindAddress, MAX_CLIENTS, 2, 0, 0);
    if (!server) {
        std::cerr << "Creating server host failed\n";
        return EXIT_FAILURE;
    }

    ncurses::initServer();
    ncurses::print("Server running (press q to quit)\n");

    // Server event loop
    ENetEvent event;
    while (ncurses::getChar() != 'q' && enet_host_service(server, &event, 100) >= 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            ncurses::print(std::format("Client connected from {}\n", event.peer->address).c_str());
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            ncurses::print(std::format("Received packet from {} (channel {})\n",
                event.peer->address, event.channelID).c_str());
            ncurses::print(printBuffer(std::span<std::byte>(
                reinterpret_cast<std::byte*>(event.packet->data),
                event.packet->dataLength)).c_str());
            // echo the packet back on the same channel
            enet_peer_send(event.peer, event.channelID, event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            ncurses::print(std::format("Client disconnected {}\n", event.peer->address).c_str());
            break;

        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            ncurses::print(std::format("Client timed out {}\n", event.peer->address).c_str());
            break;

        case ENET_EVENT_TYPE_NONE:
            break;

        default:
            assert(false);
            break;
        }
    };

    ncurses::endServer();
    enet_host_destroy(server);
    return EXIT_SUCCESS;
}

int runClient(ENetAddress& bindAddress, ENetAddress& address)
{
    enum class State {
        Connecting,
        Connected,
        Disconnecting,
        Disconnected,
    } state = State::Connecting;
    int loops = 0;

    ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;
    ENetEvent event = {};

    // TODO(lschulz): Don't require bind address for SCION
    client = enet_host_create(&bindAddress, 1, 2, 0, 0);
    if (!client) {
        std::cerr << "Creating client host failed\n";
        return EXIT_FAILURE;
    }

    peer = enet_host_connect(client, &address, 2, 0);
    if (!peer) {
        std::cerr << "Connection failed\n";
        return EXIT_FAILURE;
    }

    while (state != State::Disconnected && enet_host_service(client, &event, 100) >= 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            std::cerr << std::format("Connected to {}\n", event.peer->address);
            state = State::Connected;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            std::cout << std::format("Received packet from {} (channel {})\n",
                event.peer->address, event.channelID);
            std::cout << printBuffer(std::span<std::byte>(
                reinterpret_cast<std::byte*>(event.packet->data),
                event.packet->dataLength));
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            state = State::Disconnected;
            break;

        default:
            break;
        }

        ++loops;
        if (state == State::Connecting && loops > 10) {
            std::cerr << "Connection failed\n";
            enet_peer_reset(peer);
            state = State::Disconnected;
            loops = 0;
        }
        else if (state == State::Connected) {
            if (loops < 5) {
                static const char msg[] = "Hello!";
                auto packet = enet_packet_create(msg, sizeof(msg), ENET_PACKET_FLAG_RELIABLE);
                if (packet) {
                    if (enet_peer_send(peer, 0, packet)) {
                        std::cerr << "Sending packet failed\n";
                        enet_packet_destroy(packet);
                    }
                }
            } else {
                enet_peer_disconnect(peer, 0);
                state = State::Disconnecting;
                loops = 0;
            }
        }
        else if (state == State::Disconnecting && loops > 10) {
            std::cerr << "Reset connection after timeout\n";
            enet_peer_reset(peer);
            state = State::Disconnected;
            loops = 0;
        }
    }

    enet_host_destroy(client);
    return EXIT_SUCCESS;
}
