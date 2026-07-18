from pathlib import Path
import re


source = (Path(__file__).parents[1] / "climate-monitor.ino").read_text()

alphabet = re.search(r'ACCESS_KEY_ALPHABET\[\] = "([a-z0-9]+)"', source).group(1)
assert len(alphabet) == 32
assert len(set(alphabet)) == 32
assert not set("lo01") & set(alphabet)
assert "constexpr size_t ACCESS_KEY_LENGTH = 12;" in source
assert "constexpr unsigned long ACCESS_INFO_DISPLAY_MS = 120000UL;" in source
assert "WiFi.softAP(AP_SSID, credentialStorage.accessKey);" in source
assert "startsWith(configServer.AuthTypeDigest)" in source
assert "configServer.requestAuthentication(DIGEST_AUTH, WEB_REALM);" in source
assert source.count("if (!authenticateConfigRequest())") == 4
assert "htmlEscape(credentialStorage.password)" not in source
assert "memcpy(emptyCredentials.accessKey, credentialStorage.accessKey" in source
