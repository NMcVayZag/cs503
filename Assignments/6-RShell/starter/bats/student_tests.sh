#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file
#######################################################LOCAL___TESTING############################################################################################

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Basic single piping: ls | wc -l" {
    run ./dsh <<EOF
ls | wc -l
EOF

    # Assertions
    [ "$status" -eq 0 ]
    [ "${lines[0]}" -gt 0 ]  # Check that the output is a positive number
}

@test "Check multi piping: echo -e 'line1\nline2\nline3' | grep line | wc -l" {
    run ./dsh <<EOF
echo -e 'line1\nline2\nline3' | grep line | wc -l
EOF

    # Assertions
    [ "$status" -eq 0 ]
    [ "${lines[0]}" -eq 3 ]  # assert that ouput is 3
}

@test "Too many commands in pipeline" {
    run ./dsh <<EOF
echo cmd1 | echo cmd2 | echo cmd3 | echo cmd4 | echo cmd5 | echo cmd6 | echo cmd7 | echo cmd8 | echo cmd9
EOF

    # Assertions
    [ "$status" -ne 0 ]  # Check that the shell exited with a non-zero status
}

@test "Echo command with arguments" {
    run ./dsh <<EOF
echo Hello World
EOF

    # Expected output
    expected_output="Hello World"

    # Assertions
    [ "$status" -eq 0 ]  # checkl for exit status 0
    [ "${lines[0]}" = "$expected_output" ]  # Check that the output matches the expected output

    # print expected output versus received output in failure
    if [ "${lines[0]}" != "$expected_output" ]; then
        echo "Expected: $expected_output"
        echo "Received: ${lines[0]}"
    fi
}
########################################### REMOTE TESTING ##################################################################################
start_server() {
    ./dsh -s &
    SERVER_PID=$!
    sleep 1  # Give the server time to start
    echo "Server started with PID $SERVER_PID"
}
stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null || true
    fi
}
@test "Remote ls command" {
    start_server

    run ./dsh -c <<EOF
ls
stop-server
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    expected_substring="responsefromserver:batsdshdsh_cli.cdshlib.cdshlib.hmakefilersh_cli.crsh_server.crshlib.h"

  
    #echo "Output:"
    #for line in "${lines[@]}"; do
        #echo "$line"
    #done
        # Check if the actual output contains expected output
    if [[ "$stripped_output" != *"$expected_substring"* ]]; then
        echo "Expected substring not found in output"
        echo "Expected substring: $expected_substring"
        echo "Stripped Output: $stripped_output"
    fi
        # Assertions
    [ "$status" -eq 0 ]
        # Check if ls response is received
    [[ "$stripped_output" == *"$expected_substring"* ]]
}

@test "Remote single piping: ls | wc -l" {
    start_server

    run ./dsh -c <<EOF
ls | wc -l
stop-server
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    expected_substring="responsefromserver:9"

        # Check if the actual output contains expected output
    if [[ "$stripped_output" != *"$expected_substring"* ]]; then
        echo "Expected substring not found in output"
        echo "Expected substring: $expected_substring"
        echo "Stripped Output: $stripped_output"
    fi
        # Assertions
    [ "$status" -eq 0 ]
        # Check if ls response is received
    [[ "$stripped_output" == *"$expected_substring"* ]]
}

@test "Remote multi piping: echo -e 'line1\nline2\nline3' | grep line | wc -l" {
    start_server

    run ./dsh -c <<EOF
echo -e 'line1\nline2\nline3' | grep line | wc -l
stop-server
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    expected_substring="responsefromserver:3"

        # Check if the actual output contains expected output
    if [[ "$stripped_output" != *"$expected_substring"* ]]; then
        echo "Expected substring not found in output"
        echo "Expected substring: $expected_substring"
        echo "Stripped Output: $stripped_output"
    fi
        # Assertions
    [ "$status" -eq 0 ]
        # Check if ls response is received
    [[ "$stripped_output" == *"$expected_substring"* ]]
}

@test "Remote invalid command check" {
    start_server

    run ./dsh -c <<EOF
apple
stop-server
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')


    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "StrippedOutput: $strippedoutput"
    echo "Exit Status: $status"
    expected_substring="Executingcommand:appleinvalidinput"

        # Check if the actual output contains expected output
    if [[ "$stripped_output" != *"$expected_substring"* ]]; then
        echo "Expected substring not found in output"
        echo "Expected substring: $expected_substring"
        echo "Stripped Output: $stripped_output"
    fi
        # Assertions
    [ "$status" -eq 0 ]
        # Check if ls response is received
    [[ "$stripped_output" == *"$expected_substring"* ]]
}

@test "Remote cd functionality check" {
    start_server

    run ./dsh -c <<EOF
cd bats
ls
stop-server
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')


    # These echo commands will help with debugging and will only print if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "StrippedOutput: $strippedoutput"
    echo "Exit Status: $status"
    expected_substring="responsefromserver:assignment_tests.shstudent_tests.sh"

        # Check if the actual output contains expected output
    if [[ "$stripped_output" != *"$expected_substring"* ]]; then
        echo "Expected substring not found in output"
        echo "Expected substring: $expected_substring"
        echo "Stripped Output: $stripped_output"
    fi
        # Assertions
    [ "$status" -eq 0 ]
        # Check if ls response is received
    [[ "$stripped_output" == *"$expected_substring"* ]]
}