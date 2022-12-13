const char *ssid1 = "";
const char *password1 = "";
const char *ssid2 = "";
const char *password2 = "";

String clientSecret = "";
String clientID = "";
String access_token = "";

// follow https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/ on how to obtain one
const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFNjCCBNygAwIBAgIQCH4DWYOebCLz/pcJdRheAzAKBggqhkjOPQQDAjBKMQsw\n"
    "CQYDVQQGEwJVUzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEgMB4GA1UEAxMX\n"
    "Q2xvdWRmbGFyZSBJbmMgRUNDIENBLTMwHhcNMjIwNjA2MDAwMDAwWhcNMjMwNjA1\n"
    "MjM1OTU5WjB1MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTEWMBQG\n"
    "A1UEBxMNU2FuIEZyYW5jaXNjbzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEe\n"
    "MBwGA1UEAxMVc25pLmNsb3VkZmxhcmVzc2wuY29tMFkwEwYHKoZIzj0CAQYIKoZI\n"
    "zj0DAQcDQgAEM7bSvJL0gicfcXUnGu2/kqcEhJuwANui9W3rvklDFp7GsNdYPFKM\n"
    "+Zg+aDwaa7tj3eycVWpSfnNcgITEkMHhTqOCA3cwggNzMB8GA1UdIwQYMBaAFKXO\n"
    "N+rrsHUOlGeItEX62SQQh5YfMB0GA1UdDgQWBBSSC2QXAXHV15bHHIrgt8vhLgZ6\n"
    "nTA+BgNVHREENzA1gg4qLnR5cGljb2RlLmNvbYIVc25pLmNsb3VkZmxhcmVzc2wu\n"
    "Y29tggx0eXBpY29kZS5jb20wDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsG\n"
    "AQUFBwMBBggrBgEFBQcDAjB7BgNVHR8EdDByMDegNaAzhjFodHRwOi8vY3JsMy5k\n"
    "aWdpY2VydC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMDegNaAzhjFodHRw\n"
    "Oi8vY3JsNC5kaWdpY2VydC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMD4G\n"
    "A1UdIAQ3MDUwMwYGZ4EMAQICMCkwJwYIKwYBBQUHAgEWG2h0dHA6Ly93d3cuZGln\n"
    "aWNlcnQuY29tL0NQUzB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGGGGh0dHA6\n"
    "Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2NhY2VydHMu\n"
    "ZGlnaWNlcnQuY29tL0Nsb3VkZmxhcmVJbmNFQ0NDQS0zLmNydDAMBgNVHRMBAf8E\n"
    "AjAAMIIBfQYKKwYBBAHWeQIEAgSCAW0EggFpAWcAdgDoPtDaPvUGNTLnVyi8iWvJ\n"
    "A9PL0RFr7Otp4Xd9bQa9bgAAAYE7QeI+AAAEAwBHMEUCIQDQ8ae0L/CsuzGyJEx5\n"
    "8cd8BsENqmUf+E9hdzaQgsqL+AIgJWmcKKSb4gaDagEqPjLkRQdgAfLqlFqxF8H3\n"
    "M9Ps83AAdgA1zxkbv7FsV78PrUxtQsu7ticgJlHqP+Eq76gDwzvWTAAAAYE7QeJf\n"
    "AAAEAwBHMEUCIQDEep0uVRRywlJ9wLVVTgTKdokn2m/D0896bNSh9GPJIwIgbgWR\n"
    "A87FTZmIbglMls/i3VJntRKmjhI2HTUKSOUaxHIAdQCzc3cH4YRQ+GOG1gWp3BEJ\n"
    "SnktsWcMC4fc8AMOeTalmgAAAYE7QeKjAAAEAwBGMEQCIDoO8v9DAdGXph7DT13R\n"
    "nVQFZg0o1866m5ldDSDqqH0LAiA78hyg5U+KGXScZfua5W4K8DPCAHCYaTVNehkA\n"
    "BRIPEjAKBggqhkjOPQQDAgNIADBFAiAX7XHrh7pjcsRgNzXCSN7MOpfGbf/lbYCP\n"
    "lztHu9UDCAIhAJJnO2X2c+mt/npSg9jIIJ1PJcUcNu+jftBIgpOXYNLE\n"
    "-----END CERTIFICATE-----\n";