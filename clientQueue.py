import pika
import io
from datetime import datetime
import ssl

rabbitmq_server = 'localhost'
rabbitmq_user = 'ciao'
rabbitmq_password = 'ciao'
rabbitmq_queue = 'mq2_amqp'

#setting certificate
certfile = '/percorso/al/tuo/certificato.pem'
keyfile = '/percorso/al/tuo/chiave_privata.pem'
cafile = '/percorso/alla/tua/CA_bundle.pem'
"""
l = list()
# La callback viene chiamata quando un messaggio viene ricevuto dalla coda
def callback(ch, method, properties, body):
    if body == b'END':
        print('Frammenti terminati. File immagine completato.')
        #apri il file e scrivi
        now = datetime.now()
        dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
        output_file = 'file_'+ dt_string.replace(":", "_").replace(" ", "_").replace("/", "_") +'.jpeg'
        print(output_file)
        with io.open(output_file, 'w+') as f:
            # Il frammento di terminazione è stato ricevuto
            for i in l:
                f.write(str(i))
            channel.stop_consuming()
            f.close()
    else:
    # Scrivi il frammento dell'immagine nel file
        l.append(body)
        print('Frammento ricevuto.')
        """

# La callback viene chiamata quando un messaggio viene ricevuto dalla coda
def callback(ch, method, properties, body):
    with io.open(output_file, 'ab') as f:
        if body == b'END':
            # Il frammento di terminazione è stato ricevuto
            print('Frammenti terminati. File immagine completato.')
            channel.stop_consuming()
            f.close()
        else:
            # Scrivi il frammento dell'immagine nel file
            f.write(body)
            print('Frammento ricevuto.')


# Configura la connessione a RabbitMQ
credentials = pika.PlainCredentials(username=rabbitmq_user, password=rabbitmq_password)
# Setting certificate
context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH, cafile=cafile)
context.load_cert_chain(certfile=certfile, keyfile=keyfile)

connection_params = pika.ConnectionParameters(rabbitmq_server, 5672, '/', credentials, ssl_options=pika.SSLOptions(context))
connection = pika.BlockingConnection(connection_params)
channel = connection.channel()

# Dichiarazione della coda
channel.queue_declare(queue=rabbitmq_queue,durable=True)

# Imposta la callback per gestire i messaggi ricevuti
channel.basic_consume(queue=rabbitmq_queue, on_message_callback=callback, auto_ack=True)

# Avvia la lettura dei messaggi
print('In attesa di frammenti...')
channel.start_consuming()
