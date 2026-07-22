from pathlib import Path


source = (Path(__file__).parents[1] / "climate-monitor.ino").read_text()

assert "ACCESS_CREDENTIAL_MAGIC" in source
assert "storedMagic == ACCESS_CREDENTIAL_MAGIC" in source
assert "char grafanaApiKey[MAX_GRAFANA_API_KEY_LENGTH + 1];" in source
assert 'name=\\"grafana_api_key\\" maxlength=\\"256\\"' in source
assert "credentialStorage.grafanaApiKey[0] != '\\0'" in source
assert "http.setAuthorization(GRAFANA_INSTANCE_ID, credentialStorage.grafanaApiKey);" in source
assert r'\"timeUnixNano\"' in source
assert r'\"asDouble\"' in source
assert source.count(r'\"key\":\"deviceId\"') == 1
assert source.count(r'\"key\":\"location\"') == 1
assert source.count(r'\"key\":\"sensor\"') == 1
assert "lastGrafanaPostMs" in source
assert "GRAFANA_POST_INTERVAL_MS" in source
assert "char latestGrafanaPayload[GRAFANA_PAYLOAD_SIZE]{};" in source
assert "size_t latestGrafanaPayloadLength = 0;" in source
assert "static char payload[4096]" not in source
assert 'configServer.on("/weather", HTTP_GET, handleWeather);' in source
assert 'configServer.on("/api/grafana/latest", HTTP_GET, handleLatestGrafanaPayload);' in source
assert source.count("if (!authenticateConfigRequest())") >= 6
assert "configServer.sendContent(latestGrafanaPayload, latestGrafanaPayloadLength);" in source
assert "pathLength=\"100\"" in source
assert "setInterval(loadLatest,60000);" in source
assert "if(value>=31)" in source
assert "if(value>=28)" in source
assert "if(value>=25)" in source
assert "glc_" not in source
