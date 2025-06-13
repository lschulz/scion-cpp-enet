from pathlib import Path
from scapy.layers.l2 import Ether
from scapy.layers.inet import IP, UDP
from tests import write_packets


pkt = Ether(
    dst = "00:00:00:00:00:01",
    src = "00:00:00:00:00:02"
)

pkt /= IP(
    tos = 32,
    ttl = 20,
    id = 1,
    flags = "MF",
    frag = 0,
    src = "10.0.0.1",
    dst = "10.0.0.2"
)

pkt /= UDP(
    sport = 43000,
    dport = 1200,
)

pkt /= (1337).to_bytes(4, "big")

write_packets([pkt], Path(__file__).with_suffix(".bin"))
