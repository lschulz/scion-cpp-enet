from pathlib import Path
from scapy.layers.inet import IP, ICMP
from tests import write_packets

pkt = IP(
    tos = 32,
    ttl = 20,
    id = 1,
    flags = "DF",
    frag = 0,
    src = "10.0.0.1",
    dst = "10.0.0.2"
)

pkt /= ICMP(
    type = 0,
    code = 0,
    id = 1337,
    seq = 1,
)

write_packets([pkt], Path(__file__).with_suffix(".bin"))
