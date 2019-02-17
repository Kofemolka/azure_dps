openssl rand -out $1 32
hexdump -ve '1/1 "%.2x"' $1 > $1.hex
base64 $1 > $1.base64
