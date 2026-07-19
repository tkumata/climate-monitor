#!/usr/bin/env python3
import math
from pathlib import Path


def wet_bulb(temperature_c, relative_humidity):
    return (
        temperature_c * math.atan(0.151977 * math.sqrt(relative_humidity + 8.313659))
        + math.atan(temperature_c + relative_humidity)
        - math.atan(relative_humidity - 1.676331)
        + 0.00391838 * relative_humidity**1.5 * math.atan(0.023101 * relative_humidity)
        - 4.686035
    )


assert math.isclose(wet_bulb(30.0, 70.0), 25.60, abs_tol=0.01)
assert math.isclose(0.7 * wet_bulb(30.0, 70.0) + 0.3 * 30.0, 26.92, abs_tol=0.01)

source = (Path(__file__).parents[1] / "climate-monitor.ino").read_text()
assert "float computeIndoorWbgt(float tempC, float relativeHumidity)" in source
assert "metrics.wbgt = computeIndoorWbgt(data.temperature, data.humidity);" in source
assert "atanf(0.151977f * sqrtf(humidity + 8.313659f))" in source
