#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

#Test executing external ls command
@test "execute external command ls -l" {
    run ./dsh <<EOF
echo "ls -l" | ./dsh 
EOF
    [ "$status" -eq 0 ]
}

# Test the cd command with too many arguments
@test "cd with too many arguments" {
    run ./dsh <<EOF
cd /tmp /var
EOF
    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>TOOMANYARGSFORCD"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$status" -eq 1 ]
    [ "$stripped_output" = "$expected_output" ]
}

# Test executing an invalid command
@test "execute invalid command" {

    run "./dsh" <<EOF
invalidcommand
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>execvperror:invalidcommanddsh2>Childprocessexitedwithstatus1"

    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -ne 0 ]
}

# Test the exit command
@test "exit command" {

    run "./dsh" <<EOF
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>"

    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}