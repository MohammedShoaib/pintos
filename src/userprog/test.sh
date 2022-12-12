#!/bin/sh
make build/tests/userprog/args-none.result
make build/tests/userprog/args-single.result
make build/tests/userprog/args-multiple.result
make build/tests/userprog/args-many.result
make build/tests/userprog/args-dbl-space.result
make build/tests/userprog/halt.result
make build/tests/userprog/exit.result
make build/tests/userprog/open-normal.result
make build/tests/userprog/open-missing.result
make build/tests/userprog/open-boundary.result
make build/tests/userprog/open-empty.result
make build/tests/userprog/open-null.result
make build/tests/userprog/open-bad-ptr.result
make build/tests/userprog/open-twice.result