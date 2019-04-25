#!/usr/bin/env bash

protoc --python_out=../server --cpp_out=../src/DynSLAM/InstRecLib server_result.proto