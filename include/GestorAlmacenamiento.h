#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define TAMANO_DATO 512
#define AJUSTE_POR_POS 256
#define TAM_CABECERA_WAV 44

class GestorAlmacenamiento
{
    private:
    //Pines tarjeta SD
    int pinCS, pinSCK, pinMISO, pinMOSI;
    File archivoAbierto;
    String ruta;
    //Archivo de audio
    int tasaMuestreo, bitsPorMuestra, canales;

    public:
    GestorAlmacenamiento(int SD_CS, int SD_SCK, int SD_MISO, int SD_MOSI)
    {
        pinCS = SD_CS;
        pinSCK = SD_SCK;
        pinMISO = SD_MISO;
        pinMOSI = SD_MOSI;
    }

    bool iniciarSD()
    {
        SPI.begin(pinSCK, pinMISO, pinMOSI, pinCS);

        return SD.begin(pinCS);
    }

    void abrirArchivoWAV(const String archivo, int tasaMuestreo, int bitsPorMuestra, int canales)
    {
        // 1. Destruimos cualquier grabación fantasma anterior para no concatenar audios
        if (SD.exists(archivo)) {
            SD.remove(archivo);
        }

        // 2. Ahora sí, creamos el archivo nuevo
        archivoAbierto = SD.open(archivo, FILE_WRITE);
        ruta = archivo;

        byte header[TAM_CABECERA_WAV] = {0};
        this->tasaMuestreo = tasaMuestreo;
        this->bitsPorMuestra = bitsPorMuestra;
        this->canales = canales;

        archivoAbierto.write(header, TAM_CABECERA_WAV);
    }

    void enviarArchivoPorSerial()
    {
        File archivoLeer = SD.open(ruta, FILE_READ);
        if (!archivoLeer) {
            return;
        }

        uint8_t buffer[128]; // Enviamos en paquetes de 128 bytes
        while (archivoLeer.available())
        {
            int bytesLeidos = archivoLeer.read(buffer, sizeof(buffer));
            // Serial.write envía los ceros y unos puros, sin convertirlos a texto
            Serial.write(buffer, bytesLeidos); 
        }

        archivoLeer.close();
    }

    void guardarDato(const uint8_t *dato, int tam)
    {
        //Escribimos en bloques de 16 bits porque nos conviene
        archivoAbierto.write(dato, tam);
        //archivoAbierto.flush(); //Limpiar el buffer obliga que los datos se escriban
    }
    
    void cerrarArchivoWAV()
    {
        if (!archivoAbierto) return;

        // 1. Sincronizamos la SD para que la memoria revele el tamaño real
        archivoAbierto.flush();
        
        unsigned long tamAudio = archivoAbierto.size() - TAM_CABECERA_WAV;
        int tamArchivo = tamAudio + TAM_CABECERA_WAV - 8;
        int tasaBytes = tasaMuestreo * canales * (bitsPorMuestra / 8);

        byte header[TAM_CABECERA_WAV];
        header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
        header[4] = (byte)(tamArchivo & 0xFF);
        header[5] = (byte)((tamArchivo >> 8) & 0xFF);
        header[6] = (byte)((tamArchivo >> 16) & 0xFF);
        header[7] = (byte)((tamArchivo >> 24) & 0xFF);
        header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
        header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
        header[16] = 16; header[17] = 0; header[18] = 0; header[19] = 0; 
        header[20] = 1; header[21] = 0; 
        header[22] = canales; header[23] = 0;
        header[24] = (byte)(tasaMuestreo & 0xFF);
        header[25] = (byte)((tasaMuestreo >> 8) & 0xFF);
        header[26] = (byte)((tasaMuestreo >> 16) & 0xFF);
        header[27] = (byte)((tasaMuestreo >> 24) & 0xFF);
        header[28] = (byte)(tasaBytes & 0xFF);
        header[29] = (byte)((tasaBytes >> 8) & 0xFF);
        header[30] = (byte)((tasaBytes >> 16) & 0xFF);
        header[31] = (byte)((tasaBytes >> 24) & 0xFF);
        header[32] = (byte)(canales * (bitsPorMuestra / 8)); header[33] = 0; 
        header[34] = bitsPorMuestra; header[35] = 0; 
        header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
        header[40] = (byte)(tamAudio & 0xFF);
        header[41] = (byte)((tamAudio >> 8) & 0xFF);
        header[42] = (byte)((tamAudio >> 16) & 0xFF);
        header[43] = (byte)((tamAudio >> 24) & 0xFF);

        // 2. CERRAMOS el archivo para destruir el candado del Modo Append
        archivoAbierto.close();

        // 3. REABRIMOS en modo "r+" (Lectura/Escritura estricta)
        archivoAbierto = SD.open(ruta, "r+");
        if (archivoAbierto) {
            archivoAbierto.seek(0, SeekMode::SeekSet); // Ahora el cursor sí va al inicio
            archivoAbierto.write(header, TAM_CABECERA_WAV); // Pisamos los ceros con los datos reales
            archivoAbierto.close();
        }
    }

    //Al no poder enviar mensajes por wi-fi, nos conformamos con leer los datos
    void leerArchivo()
    {
        //Nuevamente, leemos en bloques de 16 bits porque así escribimos 
        //(es arbitrario, es el tamaño que leemos del potenciómetro)
        //Si vamos a leer el archivo, hay que abrirlo para solo lectura
        File archivoLeer = SD.open(ruta, FILE_READ);

        uint8_t lectura[TAMANO_DATO];
        uint16_t datoFinal;
        while (archivoLeer.available())
        {
            archivoLeer.read(lectura, TAMANO_DATO);
            datoFinal = lectura[0] + lectura[1] * AJUSTE_POR_POS;
            //Mostramos el "audio" guardado en el archivo
            Serial.printf("%d\n\r", datoFinal);
        }

        archivoLeer.close();
    }
    void depurarArchivo()
    {
        File archivo = SD.open(ruta, FILE_READ);
        if (!archivo) {
            Serial.println("Error: No se pudo abrir para depurar.");
            return;
        }

        Serial.println("\n--- RADIOGRAFÍA DEL ARCHIVO WAV ---");
        Serial.print("Peso físico en la SD: ");
        Serial.print(archivo.size());
        Serial.println(" bytes\n");

        Serial.println("1. CABECERA (Primeros 44 bytes):");
        for (int i = 0; i < 44; i++) {
            if (archivo.available()) {
                int byteLeido = archivo.read();
                // Formato de 2 dígitos (ej. 0A, FF, 00)
                if (byteLeido < 16) Serial.print("0"); 
                Serial.print(byteLeido, HEX);
                Serial.print(" ");
            }
        }
        Serial.println("\n");

        Serial.println("2. MUESTRAS DE AUDIO");
        for (int i = 0; i < 64; i++) { 
            if (archivo.available()) {
                int byteLeido = archivo.read();
                if (byteLeido < 16) Serial.print("0");
                Serial.print(byteLeido, HEX);
                Serial.print(" ");
            }
        }
        Serial.println("\n-----------------------------------\n");
        archivo.close();
    }

void eliminarArchivo()
    {
        if (ruta.length() > 1) 
        {
            if (SD.exists(ruta))
            {
                SD.remove(ruta);
            }
            ruta = ""; 
        }
    }

    ~GestorAlmacenamiento(){
        if (archivoAbierto){
            archivoAbierto.close();
        }

        if (ruta.length() > 1) 
        {
            if (SD.exists(ruta))
            {
                SD.remove(ruta);
            }
        }
    }
};