#!/usr/bin/env python3
import math


PRESSURE_MIN_HPA = 850.0
PRESSURE_MAX_HPA = 1100.0
def parse_pressure(value_text, quality):
    if quality not in (0, 1):
        return None
    try:
        value = float(value_text)
    except ValueError:
        return None
    return value if PRESSURE_MIN_HPA <= value <= PRESSURE_MAX_HPA else None


def altitude(temperature_c, pressure_hpa, sea_level_pressure_hpa):
    return ((sea_level_pressure_hpa / pressure_hpa) ** (1 / 5.257) - 1) * (temperature_c + 273.15) / 0.0065


def extract_noon_sea_level_pressure(payload, timestamp):
    value, quality = payload[timestamp]["normalPressure"]
    return parse_pressure(str(value), quality)


def main():
    assert parse_pressure("1001.1", 0) == 1001.1
    assert parse_pressure("1001.1", 1) == 1001.1
    assert parse_pressure("1001.1", 2) is None
    assert parse_pressure("null", 0) is None
    assert parse_pressure("1200.0", 0) is None
    fixture = {"20260717120000": {"normalPressure": [1004.0, 0]}}
    assert extract_noon_sea_level_pressure(fixture, "20260717120000") == 1004.0
    assert math.isclose(altitude(25.0, 998.4, 1001.1), 23.57, abs_tol=0.01)


if __name__ == "__main__":
    main()
