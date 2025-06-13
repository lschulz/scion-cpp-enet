from pathlib import Path
from scapy_scion.layers.scion import UDP, HopByHopExt
from scapy_scion.layers.idint import IdIntOption, IdIntEntry
from tests import write_packets


pkt1 = HopByHopExt(options=[
    IdIntOption(
        flags="discard",
        aggregation="as",
        verifier="source",
        inst_flags="node_id",
        af1="last",
        af2="last",
        inst1="ingress_tstamp",
        inst2="device_type_role",
        source_ts=1000,
        source_port=10,
        stack_len = 16,
        stack = [
            IdIntEntry(flags="source+egress", hop=0, mac=4*b"\xff"),
            IdIntEntry(flags="ingress+egress", hop=1, mask="node_id",
                mac=4*b"\xff",
                node_id=2, md1=(3).to_bytes(4, 'big'), md2=(4).to_bytes(2, 'big')),
            IdIntEntry(flags="ingress", hop=2, mask="node_id",
                mac=4*b"\xff",
                node_id=1, md1=(1).to_bytes(4, 'big'), md2=(2).to_bytes(2, 'big')),
        ]
    )
]) / UDP()

pkt2 = HopByHopExt(options=[
    IdIntOption(
        flags="encrypted",
        aggregation="off",
        verifier="third_party",
        inst_flags="node_id",
        inst1="ingress_tstamp",
        inst2="device_type_role",
        source_ts=1000,
        source_port=10,
        vl=3,
        verif_isd=1,
        verif_asn="ff00:0:1",
        verif_host="fd00::1",
        stack_len = 32,
        stack = [
            IdIntEntry(flags="source+egress+encrypted", hop=0, nonce=1, mac=4*b"\xff"),
            IdIntEntry(flags="ingress+egress+encrypted", hop=1, mask="node_id",
                nonce=2, mac=4*b"\xff",
                node_id=2, md1=(3).to_bytes(4, 'big'), md2=(4).to_bytes(2, 'big')),
            IdIntEntry(flags="ingress+encrypted", hop=2, mask="node_id",
                nonce=3, mac=4*b"\xff",
                node_id=1, md1=(1).to_bytes(4, 'big'), md2=(2).to_bytes(2, 'big')),
        ]
    )
]) / UDP()

write_packets([pkt1, pkt2], Path(__file__).with_suffix(".bin"))
