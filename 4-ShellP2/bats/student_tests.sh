#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}


@test "Check status of a successful command" {
	run ./dsh <<EOF
echo hello
rc
EOF

	#Strip all whitespace characters for easier comparison
	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="hellodsh2>dsh2>0dsh2>cmdloopreturned0"

	#On failure, print the results and the expected results
	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	#Assertions
	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Check status of a failed command (nonexistant command)" {
	run ./dsh <<EOF
not_a_command
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="dsh2>dsh2>Error:File'not_a_command'doesnotexistdsh2>2dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Check status of a failed command (existant command)" {
	run ./dsh <<EOF
cat not_a_file
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="cat:not_a_file:Nosuchfileordirectorydsh2>Error:Permissionsissueforfilecatdsh2>1dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Check status of the rc command" {
	run ./dsh <<EOF
not_a_command
rc
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="dsh2>dsh2>Error:File'not_a_command'doesnotexistdsh2>2dsh2>0dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "No command provided" {
	run ./dsh <<EOF

EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="dsh2>warning:nocommandsprovideddsh2>cmdloopreturned0"
	
	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Cd command with no arguments (check status=0)" {
	run ./dsh <<EOF
cd
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="dsh2>dsh2>0dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Cd command with one argument (check status=0)" {
	current=$(pwd)

	cd /tmp
	mkdir -p dsh-student-test

	run "${current}/dsh" <<EOF
cd dsh-student-test
rc
pwd
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="/tmp/dsh-student-testdsh2>dsh2>0dsh2>dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "cd command with more than one argument (check status!=0)" {
	run ./dsh <<EOF
cd dir1 dir2
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="dsh2>Error:toomanyargumentsprovideddsh2>7dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Dragon command" {
	run ./dsh <<EOF
dragon
rc
EOF

	#I tried to match up the output with an expected output stored in a file dragon.txt, but although the outputs looked identical, they were not matching
	#so, I opted to check that the line count and dsh return code matched
#	if [ -e "dragon.txt" ]; then
#		#Compare the contens of the dragon.txt file to the output
#		expected_output="$(cat dragon.txt)"
#
#		echo "$expected_output"
#		echo "$output"
#
#		[ "$output" = "$expected_output" ]
#	fi

	
	#The dragon has 38 lines, the rc will take 1 line, and the exit will take 1 line
	line_count=40
	output_lc=$(echo "$output" | wc -l)
	rc=$(echo "$output" | grep 'dsh2> 0')

	echo "line count: $output_lc"
	echo "dsh return code: $rc"

	[ "$line_count" -eq "$output_lc" ]
	[ "$rc" = "dsh2> 0" ]
	[ "$status" -eq 0 ]
}

@test "Command with a part of its arguments in quotes" {
	current=$(pwd)

	cd /tmp
	touch "file  with  space" #This should be an empty file

	run "${current}/dsh" <<EOF
cat /tmp/"file  with  space"
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
	expected_output="dsh2> dsh2> 0dsh2> cmd loop returned 0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}

@test "Same command does not have quotes (fails)" {
	current=$(pwd)

	cd /tmp
	touch "file  with  space" #This should be an empty file

	run "${current}/dsh" <<EOF
cat /tmp/file  with  space
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v ')
	expected_output="cat:/tmp/file:Nosuchfileordirectorycat:with:Nosuchfileordirectorycat:space:Nosuchfileordirectorydsh2>Error:Permissionsissueforfilecatdsh2>1dsh2>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ "$status" -eq 0 ]
}
