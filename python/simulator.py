
def determine_state(voltage, current, temperature):
    if temperature > 80.0 or current > 8.0 or voltage > 30.0:
        return "FAULT"

    if temperature >= 60.0 or current > 5.0 or voltage > 26.0:
        return "WARNING"

    return "NORMAL"

scenarios = [
    {
        "name": "NORMAL",
        "voltage": 24.0,
        "current": 3.0,
        "temperature": 40.0,
        "expected_state": "NORMAL"
    },
    {
        "name": "HIGH TEMPERATURE",
        "voltage": 24.0,
        "current": 3.0,
        "temperature": 70.0,
        "expected_state": "WARNING"
    },
    {
        "name": "OVERCURRENT",
        "voltage": 24.0,
        "current": 8.0,
        "temperature": 65.0,
        "expected_state": "WARNING"
    },
    {
        "name": "OVERVOLTAGE",
        "voltage": 30.0,
        "current": 3.0,
        "temperature": 45.0,
        "expected_state": "WARNING"
    },
    {
        "name": "CRITICAL",
        "voltage": 30.0,
        "current": 8.0,
        "temperature": 90.0,
        "expected_state": "FAULT"
    }
]


passed = 0
failed = 0

for scenario in scenarios:
    actual_state = determine_state(
        scenario["voltage"],
        scenario["current"],
        scenario["temperature"]
    )

    expected_state = scenario["expected_state"]

    print("Scenario:", scenario["name"])
    print("  Expected:", expected_state)
    print("  Actual:  ", actual_state)

    if actual_state == expected_state:
        print("  Result:   PASS")
        passed += 1
    else:
        print("  Result:   FAIL")
        failed += 1

    print()

print("----------------------------")
print("Tests passed:", passed)
print("Tests failed:", failed)
print("Total tests:", passed + failed)