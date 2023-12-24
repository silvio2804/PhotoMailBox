# mqtt_sub.py - Python MQTT subscribe example 
#
import paho.mqtt.client as mqtt
import base64
from PIL import Image
import io

frag_hash = dict()
total_fragments = 0
key = 0

def add_padding(base64_string):
    # Calcola il numero di caratteri di padding necessari
    print(base64_string)
    padding_needed = 4 - (len(base64_string) % 4) if len(base64_string) % 4 != 0 else 0
    
    # Aggiungi il padding alla stringa
    padded_string = base64_string + '=' * padding_needed
    print(padded_string)
    return padded_string

def assemble_and_display():
    
        img_str = "".join(frag_hash.values())
        image_data = base64.b64decode(img_str)
        with open("/home/silvioventu/Scrivania/imageFile","w") as f:
            f.write(img_str)
        # Crea un oggetto di tipo immagine usando PIL (Python Imaging Library)
        image = Image.open(io.BytesIO(image_data))

        # Visualizza l'immagine (opzionale)
        image.show()

def on_connect(client, userdata, flags, rc):
    print("Connected to broker")
 
def on_message(client, userdata, msg):
    print("Message received")
    print(type(msg.payload))
    
    str_padded = add_padding(str(msg.payload))
    topic = msg.topic
    if topic.endswith("/total_fragments"):
        # Rileva il numero totale di frammenti
        total_fragments = int(msg.payload)
        print("Total fragments:", total_fragments)
        
    if msg.payload == b"END":
        # Rilevato il messaggio di controllo "END"
        print("End of image transmission")
        if len(frag_hash) == total_fragments:
            print("the number of fragments is correct")
            assembly_and_display()
        else:
            print("error, num fragment incorrect")
    else:
        global key
        frag_hash[key] = str_padded
        key = key+1
                   

if __name__ == '__main__':
    client = mqtt.Client()
    client.username_pw_set("ciao", password='ciao')
    client.connect("localhost", 1883) 

    client.on_connect = on_connect       #attach function to callback
    client.on_message = on_message       #attach function to callback

    client.subscribe("mq2_mqtt") 
    client.loop_forever()                 #start the loop
    
