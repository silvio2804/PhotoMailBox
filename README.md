# PhotoMailBox
IOT Project for Master in Cybersecurity.

## Goal
The goal of this project is to implement a smart box for the letters. 

## Use case 
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
Rabbitmq server is configured to accept only certified client.  
Esp32 code verify server certificate.
The connection is cyphered with RSA thanks to tls handshake.

## Server configuration 

## Generate certificate
X500

## Esp32

* foto dell'esp32 su mb
* programmatore
* ecc
