from pathlib import Path
from datetime import datetime
from scapy_scion.layers.scion import UDP, SCION, SCIONPath, InfoField, HopField
from tests import write_packets


keys = [
    "byql+EpU2czJMKtRSH8ybA==",
    "6kWxcoeOx7QXW5Ydt9p6Ng==",
    "lE8KhaYBJy5xHIYPdQCLMQ==",
    "lE8KhaYBJy5xHIYPdQCLMQ==",
    "aKlN2XehHJwdhxWv/wbw0A==",
]

pkt = SCION(
    qos = 32,
    fl = 0xf_ffff,
    dst_isd = 1,
    dst_asn = "ff00:0:1",
    src_isd = 2,
    src_asn = "ff00:0:2",
    dst_host = "10.0.0.1",
    src_host = "fd00::2",
    path = SCIONPath(
        curr_inf = 1,
        curr_hf = 4,
        seg0_len = 3,
        seg1_len = 2,
        seg2_len = 0,
        info_fields = [
            InfoField(flags="", segid=1,
                timestamp=datetime.fromisoformat("2025-03-25T12:00:00Z")),
            InfoField(flags="C", segid=2,
                timestamp=datetime.fromisoformat("2025-03-25T12:00:00Z")),
        ],
        hop_fields = [
            HopField(exp_time=0, cons_ingress=0, cons_egress=1),
            HopField(exp_time=0, cons_ingress=2, cons_egress=3),
            HopField(exp_time=0, cons_ingress=4, cons_egress=0),
            HopField(exp_time=255, cons_ingress=0, cons_egress=5),
            HopField(exp_time=255, cons_ingress=6, cons_egress=0),
        ],
    )
)
pkt.path.init_path(keys, seeds=[b"\x9d\x53", b"\x69\x91"])

pkt /= UDP(
    sport = 43000,
    dport = 1200,
)

pkt /= (1337).to_bytes(4, "big")

write_packets([pkt], Path(__file__).with_suffix(".bin"))
