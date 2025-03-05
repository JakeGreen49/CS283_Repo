#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Commands work no piping" {
    run ./dsh <<EOF                
echo hello world
EOF
	
	# Trim the output of whitespace to make it easier to compare
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="helloworlddsh3>dsh3>cmdloopreturned0"

	# On failure, print the output, status, and expected output
	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

    # Assertions
    [ "$status" -eq 0 ]
	[ "$stripped_output" = "$expected_output" ]
}

@test "Built-in commands work" {
	run ./dsh <<EOF
cd /
rc
pwd
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="/dsh3>dsh3>0dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

    [ "$status" -eq 0 ]
	[ "$stripped_output" = "$expected_output" ]
}

@test "Piping works with 3 commands" {
	run ./dsh <<EOF
cat bats/student_tests.sh | grep "^@test" | wc -l
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="16dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]
}

@test "Pipe limit error" {
	run ./dsh <<EOF
c1 | c2 | c3 | c4 | c5 | c6 | c7 | c8 | c9
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="dsh3>error:pipinglimitedto8commandsdsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]
}

@test "Error at the end of a pipeline (rc != 0)" {
	run ./dsh <<EOF
cat bats/student_tests.sh | not_a_command
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="dsh3>dsh3>Error:Errorinexecutingcommand'not_a_command'dsh3>512dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]
}

@test "Error inside of a pipeline (rc = 0)" {
	run ./dsh <<EOF
echo hello world | grep "q" | wc -l
rc
EOF
	
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="0dsh3>Error:Errorinexecutingcommand'grep'dsh3>0dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]
}

@test "Pipe max number of commands (8)" {
	run ./dsh <<EOF
echo "What a pipeline" | grep Wh | sed s/h//g | grep Wat | tr -d " " | grep "[^ ]" | wc -m | grep 13
rc
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="13dsh3>dsh3>0dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "Quoted arguments work inside a pipeline" {
	current=$(pwd)

	cd /tmp
	touch "file  with  space"

	run "${current}/dsh" <<EOF
ls -1 /tmp | grep "file  with  space"
EOF
	
	# For this test we do not trim the spaces to check that they were used in the command as expected
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v')
	expected_output="file  with  spacedsh3> dsh3> cmd loop returned 0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "STDIN redirection (<) works with a single command" {
	current=$(pwd)

	cd /tmp
	touch stdin_file
	echo "Feed me into a command" > stdin_file

	run "${current}/dsh" <<EOF
wc -m < /tmp/stdin_file
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="23dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "STDOUT redirection (>) works with a single command" {
	run ./dsh <<EOF
echo "Feed me into a file" > tmp.txt
cat tmp.txt
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="Feedmeintoafiledsh3>dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "STDOUT redirection in append mode (>>) works with a single command" {
	run ./dsh <<EOF
echo "We got a second line now" >> tmp.txt
cat tmp.txt
rm tmp.txt
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="FeedmeintoafileWegotasecondlinenowdsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "STDIN redirection (<) works inside a pipeline" {
	run ./dsh <<EOF
grep ".*" < /tmp/stdin_file | wc -m
EOF

	# This should have an identical output to the STDIN redirect test with a single command
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="23dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "STDOUT redirection (>) works inside a pipeline" {
	run ./dsh <<EOF
ls -1 | grep "\.h$" > tmp.txt | wc -l
cat tmp.txt
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="0dshlib.hdsh3>dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "STDOUT append mode redirection (>>) works inside a pipeline" {
	run ./dsh <<EOF
ls -1 | grep "\.c$" >> tmp.txt | wc -l
cat tmp.txt
rm tmp.txt
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="0dshlib.hdragon.cdsh_cli.cdshlib.cdsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "Multiple redirection modes work on an individual command" {
	run ./dsh <<EOF
grep ".*" < /tmp/stdin_file > tmp2.txt
cat tmp2.txt
sed "s/Feed/Append/g" < tmp2.txt >> tmp2.txt
cat tmp2.txt
rm tmp2.txt
EOF

	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="FeedmeintoacommandFeedmeintoacommandAppendmeintoacommanddsh3>dsh3>dsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}

@test "All three redirection modes work at once" {
	run ./dsh <<EOF
sed "s/Feed/Mash/g" < /tmp/stdin_file > tmp3.txt >> tmp3.txt
cat tmp3.txt
rm tmp3.txt
EOF

	# Even though we redirect out in append mode after in normal mode, there will still only be one line
	stripped_output=$(echo "$output" | tr -d '\t\f\r\n\v ')
	expected_output="Mashmeintoacommanddsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

	echo "Captured stdout: $output"
	echo "Output: $stripped_output"
	echo "Status: $status"
	echo "Expected output: $expected_output"

	[ "$stripped_output" = "$expected_output" ]
	[ $status -eq 0 ]

}
