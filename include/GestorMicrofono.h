#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

//Frecuencia del buzzer y volumen
#define MIN_LECTURA_ANALOGICA 0
#define MAX_LECTURA_ANALOGICA 4095
#define MIN_VOL 1
#define MAX_VOL 100
#define MIN_FREC 500
#define MAX_FREC 2500

//Lectura de audio
#define TASA_MUESTREO 48000

class GestorDeMicrofono {
private:
    // --- Atributos de Hardware ---
    int pinBuzzer;
    int pinLed;
    int pinVolumen;
    int frecuenciaPitido;

    // --- Pines del Micrófono I2S (Para uso futuro) ---
    int pinMicBCLK;
    int pinMicWS;
    int pinMicDATA;

    // --- Estado Interno ---
    int volumenActual;

    //Para almacenar el audio
    GestorAlmacenamiento *gestorAlmacenamiento;
            int32_t i2sBuffer[256];
        int16_t muestras16Bits[256];

public:
    GestorDeMicrofono(int pBuzzer, int pLed, int pVol, int frec, int bclk, int ws, int data) {
        pinBuzzer = pBuzzer;
        pinLed = pLed;
        pinVolumen = pVol;
        frecuenciaPitido = frec;
        
        pinMicBCLK = bclk;
        pinMicWS = ws;
        pinMicDATA = data;
        
        volumenActual = 0;
    }

    void setAlmacenamiento(GestorAlmacenamiento *gestor){
        gestorAlmacenamiento = gestor;
    }

    void iniciar() {
        pinMode(pinLed, OUTPUT);
        digitalWrite(pinLed, LOW); 

        pinMode(pinBuzzer, OUTPUT);
        digitalWrite(pinBuzzer, LOW); 

        analogReadResolution(12);

        // Inicialización del driver I2S: i2s_driver_install()

        i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = TASA_MUESTREO,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

        i2s_pin_config_t pin_config = {
        .bck_io_num = pinMicBCLK,
        .ws_io_num = pinMicWS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = pinMicDATA
        };

        i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        i2s_set_pin(I2S_NUM_0, &pin_config);
        i2s_zero_dma_buffer(I2S_NUM_0);

        Serial.println("Gestor de Micrófono y Feedback inicializado.");
    }

    // ==========================================
    // CONTROL DE VOLUMEN
    // ==========================================
    
    // Lee el potenciómetro, lo mapea a porcentaje (0-100) y lo devuelve
    void leerYActualizarVolumen() {
        int lecturaCruda = analogRead(pinVolumen);
        
        // Mapeamos el valor crudo (0 - 4095) a un porcentaje amigable (1 - 100)
        volumenActual = map(lecturaCruda, MIN_LECTURA_ANALOGICA, MAX_LECTURA_ANALOGICA, MIN_VOL, MAX_VOL);
        
    }

    // Devuelve el último volumen leído sin volver a consultar el hardware
    int obtenerVolumen() {
        return volumenActual;
    }

    // ==========================================
    // FEEDBACK VISUAL Y AUDITIVO
    // ==========================================

    void encenderLed() {
        digitalWrite(pinLed, HIGH);
    }

    void apagarLed() {
        digitalWrite(pinLed, LOW);
    }

    void encenderBuzzer() {
        // 3. Mapeamos el porcentaje (1-100) a un rango de frecuencias audibles.
        // 500 Hz es un tono grave (volumen bajo) y 2500 Hz es agudo (volumen alto).
        int frecuenciaDinamica = map(volumenActual, MIN_VOL, MAX_VOL, MIN_FREC, MAX_FREC);
        
        // 4. Emitimos el tono calculado
        tone(pinBuzzer, frecuenciaDinamica);
    }

    void apagarBuzzer() {
        noTone(pinBuzzer);
    }

    // ==========================================
    // PLACEHOLDERS PARA EL AUDIO I2S
    // ==========================================

    void confirmarInicioGrabacion() {
        apagarLed(); // El LED avisa que se está grabando
        encenderBuzzer();         
        Serial.println("Boton de \"Grabar\" presionado");
        // TODO Lógica de lectura I2S y escritura en SD
    }

    void iniciarGrabacion(const String ruta) {
        encenderLed(); // El LED avisa que se está grabando
        apagarBuzzer();         
        Serial.println("Grabando audio...");
        // TODO Lógica de lectura I2S y escritura en SD
        gestorAlmacenamiento->abrirArchivoWAV(ruta, TASA_MUESTREO, 16, 1);
    }
    void registrarMedida()
    {
        size_t bytesLeidos;
        i2s_read(I2S_NUM_0, (void *)i2sBuffer, sizeof(i2sBuffer), &bytesLeidos, portMAX_DELAY);

        int cantidadDeMuestras = bytesLeidos / 4; // Total de muestras de 32 bits recibidas (L + R)
        int muestrasGuardadas = 0;
    
        // Avanzamos de a 2 pasos (i += 2) para capturar solo el canal activo
        // y omitir la muestra de silencio del canal vacío
        for (int i = 0; i < cantidadDeMuestras; i += 2) {
            muestras16Bits[muestrasGuardadas] = (int16_t)(i2sBuffer[i] >> 14); 
            muestrasGuardadas++;
        }

        // Multiplicamos muestrasGuardadas por 2 porque cada int16_t ocupa 2 bytes físicos
        gestorAlmacenamiento->guardarDato((uint8_t*)muestras16Bits, muestrasGuardadas * 2);
    }

    void detenerGrabacion() {
        apagarLed();
        gestorAlmacenamiento->cerrarArchivoWAV();
        Serial.println("Grabación detenida.");
    }
};