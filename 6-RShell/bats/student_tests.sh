#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

# This function will remove any extant dsh process at the end of the testcase
teardown() {
	dsh_pid="$(pgrep dsh)"
	server_up=$?
	if [ $server_up -eq 0 ]; then
		for pid in $(pgrep dsh); do
			kill -TERM "$pid"
		done
		#kill -TERM "$dsh_pid"
	fi 
}


@test "Local mode still works" {
	run ./dsh <<EOF
cat bats/student_tests.sh | grep "^@test" | wc -l
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="13localmodedsh4>dsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Client won't connect when server isn't started" {
	run ./dsh -c <<EOF
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="Connecterror:Connectionrefusedsocketclientmode:addr:127.0.0.1:1234"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -ne 0 ]
}

@test "Single threaded server, single command" {
	./dsh -s &
	run ./dsh -c <<EOF
echo hello world
stop-server
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>helloworlddsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Single threaded server, command pipeline" {
	./dsh -s &
	run ./dsh -c <<EOF
cat bats/student_tests.sh | grep ^@test | wc -l
stop-server
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>13dsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Single threaded server, pipeline too big" {
	./dsh -s &
	run ./dsh -c <<EOF
c1 | c2 | c3 | c4 | c5 | c6 | c7 | c8 | c9
stop-server
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>error:pipinglimitedto8commandsdsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}


@test "Single threaded server, built-in commands" {
	./dsh -s &
	run ./dsh -c <<EOF
cd bats
ls
rc
stop-server
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>dsh4>assignment_tests.shstudent_tests.shdsh4>0dsh4>cmdloopreturned0"
	server_up=$(ps | grep dsh | wc -l) # Should be 0 since the server and client were stopped

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"
	echo "Server up: $server_up"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
	[ "$server_up" -eq 0 ]

}

@test "Single threaded server, test exit command" {
	./dsh -s &
	run ./dsh -c <<EOF
exit
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>cmdloopreturned0"
	server_up=$(ps | grep dsh | wc -l) # Should be 1 since only the client was ended

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"
	echo "Server up: $server_up"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
	[ "$server_up" -eq 1 ]
}

@test "Single threaded server, redirections (<, >, >>) function properly" {
	touch /tmp/test_file
	./dsh -s &
	run ./dsh -c <<EOF
echo "first line" > /tmp/test_file
echo "second line" >> /tmp/test_file
wc -l < /tmp/test_file
stop-server
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>dsh4>dsh4>2dsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Single threaded server, quoted arguments" {
	./dsh -s &
	run ./dsh -c <<EOF
echo "hello    world"
stop-server
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v')
	expected_output="socket client mode:  addr:127.0.0.1:1234dsh4> hello    worlddsh4> cmd loop returned 0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Multi threaded server, single command" {
	./dsh -s -x &
	./dsh -c &
	run ./dsh -c <<EOF
echo "hello   world"
exit
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v')
	expected_output="socket client mode:  addr:127.0.0.1:1234dsh4> hello   worlddsh4> cmd loop returned 0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Multi threaded server, command pipelines" {
	./dsh -s -x &
	./dsh -c &
	run ./dsh -c <<EOF
cat bats/student_tests.sh | grep ^@test | wc -l
exit
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>13dsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]



}

@test "Multi threaded server, redirections function properly" {
	./dsh -s -x &
	./dsh -c &
	run ./dsh -c <<EOF
echo "third line" >> /tmp/test_file
cat /tmp/test_file | wc -l
rm -f /tmp/test_file
exit
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>dsh4>3dsh4>dsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

@test "Multi threaded server, built-in commands" {
	./dsh -s -x &
	./dsh -c &
	run ./dsh -c <<EOF
cd /tmp
pwd
exit
EOF

	# Trim all whitespace from the output for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="socketclientmode:addr:127.0.0.1:1234dsh4>dsh4>/tmpdsh4>cmdloopreturned0"

    # On failure, print the output, status, and expected output
    echo "Captured stdout: $output"
    echo "Output: $stripped_output"
    echo "Status: $status"
    echo "Expected output: $expected_output"

	# Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]

}

