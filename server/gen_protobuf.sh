#! /bin/bash

protoc -I=../src/DynSLAM/InstRecLib/proto --python_out=. ../src/DynSLAM/InstRecLib/proto/server_result.proto
