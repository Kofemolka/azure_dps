echo "$1"

echo '[Generating Key]'
openssl genrsa -out $1.key 2048
echo '[Creating request]'
openssl req -new -key $1.key -out $1.csr -subj "/C=/ST=/L=/O=farmersedge/CN=$1"
echo '[Creating certificate]'
openssl x509 -req -in $1.csr -CA $2.pem -CAkey $2.key -CAcreateserial -out $1.pem -days 1024 -sha256


