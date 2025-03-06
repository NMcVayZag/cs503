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
}
stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null || true
    fi
}
@test "remote ls command" {
    start_server

    run ./dsh -c <<EOF
ls
stop-server
EOF

    # Assertions
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ge 5 ]  # Check that there are at least 5 lines of output
    echo "Output:"
    for line in "${lines[@]}"; do
        echo "$line"
    done
        # Check if the actual output matches the expected output
    if [ "${lines[0]}" != "$expected_output" ]; then
        echo "Expected: $expected_output"
        echo "Received: ${lines[0]}"
    fi
}