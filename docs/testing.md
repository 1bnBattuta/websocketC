# 04/03/2026 - Testing handshake requests and server responses:
I used curl to send tailored http request to test server behaviour when faced with a non
websocket http request, alongside with the response to a valid handshake request.
base64 encoding + sha1 testing was done using a web debugger and some test cases.
Examples : curl -v \
            -H "Upgrade: websocket" \
            -H "Connection: Upgrade" \
            -H "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==" \
            -H "Sec-WebSocket-Version: 13" \
            http://localhost:443

From now i will be using "websocat" to further test a full connection. 