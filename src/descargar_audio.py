#pip install pyserial 
#no es lo mismo que serial, por las dudas
import serial

#Este script básicamente agarra el audio grabado en la SD y loguarda en un archivo WAV. Lo uso principalmente para
#probar descargar el audio y su calidad sin meterme a levantar un servidor web

puerto = serial.Serial('COM3', 921600, timeout=2) 
nombre_archivo = "audio_recibido.wav"

print(f"Escuchando el puerto {puerto.name}...")
print("Presiona el boton CONFIRMAR en tu placa AHORA.")

with open(nombre_archivo, "wb") as archivo:
    mientras_escucha = True
    bytes_descargados = 0
    encontrado_riff = False
    buffer_residual = b""
    
    while mientras_escucha:
        datos = puerto.read(1024)
        
        if datos:
            # Si aún no encontramos el inicio del audio, buscamos la firma
            if not encontrado_riff:
                buffer_residual += datos
                indice_riff = buffer_residual.find(b'RIFF')
                
                if indice_riff != -1:
                    # ¡Encontramos la cabecera! Cortamos el texto anterior
                    encontrado_riff = True
                    datos_utiles = buffer_residual[indice_riff:]
                    archivo.write(datos_utiles)
                    bytes_descargados += len(datos_utiles)
                    print("\n[+] Cabecera WAV encontrada. Descargando audio", end="", flush=True)
            
            # Si ya estamos sincronizados, guardamos el binario puro
            else:
                archivo.write(datos)
                bytes_descargados += len(datos)
                print(".", end="", flush=True)
        else:
            print("\n\nDescarga finalizada.")
            if bytes_descargados > 0:
                print(f"¡Éxito! Se guardaron {bytes_descargados} bytes puros en '{nombre_archivo}'.")
            else:
                print("¡Atención! No se encontró la cabecera WAV en los datos recibidos.")
                
            mientras_escucha = False

puerto.close()