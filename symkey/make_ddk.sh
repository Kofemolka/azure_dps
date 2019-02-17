echo -n "$1" | openssl dgst -sha256 -mac hmac -macopt hexkey:$(cat $2) -binary | base64 > $1.ddk
