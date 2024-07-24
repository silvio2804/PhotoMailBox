# PhotoMailBox
IOT Project for Master in Cybersecurity.

## Goal
The goal of this project is to implement a smart box for the letters. 

**Use case**:
* The post man inserts a letter inside the mailbox. The smartbox is an esp32 placed inside the mail that will take a photo of the content.
* The photo will be sent to an MQTT server
* A client python download the image.

## Architecture
The project follows the message oriented architecture:
* esp32 is producer of the message (photo);
* rabbitmq is the broker that receave the message. It stores the message in a queue;
* python client is the consumer that downloads the photo from the server.

## Security
**Mtls** (mutual tls) allows client to verify server certificate and viceversa.
* Rabbitmq server is configured to accept only certified client.  
* Esp32 code verify server certificate.
* The connection is cyphered with RSA thanks to tls handshake.

## Server configuration 
If not exists, create a file rabbitmq.conf in /etc/rabbitmq

```
listeners.ssl.default = 5671

ssl_options.cacertfile = /etc/pki/tls/genCert/ca_s.pem
ssl_options.certfile = /etc/pki/tls/genCert/server_s.pem
ssl_options.keyfile = /etc/pki/tls/genCert/server.key

#il server verifica il client con il tag verify_peer

ssl_options.verify = verify_peer
ssl_options.fail_if_no_peer_cert  = true
ssl_options.versions.1 = tlsv1.2


# default TLS-enabled port for MQTT connections
mqtt.listeners.ssl.default = 8883
mqtt.listeners.tcp.default = 1883
log.connection.level = debug

ssl_options.ciphers.1 = TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
```
Remember to change permissions of folder, otherwise rabbitmq cannot see the file.

## Generate certificates
We use self-signed cerificate x509.

Use the follow bash script to generate at one time all cerficates.

```
# IP
CA_IP_CN="192.168.XXX.XXX" # 
SERVER_IP_CN="192.168.XXX" # or FQDN
CLIENT_HOSTNAME="192.168.XXX"  #IP client

# PARAMETERS
SUBJECT_CA="/C=SE/ST=Italy/L=Roma/O=himinds/OU=CA/CN=$CA_IP_CN"
SUBJECT_SERVER="/C=SE/ST=Italy/L=Roma/O=himinds/OU=Server/CN=$SERVER_IP_CN"
SUBJECT_CLIENT="/C=SE/ST=Italy/L=Roma/O=himinds/OU=Client/CN=$CLIENT_HOSTNAME"

# Generate CA certificate
function generate_CA () {
   echo "$SUBJECT_CA"
   openssl req -x509 -nodes -sha256 -newkey rsa:2048 -subj "$SUBJECT_CA"  -days 365 -keyout ca.key -out ca.crt
}
# Generate server cerficate, 
function generate_server () {
   echo "$SUBJECT_SERVER"
   openssl req -nodes -sha256 -new -subj "$SUBJECT_SERVER" -keyout server.key -out server.csr  # server certificate, key
   openssl x509 -req -sha256 -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 # sign certificate by CA
}

# Generate client certificate
function generate_client () {
   echo "$SUBJECT_CLIENT"
   openssl req -new -nodes -sha256 -subj "$SUBJECT_CLIENT" -out client.csr -keyout client.key 
   openssl x509 -req -sha256 -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365
}

# call funcions

generate_CA
generate_server
generate_client
```
## Import certificate into arduino IDE
Use an older version of Arduino IDE: 1.X. Install a plugin .jar to deploy certificate into esp32.


