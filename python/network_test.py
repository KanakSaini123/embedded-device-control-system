import socket
import json
import time

HOST = "127.0.0.1"
PORT = 5000

scenarios = [
    {
        "scenario": "NORMAL",
        "voltage": 24.0,
        "current": 3.0,
        "temperature": 40.0,
        "expected_state": "NORMAL"
    },
    {
        "scenario": "HIGH_TEMPERATURE",
        "voltage": 24.0,
        "current": 3.0,
        "temperature": 70.0,
        "expected_state": "WARNING"
    },
    {
        "scenario": "OVERCURRENT",
        "voltage": 24.0,
        "current": 9.0,
        "temperature": 65.0,
        "expected_state": "FAULT"
    },
    {
        "scenario": "OVERVOLTAGE",
        "voltage": 31.0,
        "current": 3.0,
        "temperature": 45.0,
        "expected_state": "FAULT"
    },
    {
        "scenario": "CRITICAL",
        "voltage": 30.0,
        "current": 9.0,
        "temperature": 90.0,
        "expected_state": "FAULT"
    }
]


passed = 0
failed = 0

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print("Connecting to C++ server...")

client.connect((HOST, PORT))

print("Connected to C++ server.")

for scenario in scenarios:
    message = json.dumps(scenario)

    print("\nSending:", message)

    client.sendall(message.encode("utf-8"))

    response = b""

    while not response.endswith(b"\n"):
        response += client.recv(512)

    response = response.decode("utf-8").strip()

    print("Received from C++:", response)

    response_data = json.loads(response)

    actual_state = response_data["state"]
    expected_state = scenario["expected_state"]

    print("Expected:", expected_state)
    print("Actual:  ", actual_state)

    if actual_state == expected_state:
        print("Result: PASS")
        passed += 1
    else:
        print("Result: FAIL")
        failed += 1

    time.sleep(2)


print("\nAll scenarios sent.")

print("\n----------------------------")
print("Tests passed:", passed)
print("Tests failed:", failed)
print("Total tests:", passed + failed)

client.close()

print("Connection closed.")