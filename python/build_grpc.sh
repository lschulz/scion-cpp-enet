#!/bin/sh

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/experimental/v1/seg_detached_extensions.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/v1/cppki.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/v1/drkey.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/v1/renewal.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/v1/seg.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/v1/seg_extensions.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/control_plane/v1/svc_resolution.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/crypto/v1/signed.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/daemon/v1/daemon.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/discovery/v1/discovery.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/drkey/v1/drkey.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/gateway/v1/control.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/gateway/v1/prefix.proto

python -m grpc_tools.protoc --python_out=. --pyi_out=. --grpc_python_out=. \
    -I .. proto/hidden_segment/v1/hidden_segment.proto
